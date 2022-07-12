//////////////////////////////////////////////////////////////////////////
#include <jse/jse.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <filesystem>
//////////////////////////////////////////////////////////////////////////

using namespace jse;

TEST_CASE("single_rule", "[validator]")
{
    INFO("test1.");
    json input = R"(
        {
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object"
        }
        ]
        )"_json;

    JSE jse;

    bool b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(b);
}

TEST_CASE("only_one_rule_can_be_valid", "[validator]")
{
    json input = R"(
        {
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object"
        },
        {
            "pointer": "/",
            "type": "object"
        }
        ]
        )"_json;

    JSE jse;

    bool b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(!b);
}

TEST_CASE("min_bound_numeric", "[validator]")
{
    json input = R"(
        {
            "field1": 48.5
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "required": ["field1"]
        },
        {
            "pointer": "/field1",
            "type": "float",
            "min": 45
        }
        ]
        )"_json;

    JSE jse;

    bool b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(b);

    input["field1"] = 40.5;

    b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(!b);
}

TEST_CASE("file_type", "[validator]")
{
    json input = R"(
        {
            "file1": "CMakeCach.txt"
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "optional": ["file1"]
        },
        {
            "pointer": "/file1",
            "type": "file",
            "extensions": [".txt"],
            "default": "somestring"
        }
        ]
        )"_json;

    JSE jse;
    bool b;

    jse.cwd = std::filesystem::current_path();
    jse.strict = true;
    jse.skip_file_check = false;
    b = jse.verify_json(input, rules);

    INFO(jse.log2str());
    REQUIRE(!b);

    input["file1"] = "CMakeCache.txt";

    b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(b);

    rules[1]["extensions"][0] = ".msh";

    b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(!b);
}

TEST_CASE("type_string", "[validator]")
{
    json input = R"(
        {
            "string1": "teststring"
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "required": ["string1"]
        },
        {
            "pointer": "/string1",
            "type": "string"
        }
        ]
        )"_json;

    JSE jse;

    bool b;
    // b = jse.verify_json(input,rules);
    // INFO(jse.log2str());
    // REQUIRE(b);

    rules[1]["options"][0] = "blah";

    b = jse.verify_json(input, rules);
    INFO(jse.log2str());
    REQUIRE(!b);

    // rules[1]["options"][1] = "teststring";

    // b = jse.verify_json(input,rules);
    // INFO(jse.log2str());
    // REQUIRE(b);
}

TEST_CASE("type_object", "[validator]")
{
    json input = R"(
        {
            "string1": "teststring"
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "required": ["string1"]
        }
        ]
        )"_json;

    JSE jse;

    jse.strict = true;
    REQUIRE(!jse.verify_json(input, rules));
    jse.strict = false;
    REQUIRE(jse.verify_json(input, rules));

    rules[0]["required"][1] = "randomstring";

    REQUIRE(!jse.verify_json(input, rules));
}

TEST_CASE("file_01", "[validator]")
{
    std::ifstream ifs1("../data/input_01.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/rules_01.json");
    json rules = json::parse(ifs2);

    JSE jse;

    jse.strict = true;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("pushbox", "[validator]")
{
    std::ifstream ifs1("../data/push_box.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    JSE jse;

    jse.strict = true;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("screw", "[validator]")
{
    std::ifstream ifs1("../data/screw.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    JSE jse;

    jse.strict = true;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("slingshot", "[validator]")
{
    std::ifstream ifs1("../data/sling_shots.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    JSE jse;

    jse.strict = true;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("polyfem-data", "[validator]")
{
    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    std::string path("../data/polyfem-data-jsons");
    std::string ext(".json");
    for (auto &p : std::filesystem::recursive_directory_iterator(path))
    {
        if (p.path().extension() == ext)
        {
            if (p.path().stem().string() == "common")
                continue;
            INFO(p.path().string());
            std::ifstream ifs1(p.path().string());
            json input = json::parse(ifs1);
            JSE jse;

            jse.strict = true;

            bool r = jse.verify_json(input, rules);
            std::string s = jse.log2str();
            INFO(s);
            REQUIRE(r);
        }
    }
}
TEST_CASE("simple", "[inject]")
{
    json input = R"(
        {
            "string1": "teststring",
            "geometry":
            {
                "nested": 3
            }
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "required": ["string1"],
            "optional":["geometry","other"]
        },
        {
            "pointer": "/geometry",
            "type": "object",
            "default": null,
            "optional": ["nested"]
        },
        {
            "pointer": "/geometry/nested",
            "type": "int",
            "default": 3
        },
        {
            "pointer": "/other",
            "type": "int",
            "default": null
        },
        {
            "pointer": "/other/nested",
            "type": "int",
            "default": 3
        }
        ]
        )"_json;

    json output = R"(
        {
            "string1": "teststring",
            "geometry":
            {
                "nested": 3
            },
            "other":
            {
                "nested": 3
            }
        }
        )"_json;

    JSE jse;

    jse.strict = false;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);

    json return_json = jse.inject_defaults(input, rules);

    INFO(return_json);
    REQUIRE(return_json == output);
}

TEST_CASE("list", "[inject]")
{
    json input = R"(
        {
            "list1":
            [
                {
                    "count": 0,
                    "other": 1
                },
                {
                    "count": 0
                }
            ]
        }
        )"_json;

    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "object",
            "required": ["list1"]
        },
        {
            "pointer": "/list1",
            "type": "list"
        },
        {
            "pointer": "/list1/*",
            "type": "object",
            "required": ["count"],
            "optional": ["other"]
        },
        {
            "pointer": "/list1/*/count",
            "type": "int"
        },
        {
            "pointer": "/list1/*/other",
            "type": "int",
            "default": 2
        }
        ]
        )"_json;

    json output = R"(
        {
            "list1":
            [
                {
                    "count": 0,
                    "other": 1
                },
                {
                    "count": 0,
                    "other": 2
                }
            ]
        }
        )"_json;

    JSE jse;

    jse.strict = false;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
    INFO(s);
    REQUIRE(r);

    json return_json = jse.inject_defaults(input, rules);

    INFO(return_json);
    REQUIRE(return_json == output);
}
