////////////////////////////////////////////////////////////////////////////////
#include <jse/jse.h>
#include <iostream>
#include <filesystem> // C++17
#include <sstream>
#include <algorithm>
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
        //assert(verify_json(input_copy, rules));

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

    bool JSE::verify_json(const string &pointer, json &input, const json &rules)
    {
        // if (pointer == "/common")
        //     std::cout << "gotcha" << std::endl;
        // Find all rules that apply for the input node
        // TODO: accelerate this
        std::vector<json> matching_rules = collect_pointer(pointer, rules);

        // There must be at least one, otherwise warning and return true if not strict
        if (matching_rules.empty())
        {
            log.push_back(log_item("warning", "Unknown entry " + pointer));

            if (strict)
                return false;
            else
                return true;
        }

        // Test all rules, only one must pass, otherwise throw exception
        int count = 0;
        json single_matched_rule;

        for (auto i : matching_rules)
        {
            if (verify_rule(input, i))
            {
                count++;
                single_matched_rule = i;
            }
        }

        if (count == 0 && !matching_rules.empty())
        {
            // Before giving up, try boxing a primitive type
            if (boxing_primitive && !input.is_array())
            {
                string new_pointer = (pointer == "/" ? "" : pointer) + "/*";
                // Make sure there are some rules for the boxed version before recursively checking
                if (collect_pointer(new_pointer, rules).size() > 0)
                    if (verify_json(new_pointer, input, rules))
                        return true;
            }

            std::stringstream s;
            s << "No valid rules in this list:";
            for (auto i : matching_rules)
                s << i << std::endl;
            log.push_back(log_item("error", s.str()));
            return false;
        }

        if (count > 1)
        {
            std::stringstream s;
            s << "Multiple valid rules in this list, only one should be valid:";
            for (auto i : matching_rules)
                s << i << std::endl;
            log.push_back(log_item("error", s.str()));
            return false;
        }

        // If it passes and if it is a dictionary, then test all childrens
        if (input.is_object())
            for (auto &i : input.items())
            {
                string new_pointer = (pointer == "/" ? "" : pointer) + "/" + i.key();
                // first of all, let's check if the specs are correct
                json defaults = collect_default_rules(new_pointer, rules);

                // if it is mandatory, make sure there are no defaults
                if (single_matched_rule.contains("required") && contained_in_list(i.key(), single_matched_rule["required"]))
                {
                    if (defaults.size() != 0)
                    {
                        log.push_back(log_item("error", "Inconsistent specifications: " + new_pointer + " is a mandatory field with a default value."));
                        return false;
                    }
                }
                // if it is optional, there should be only one default in the specs
                else if (single_matched_rule.contains("optional") && contained_in_list(i.key(), single_matched_rule["optional"]))
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
                if (!verify_json(new_pointer, i.value(), rules))
                    return false;
            }

        // If the dictionary is valid and has optional fields, add defaults for the optional fields
        
        if (input.is_object() || input.is_null()) // Note: null fields might be objects, and thus might have optional fields
            if (single_matched_rule.contains("optional")) // the object has a list of optional
            {
                // std::cout << "Before adding: " << input << std::endl;
                for (auto &i : single_matched_rule["optional"]) // for each optional field
                    if (!input.contains(i)) // if not already in the object
                    {
                        string new_pointer = (pointer == "/" ? "" : pointer) + "/" + string(i);
                        json defaults = collect_default_rules(new_pointer, rules); // Find the default
                        if (defaults.size() != 1)
                        {
                            log.push_back(log_item("error", "Inconsistent specifications: " + new_pointer + " is an optional field with " + std::to_string(defaults.size()) + " default values."));
                            return false;
                        }
                        input[string(i)] = defaults[0]["default"];

                        // Let's validate/inject the default subtree
                        if (!verify_json(new_pointer, input[string(i)], rules))
                            return false;

                    }
                // std::cout << "After adding: " << input << std::endl;
            }

        // In case of a list
        // All the elements in the list must pass the test
        if (input.is_array())
        {
            for (auto& i : input)
                if (!verify_json((pointer == "/" ? "" : pointer) + "/*", i, rules))
                    return false;
        }

        // If they all pass, return true
        return true;
    };
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
        else
        {
            log.push_back(log_item("error", "Unknown rule type " + type));
            return false;
        }
    };
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
            std::string ext = p.extension();
            int count = 0;
            for (auto e : rule["extensions"])
                if (e == ext)
                    count++;
            if (count != 1)
                return false;
        }

        return true;
    };
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
    };
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
    };
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
    };
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
    };
    bool JSE::verify_rule_object(const json &input, const json &rule)
    {
        assert(rule.at("type") == "object");

        if (!input.is_object() && !input.is_null())
            return false;

        if (rule.contains("required"))
            for (auto e : rule["required"])
                if (!input.contains(string(e)))
                    return false;

        return true;
    };
    bool JSE::verify_rule_bool(const json &input, const json &rule)
    {
        assert(rule.at("type") == "bool");

        if (!input.is_boolean())
            return false;

        return true;
    };

    bool JSE::verify_rule_list(const json &input, const json &rule)
    {
        assert(rule.at("type") == "list");

        if (!input.is_array())
            return false;

        if (rule.contains("min") && !(input.size() >= rule["min"]))
            return false;

        if (rule.contains("max") && !(input.size() <= rule["max"]))
            return false;

        return true;
    };

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
    };

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
    };

    bool JSE::contained_in_list(string item, const json &list)
    {
        return std::find(list.begin(), list.end(), item) != list.end();
    };

    std::vector<json> JSE::collect_pointer(const string &pointer, const json &rules)
    {
        std::vector<json> matching_rules;
        for (auto i : rules)
        {
            if (i.at("pointer") == pointer)
                matching_rules.push_back(i);
        }
        return matching_rules;
    };

    json JSE::find_valid_rule(const string &pointer, const json &input, const json &rules)
    {
        for (auto i : collect_pointer(pointer, rules))
            if (verify_rule(input, i))
                return i;

        return json();
    };

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
    };

} // namespace jse
