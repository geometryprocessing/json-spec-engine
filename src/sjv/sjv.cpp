////////////////////////////////////////////////////////////////////////////////
#include <sjv/sjv.h>
#include <iostream>
////////////////////////////////////////////////////////////////////////////////

namespace sjv
{
    bool sjv::verify_json(const json &input, const json &rules)
    {
        return verify_json("/", input, rules);
    }

    bool sjv::verify_json(const string &pointer, const json &input, const json &rules)
    {
        try
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

            if (count != 1)
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
        }
        catch (std::vector<json> e)
        {
            return false;
        }
    };
    bool sjv::verify_rule(const json &input, const json &rule)
    {
        string type = rule.at("type");
        if (type == "skip_check")
            return true;
        else if (type == "float")
            return verify_rule_float(input, rule);
        else
        {
            std::cout << "ERROR: Unknown type " << std::endl;
            exit(-1);
            return false;
        }
    };
    bool sjv::verify_rule_file(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_float(const json &input, const json &rule)
    {
        assert(rule.at("type") == "float");

        if (!input.is_number_float())
            return false;

        if (rule.contains("min") && input < rule["min"])
            return false;

        return true;
    };
    bool sjv::verify_rule_int(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_path(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_string(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_dict(const json &input, const json &rule)
    {
        return false;
    };

} // namespace sjv
