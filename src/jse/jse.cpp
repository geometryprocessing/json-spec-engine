////////////////////////////////////////////////////////////////////////////////
#include <jse/jse.h>
#include <iostream>
#include <filesystem> // C++17
#include <sstream>
#include <algorithm>
#include <fstream>
////////////////////////////////////////////////////////////////////////////////

namespace jse
{

    //////////// PUBLIC

    bool JSE::verify_json(const json &input, const json &rules)
    {
        log.clear();
        json input_copy = input;
        return verify_json("/", input_copy, rules);
    };

    json JSE::inject_defaults(const json &input, const json &rules)
    {
        log.clear();
        json input_copy = input;
        bool b = verify_json("/", input_copy, rules);
        assert(b); // Make sure the original file is valid

        // Check that the new file, after adding defaults, is also valid
        // assert(verify_json(input_copy, rules));

        return input_copy;
    };

    std::string JSE::log2str()
    {
        std::stringstream s;

        for (log_item i : log)
        {
            s << i.first << ": " << i.second << std::endl;
        }
        return s.str();
    };

    //////////// PRIVATE

    namespace
    {
        // adds key after the provided pointer, correctly handling the json special case that root has an ending /
        std::string append_pointer(const std::string &pointer, const std::string &key)
        {
            if (pointer == "/")
                return "/" + key;
            else
                return pointer + "/" + key;
        }

        // adds key before the provided pointer, correctly handling the json special case that root has an ending /
        std::string prepend_pointer(const std::string &pointer, const std::string &key)
        {
            if (key == "/")
                return pointer;
            else if (pointer == "/")
                return key;
            else
                return key + pointer;
        }

    } // namespace

    // enriches a given json spec with included json specs
    json JSE::inject_include(const json &rules)
    {
        std::vector<string> dirs = include_directories;
        dirs.push_back(""); // adding default path

        // max 10 levels of nesting to avoid infinite loops
        json current = rules;

        for (size_t x = 0; x < 10; x++)
        {
            // check if the rules have any include
            bool include_present = false;
            for (const auto &rule : current)
                if (rule.at("type") == "include")
                    include_present = true;

            // if there are no includes, return the current ones
            if (!include_present)
                return current;

            json enriched;
            // otherwise, do a round of replacement
            for (const auto &rule : current)
            {
                // copy all rules that are not include
                if (rule.at("type") != "include")
                {
                    enriched.push_back(rule);
                }
                // if the rule is an include, expand the node with a copy of the included file
                else
                {
                    bool replaced = false;
                    // the include file could be in any of the include directories
                    for (const auto &dir : dirs)
                    {
                        string spec_file = rule.at("spec_file");
                        string f = dir + "/" + spec_file;
                        // check if the file exists
                        if (std::filesystem::is_regular_file(f))
                        {
                            std::ifstream ifs(f);
                            json include_rules = json::parse(ifs);

                            // loop over all rules to add the prefix
                            for (auto &i_rule : include_rules)
                            {
                                string prefix = rule.at("pointer");
                                string pointer = i_rule.at("pointer");
                                string new_pointer = prepend_pointer(pointer, prefix);
                                i_rule.at("pointer") = new_pointer;
                            }

                            // save modified rules
                            for (const auto &i_rule : include_rules)
                                enriched.push_back(i_rule);

                            // one substitution is enough, give up the search over include dirs
                            replaced = true;
                            break;
                        }
                    }

                    if (!replaced)
                    {
                        string pointer = rule.at("pointer");
                        throw std::runtime_error("Failed to replace the include rule: " + pointer);
                        assert(replaced == true);
                    }
                }
            }

            // now that we replaced the include, copy it back to current
            current = enriched;
        }

        throw std::runtime_error("Reached maximal 10 levels of include recursion.");
    }

