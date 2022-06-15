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
            if (input.is_array())
                for (auto i : input)
                    verify_json(pointer, i, rules);

            // Find all rules that apply for the input node
            // TODO: accelerate this
            std::vector<json> matching_rules;
            for (auto i : rules)
            {
                if (i.at("pointer") == pointer)
                    matching_rules.push_back(i);
            }

            // There must be at least one, otherwise warning
            if (matching_rules.empty())
                std::cout << "WARNING: "
                          << "Unknown entry " << pointer << std::endl;

            // Test all rules, only one must pass, otherwise throw exception
            int count = 0;

            for (auto i : matching_rules)
                if (verify_rule(input, i))
                    count++;

            if (count != 1)
                throw matching_rules;

            // If it passes and if it is a dictionary, then test all childrens
            if (input.is_structured())
                for (auto &i : input.items())
                    verify_json(pointer + i.key() + "/", i.value(), rules);

            // If they all pass, return true
            return true;
        }
        catch (std::vector<json> e)
        {
            for (auto i: e)
                std::cout << "ERROR: " << i << std::endl; 
            return false;
        }
    };
    bool sjv::verify_rule(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_file(const json &input, const json &rule)
    {
        return false;
    };
    bool sjv::verify_rule_double(const json &input, const json &rule)
    {
        return false;
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
