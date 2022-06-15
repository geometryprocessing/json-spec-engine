#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <memory>

using json = nlohmann::json;
using string = std::string;

namespace sjv
{
    class sjv
    {
    public:
        // verify the input json against the set of rules in specs
        bool verify_json(const json &input, const json &rules);
        bool verify_json(const string &pointer, const json &input, const json &rules);

        // Dispatcher for rule verification
        bool verify_rule(const json &input, const json &rule);

        // Type-specific rule handlers
        bool verify_rule_file(const json &input, const json &rule);
        bool verify_rule_double(const json &input, const json &rule);
        bool verify_rule_int(const json &input, const json &rule);
        bool verify_rule_path(const json &input, const json &rule);
        bool verify_rule_string(const json &input, const json &rule);
        bool verify_rule_dict(const json &input, const json &rule);

        // Working directory
        string cwd;
    };

} // namespace sjv