    bool JSE::verify_json(const string &pointer, json &input, const json &rules)
    {
        // Find all rules that apply for the input node
        // TODO: accelerate this
        std::vector<json> matching_rules = collect_pointer(pointer, rules);

        // There must be at least one, otherwise warning and return true if not strict
        if (matching_rules.empty())
        {
            log.push_back(log_item("warning", "Unknown entry " + pointer));
            return !strict;
        }

        // Test all rules, one and only one must pass, otherwise throw exception
        std::vector<json> verified_matching_rules;
        for (auto r : matching_rules)
        {
            if (verify_rule(input, r))
            {
                verified_matching_rules.push_back(r);
            }
        }

        if (verified_matching_rules.size() == 0)
        {
            // Before giving up, try boxing a primitive type
            if (boxing_primitive && !input.is_array())
            {
                string new_pointer = append_pointer(pointer, "*");
                // Make sure there are some rules for the boxed version before recursively checking
                if (collect_pointer(new_pointer, rules).size() > 0)
                    if (verify_json(new_pointer, input, rules))
                        return true;
            }

            std::stringstream s;
            s << "No rule matched for \"" << pointer << "\": " << input.dump(/*indent=*/4) << std::endl;
            s << "No valid rules in this list:\n";
            for (int i = 0; i < matching_rules.size(); i++)
                s << i << ": " << matching_rules[i].dump(/*indent=*/4) << "\n";
            log.push_back(log_item("error", s.str()));
            return false;
        }
        else if (verified_matching_rules.size() > 1)
        {
            std::stringstream s;
            s << "Multiple rules matched for \"" << pointer << "\": " << input.dump(/*indent=*/4) << std::endl;
            s << "Multiple valid rules in this list, only one should be valid:";
            for (int i = 0; i < verified_matching_rules.size(); i++)
                s << i << ": " << verified_matching_rules[i].dump(/*indent=*/4) << "\n";
            log.push_back(log_item("error", s.str()));
            return false; // if not strict, we do not check for extra keys to distinguish between rules
        }
        const json &single_matched_rule = verified_matching_rules.front();

        // If it passes and if it is a dictionary, then test all childrens
        if (input.is_object())
        {
            for (const auto &[key, value] : input.items())
            {
                const string new_pointer = append_pointer(pointer, key);
                // first of all, let's check if the specs are correct
                const json defaults = collect_default_rules(new_pointer, rules);

                // if it is mandatory, make sure there are no defaults
                if (single_matched_rule.contains("required") && contained_in_list(key, single_matched_rule["required"]))
                {
                    if (defaults.size() != 0)
                    {
                        log.push_back(log_item("error", "Inconsistent specifications: " + new_pointer + " is a mandatory field with a default value."));
                        return false;
                    }
                }
                // if it is optional, there should be only one default in the specs
                else if (single_matched_rule.contains("optional") && contained_in_list(key, single_matched_rule["optional"]))
                {
                    if (defaults.size() != 1)
                    {
                        log.push_back(log_item("error", "Inconsistent specifications: " + new_pointer + " is an optional field with " + std::to_string(defaults.size()) + " default values."));
                        return false;
                    }
                }
                // if it is not mandatory and not optional, something is wrong
                else
                {
                    log.push_back(log_item("warning", "Inconsistent specifications: " + new_pointer + " is neither an optional or a mandatory field."));
                    if (strict)
                        return false;
                }

                // now let's make sure it can be validated
                if (!verify_json(new_pointer, value, rules))
                    return false;
            }
        }

        // If the dictionary is valid and has optional fields, add defaults for the optional fields

        if ((input.is_object() || input.is_null())       // Note: null fields might be objects, and thus might have optional fields
            && single_matched_rule.contains("optional")) // <- the object has a list of optional
        {
            for (const std::string &key : single_matched_rule["optional"]) // for each optional field
            {
                if (input.contains(key)) // skip if already in the object
                    continue;

                string new_pointer = append_pointer(pointer, key);
                json defaults = collect_default_rules(new_pointer, rules); // Find the default
                if (defaults.size() != 1)
                {
                    log.push_back(log_item("error", "Inconsistent specifications: " + new_pointer + " is an optional field with " + std::to_string(defaults.size()) + " default values."));
                    return false;
                }
                if (defaults[0]["default"] != "skip")
                {
                    input[key] = defaults[0]["default"];

                    // Let's validate/inject the default subtree
                    if (!verify_json(new_pointer, input[key], rules))
                        return false;
                }
            }
        }

        // In case of a list
        // All the elements in the list must pass the test
        if (input.is_array())
        {
            for (auto &i : input)
                if (!verify_json(append_pointer(pointer, "*"), i, rules))
                    return false;
        }

        // If they all pass, return true
        return true;
    }

