#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <memory>

using json = nlohmann::json;
using string = std::string;

namespace sjv
{
    class SJV
    {
    public:
        // verify the input json against the set of rules in specs
        bool verify_json(const json &input, const json &rules);

        // enriches a given json with default values
        json inject_defaults(const json &input, const json &rules);

        // log to string
        std::string log2str();

        // Working directory
        string cwd = ".";

        // if strict == false, a json is valid even if it has entries not validated by a rule
        bool strict = false;
        bool skip_file_check = true;
        // automatic boxing for primitive types
        // if all rules fail for a basic type, try boxing it once and try again
        bool boxing_primitive = true;

        // message list
        typedef std::pair<std::string, std::string> log_item;
        std::vector<log_item> log;

    private:
        // enriches a given json with default values
        void inject_defaults(const string &pointer, const json &input, const json &rules, json &output);

        // Verify a node pointed by
        bool verify_json(const string &pointer, const json &input, const json &rules);

        // Dispatcher for rule verification
        bool verify_rule(const json &input, const json &rule);

        // Type-specific rule handlers
        bool verify_rule_file(const json &input, const json &rule);
        bool verify_rule_folder(const json &input, const json &rule);
        bool verify_rule_float(const json &input, const json &rule);
        bool verify_rule_int(const json &input, const json &rule);
        bool verify_rule_string(const json &input, const json &rule);
        bool verify_rule_object(const json &input, const json &rule);
        bool verify_rule_bool(const json &input, const json &rule);
        bool verify_rule_list(const json &input, const json &rule);

        // Collect all rules having a default
        json collect_default_rules(const json &rules);

        // Collect all rules having a default for a given pointer
        json collect_default_rules(const string &pointer, const json &rules);

        // Collect all rules having a given pointer
        std::vector<json> collect_pointer(const string &pointer, const json &rules);

        // Find the first rule matching a pointer
        json find_valid_rule(const string &pointer, const json &input, const json &rules);

        // Utils
        bool contained_in_list(string item, const json &list);

        // Checks if a given json pointer is a subset of a pointer string (containing wildcards).
        // If it is, the second return parameter is an instantiated pointer
        std::tuple<bool, string> is_subset_pointer(const string &json_pointer, const string &pointer);
    };

} // namespace sjv
