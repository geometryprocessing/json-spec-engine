#include <jse/jse.h>

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    struct Options
    {
        std::filesystem::path input;
        std::filesystem::path output_header;
        std::filesystem::path output_source;
        std::string namespace_name = "jse";
        std::string function_name = "rules";
        std::vector<std::string> include_dirs;
    };

    void print_usage(const char *argv0)
    {
        std::cerr << "Usage: " << argv0 << " --input spec.json"
                  << " --output-header spec.hpp --output-source spec.cpp"
                  << " [--namespace name] [--function name]"
                  << " [--include-dir dir ...]\n";
    }

    bool is_identifier(const std::string &value)
    {
        if (value.empty())
            return false;

        const auto first = static_cast<unsigned char>(value.front());
        if (!(std::isalpha(first) || value.front() == '_'))
            return false;

        for (char c : value.substr(1))
        {
            const auto uc = static_cast<unsigned char>(c);
            if (!(std::isalnum(uc) || c == '_'))
                return false;
        }

        return true;
    }

    std::vector<std::string> split_namespace(const std::string &namespace_name)
    {
        std::vector<std::string> parts;
        if (namespace_name.empty())
            return parts;

        std::size_t start = 0;
        while (start <= namespace_name.size())
        {
            const std::size_t end = namespace_name.find("::", start);
            const std::string part = namespace_name.substr(
                start, end == std::string::npos ? std::string::npos : end - start);
            if (!is_identifier(part))
                throw std::runtime_error("Invalid namespace component: " + part);

            parts.push_back(part);

            if (end == std::string::npos)
                break;
            start = end + 2;
        }

        return parts;
    }

    Options parse_options(int argc, char *argv[])
    {
        Options options;

        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];
            const auto require_value = [&](const std::string &option) -> std::string {
                if (i + 1 >= argc)
                    throw std::runtime_error("Missing value for " + option);
                return argv[++i];
            };

            if (arg == "--input")
                options.input = require_value(arg);
            else if (arg == "--output-header")
                options.output_header = require_value(arg);
            else if (arg == "--output-source")
                options.output_source = require_value(arg);
            else if (arg == "--namespace")
                options.namespace_name = require_value(arg);
            else if (arg == "--function")
                options.function_name = require_value(arg);
            else if (arg == "--include-dir")
                options.include_dirs.push_back(require_value(arg));
            else if (arg == "--help" || arg == "-h")
            {
                print_usage(argv[0]);
                std::exit(EXIT_SUCCESS);
            }
            else
            {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        if (options.input.empty())
            throw std::runtime_error("Missing --input");
        if (options.output_header.empty())
            throw std::runtime_error("Missing --output-header");
        if (options.output_source.empty())
            throw std::runtime_error("Missing --output-source");
        if (!is_identifier(options.function_name))
            throw std::runtime_error("Invalid function name: " + options.function_name);

        split_namespace(options.namespace_name);

        return options;
    }

    jse::json load_and_expand_rules(const Options &options)
    {
        std::ifstream input(options.input);
        if (!input)
            throw std::runtime_error("Failed to open input spec: " + options.input.string());

        jse::json rules = jse::json::parse(input);

        jse::JSE engine;
        engine.include_directories = options.include_dirs;

        const auto parent = std::filesystem::absolute(options.input).parent_path();
        if (!parent.empty())
            engine.include_directories.push_back(parent.string());

        return engine.inject_include(rules);
    }

    std::string raw_string_literal(const std::string &value)
    {
        for (int i = 0; i < 10000; ++i)
        {
            const std::string delimiter = i == 0 ? "JSE_JSON" : "JSE_JSON_" + std::to_string(i);
            if (value.find(")" + delimiter + "\"") == std::string::npos)
                return "R\"" + delimiter + "(\n" + value + "\n)" + delimiter + "\"";
        }

        throw std::runtime_error("Failed to find a valid raw string delimiter.");
    }

    void ensure_parent_directory(const std::filesystem::path &path)
    {
        const auto parent = path.parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent);
    }

    void write_file(const std::filesystem::path &path, const std::string &content)
    {
        ensure_parent_directory(path);

        std::ofstream output(path);
        if (!output)
            throw std::runtime_error("Failed to open output file: " + path.string());

        output << content;
        if (!output)
            throw std::runtime_error("Failed to write output file: " + path.string());
    }

    void open_namespaces(std::ostream &os, const std::vector<std::string> &namespaces)
    {
        for (const auto &name : namespaces)
            os << "namespace " << name << "\n{\n";
        if (!namespaces.empty())
            os << "\n";
    }

    void close_namespaces(std::ostream &os, const std::vector<std::string> &namespaces)
    {
        for (auto it = namespaces.rbegin(); it != namespaces.rend(); ++it)
            os << "} // namespace " << *it << "\n";
    }

    std::string header_content(const Options &options)
    {
        const auto namespaces = split_namespace(options.namespace_name);

        std::ostringstream os;
        os << "#pragma once\n\n";
        os << "#include <nlohmann/json.hpp>\n\n";
        open_namespaces(os, namespaces);
        os << "const nlohmann::json &" << options.function_name << "();\n";
        if (!namespaces.empty())
            os << "\n";
        close_namespaces(os, namespaces);

        return os.str();
    }

    std::string source_content(const Options &options, const jse::json &rules)
    {
        const auto namespaces = split_namespace(options.namespace_name);
        const auto header_name = options.output_header.filename().string();
        const std::string rules_text = rules.dump(4, ' ', true);

        std::ostringstream os;
        os << "#include \"" << header_name << "\"\n\n";
        open_namespaces(os, namespaces);
        os << "const nlohmann::json &" << options.function_name << "()\n";
        os << "{\n";
        os << "    static const nlohmann::json value = nlohmann::json::parse("
           << raw_string_literal(rules_text) << ");\n";
        os << "    return value;\n";
        os << "}\n";
        if (!namespaces.empty())
            os << "\n";
        close_namespaces(os, namespaces);

        return os.str();
    }
} // namespace

int main(int argc, char *argv[])
{
    try
    {
        const Options options = parse_options(argc, argv);
        const jse::json rules = load_and_expand_rules(options);

        write_file(options.output_header, header_content(options));
        write_file(options.output_source, source_content(options, rules));
        return EXIT_SUCCESS;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << std::endl;
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
}