    bool JSE::verify_rule(const json &input, const json &rule)
    {
        // std::cout << "Verifying " << input << std::endl;
        string type = rule.at("type");
        if (type == "list")
            return verify_rule_list(input, rule);
        else if (type == "float")
            return verify_rule_float(input, rule);
        else if (type == "int")
            return verify_rule_int(input, rule);
        else if (type == "file")
            return verify_rule_file(input, rule);
        else if (type == "folder")
            return verify_rule_folder(input, rule);
        else if (type == "string")
            return verify_rule_string(input, rule);
        else if (type == "object")
            return verify_rule_object(input, rule);
        else if (type == "bool")
            return verify_rule_bool(input, rule);
        else if (type == "include")
            return verify_rule_include(input, rule);
        else
        {
            log.push_back(log_item("error", "Unknown rule type " + type));
            return false;
        }
    }

    bool JSE::verify_rule_file(const json &input, const json &rule)
    {
        assert(rule.at("type") == "file");

        if (!input.is_string())
            return false;

        std::string p_str = cwd + "/" + string(input);
        std::filesystem::path p = std::filesystem::path(p_str);

        if (!skip_file_check && !std::filesystem::is_regular_file(p))
        {
            log.push_back(log_item("warning", "File not found: " + p_str));
            if (strict)
                return false;
        }

        if (!skip_file_check && rule.contains("extensions"))
        {
            std::string ext = p.extension().string();
            int count = 0;
            for (auto e : rule["extensions"])
                if (e == ext)
                    count++;
            if (count != 1)
                return false;
        }

        return true;
    }

    bool JSE::verify_rule_folder(const json &input, const json &rule)
    {
        assert(rule.at("type") == "folder");

        std::string p_str = cwd + "/" + string(input);
        std::filesystem::path p = std::filesystem::path(p_str);

        if (!std::filesystem::is_directory(p))
        {
            log.push_back(log_item("warning", "Folder not found: " + p_str));
            if (strict)
                return false;
        }

        return true;
    }

    bool JSE::verify_rule_float(const json &input, const json &rule)
    {
        assert(rule.at("type") == "float");

        if (!input.is_number())
            return false;

        if (rule.contains("min") && input < rule["min"])
            return false;

        if (rule.contains("max") && input > rule["max"])
            return false;

        return true;
    }

    bool JSE::verify_rule_int(const json &input, const json &rule)
    {
        assert(rule.at("type") == "int");

        if (!input.is_number_integer())
            return false;

        if (rule.contains("min") && input < rule["min"])
            return false;

        if (rule.contains("max") && input > rule["max"])
            return false;

        return true;
    }

    bool JSE::verify_rule_string(const json &input, const json &rule)
    {
        assert(rule.at("type") == "string");

        if (!input.is_string())
            return false;

        if (rule.contains("options"))
        {
            int count = 0;
            for (auto e : rule["options"])
                if (e == input)
                    count++;
            if (count != 1)
                return false;
        }

        return true;
    }
    bool JSE::verify_rule_object(const json &input, const json &rule)
    {
        assert(rule.at("type") == "object");

        if (!input.is_object() && !input.is_null())
            return false;

        // Check that all required fields are present
        if (rule.contains("required"))
            for (auto e : rule["required"])
                if (!input.contains(string(e)))
                    return false;

        if (strict) // strict mode: check that no extra fields are present
        {
            std::vector<std::string> keys;
            keys.reserve((rule.contains("required") ? rule["required"].size() : 0)
                         + (rule.contains("optional") ? rule["optional"].size() : 0));
            if (rule.contains("required"))
                keys.insert(keys.end(), rule["required"].begin(), rule["required"].end());
            if (rule.contains("optional"))
                keys.insert(keys.end(), rule["optional"].begin(), rule["optional"].end());

            for (const auto &[key, value] : input.items())
                if (std::find(keys.begin(), keys.end(), key) == keys.end())
                    return false;
        }

        if (rule.contains("type_name")
            && (!input.contains("type") || input["type"] != rule["type_name"]))
            return false;

        return true;
    }
    bool JSE::verify_rule_bool(const json &input, const json &rule)
    {
        assert(rule.at("type") == "bool");
        return input.is_boolean();
    }

