////////////////////////////////////////////////////////////////////////////////
#include <sjv/sjv.h>
#include <iostream>
#include <filesystem> // C++17
////////////////////////////////////////////////////////////////////////////////

namespace sjv
{
    bool sjv::verify_json(const json &input, const json &rules)
    {
        return verify_json("/", input, rules);
    }

    bool sjv::verify_json(const string &pointer, const json &input, const json &rules)
    {
        // Polymorphism on list, for us a single item or a list are indistinguishable
        // All the elements in the list must pass the test
        if (input.is_array())
        {
            for (auto i : input)
                if (!verify_json(pointer, i, rules))
                    return false;
            return true;
        }

        // Find all rules that apply for the input node
        // TODO: accelerate this
        std::vector<json> matching_rules;
        for (auto i : rules)
        {
            if (i.at("pointer") == pointer)
                matching_rules.push_back(i);
        }

        // There must be at least one, otherwise warning and return true if not strict
        if (matching_rules.empty())
        {
            std::cout << "WARNING: "
                      << "Unknown entry " << pointer << std::endl;

            if (strict)
                return false;
        }

        // Test all rules, only one must pass, otherwise throw exception
        int count = 0;

        for (auto i : matching_rules)
            if (verify_rule(input, i))
                count++;

        if (count == 0 && !matching_rules.empty())
        {
            std::cout << "ERROR: No valid rules in this list:" << std::endl;
            for (auto i : matching_rules)
                std::cout << i << std::endl;
            return false;
        }

        if (count > 1)
        {
            std::cout << "ERROR: Multiple valid rules in this list, only one should be valid:" << std::endl;
            for (auto i : matching_rules)
                std::cout << i << std::endl;
            return false;
        }

        // If it passes and if it is a dictionary, then test all childrens
        // TODO: if any of these fails we need to report it somehow
        if (input.is_structured())
            for (auto &i : input.items())
                if (!verify_json(pointer + i.key() + "/", i.value(), rules))
                    return false;

        // If they all pass, return true
        return true;
    };
    bool sjv::verify_rule(const json &input, const json &rule)
    {
        string type = rule.at("type");
        if (type == "skip_check")
            return true;
        else if (type == "float")
            return verify_rule_float(input, rule);
        else if (type == "int")
            return verify_rule_float(input, rule);
        else if (type == "file")
            return verify_rule_file(input, rule);
        else if (type == "folder")
            return verify_rule_folder(input, rule);
        else if (type == "string")
            return verify_rule_string(input, rule);
        else if (type == "object")
            return verify_rule_object(input, rule);
        else
        {
            std::cout << "ERROR: Unknown type " << std::endl;
            exit(-1);
            return false;
        }
    };
    bool sjv::verify_rule_file(const json &input, const json &rule)
    {
        assert(rule.at("type") == "file");

        std::string p_str = cwd + "/" + string(input);
        std::filesystem::path p = std::filesystem::path(p_str);

        if (!std::filesystem::is_regular_file(p))
            return false;

        if (rule.contains("extensions"))
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
    bool sjv::verify_rule_folder(const json &input, const json &rule)
    {
        assert(rule.at("type") == "folder");

        std::string p_str = cwd + "/" + string(input);
        std::filesystem::path p = std::filesystem::path(p_str);

        if (!std::filesystem::is_directory(p))
            return false;

        return true;
    };
    bool sjv::verify_rule_float(const json &input, const json &rule)
    {
        assert(rule.at("type") == "float");

        if (!input.is_number_float())
            return false;

        if (rule.contains("min") && input < rule["min"])
            return false;

        if (rule.contains("max") && input > rule["max"])
            return false;

        return true;
    };
    bool sjv::verify_rule_int(const json &input, const json &rule)
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
    bool sjv::verify_rule_string(const json &input, const json &rule)
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
    bool sjv::verify_rule_object(const json &input, const json &rule)
    {
        assert(rule.at("type") == "object");

        if (!input.is_object())
            return false;

        if (rule.contains("required"))
            for (auto e : rule["required"])
                if (!input.contains(string(e)))
                    return false;

        return true;
    };

} // namespace sjv