    bool JSE::verify_rule_list(const json &input, const json &rule)
    {
        assert(rule.at("type") == "list");

        if (!input.is_array())
            return false;

        if (rule.contains("min") && input.size() < rule["min"])
            return false;

        if (rule.contains("max") && input.size() > rule["max"])
            return false;

        return true;
    }

    bool JSE::verify_rule_include(const json &input, const json &rule)
    {
        assert(rule.at("type") == "include");

        // An include rule always fails, they should be processed first by the inject_include function
        return false;
    }

    json JSE::collect_default_rules(const json &rules)
    {
        // Find all rules that apply for the input node
        // TODO: accelerate this

        std::vector<json> matching_rules;
        for (auto i : rules)
        {
            if (i.contains("default"))
                matching_rules.push_back(i);
        }

        return matching_rules;
    }

    json JSE::collect_default_rules(const string &pointer, const json &rules)
    {
        // Find all rules that apply for the input node
        // TODO: accelerate this

        std::vector<json> matching_rules;
        for (auto i : rules)
        {
            if (i.at("pointer") == pointer && i.contains("default"))
                matching_rules.push_back(i);
        }

        return matching_rules;
    }

    bool JSE::contained_in_list(string item, const json &list)
    {
        return std::find(list.begin(), list.end(), item) != list.end();
    }

    std::vector<json> JSE::collect_pointer(const string &pointer, const json &rules)
    {
        std::vector<json> matching_rules;
        for (auto i : rules)
        {
            if (i.at("pointer") == pointer)
                matching_rules.push_back(i);
        }
        return matching_rules;
    }

    json JSE::find_valid_rule(const string &pointer, const json &input, const json &rules)
    {
        for (auto i : collect_pointer(pointer, rules))
            if (verify_rule(input, i))
                return i;

        return json();
    }

    std::tuple<bool, string> JSE::is_subset_pointer(const string &json, const string &pointer)
    {
        // Splits a string into tokens using the deliminator delim
        auto tokenize = [](std::string const &str, const char delim) {
            size_t start;
            size_t end = 0;
            std::vector<string> out;

            while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
            {
                end = str.find(delim, start);
                out.push_back(str.substr(start, end - start));
            }

            return out;
        };

        // Replaces occurrences of a substring
        auto replace_all = [](std::string str, const std::string &from, const std::string &to) {
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        };

        // Check if a string is an integer
        auto is_number = [](const string &str) {
            for (char const &c : str)
                if (std::isdigit(c) == 0)
                    return false;
            return true;
        };

        // Tokenize json_pointer
        std::vector<string> json_t = tokenize(json, '/');
        std::vector<string> pointer_t = tokenize(pointer, '/');

        // if the json is not shorter or if there are no tokens, give up
        if ((json_t.size() >= pointer_t.size())
            || (pointer_t.size() == 0))
            return {false, ""};

        std::string buf = "";
        // if it is shorter, match every entry
        for (unsigned i = 0; i < pointer_t.size(); ++i)
        {
            // if there is no corresponding entry on json, copy and move on
            if (json_t.size() <= i)
            {
                buf.append("/" + replace_all(pointer_t[i], "*", "0"));
            }
            // if there is an entry on json and it is the same, move on
            else if (json_t[i] == pointer_t[i])
            {
                buf.append("/" + pointer_t[i]);
            }
            // if the pointer contains a star, then accept any integer on json
            else if (pointer_t[i] == "*")
            {
                if (is_number(json_t[i]))
                    buf.append("/" + json_t[i]);
            }
            // if no rule matches it is not a match
            else
            {
                return {false, ""};
            }
        }

        return {true, buf};
    }

} // namespace jse
