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

const std::string root_path = JSE_TEST_ROOT;

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
            "file1": "dummi.txt"
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

    jse.cwd = root_path;
    jse.strict = true;
    jse.skip_file_check = false;
    b = jse.verify_json(input, rules);

    INFO(jse.log2str());
    REQUIRE(!b);

    input["file1"] = "dummy.txt";

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

TEST_CASE("include_rule", "[validator]")
{
    json rules = R"(
        [
        {
            "pointer": "/",
            "type": "include",
            "spec_file": "rules_02.json"
        },
        {
            "pointer": "/object1",
            "type": "include",
            "spec_file": "rules_02.json"
        }
        ]
        )"_json;

    JSE jse;
    jse.include_directories.push_back(root_path);
    json new_rules = jse.inject_include(rules);

    std::ifstream ifs2(root_path + "/rules_03.json");
    json matching = json::parse(ifs2);

    INFO(new_rules);
    INFO(matching);

    INFO(jse.log2str());
    REQUIRE(new_rules == matching);
}

TEST_CASE("file_01", "[validator]")
{
    std::ifstream ifs1(root_path + "/input_01.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2(root_path + "/rules_01.json");
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
    std::ifstream ifs1(root_path + "/push_box.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2(root_path + "/default_rules.json");
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
    std::ifstream ifs1(root_path + "/screw.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2(root_path + "/default_rules.json");
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
    std::ifstream ifs1(root_path + "/sling_shots.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2(root_path + "/default_rules.json");
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
    std::ifstream ifs2(root_path + "/default_rules.json");
    json rules = json::parse(ifs2);

    std::string path(root_path + "/polyfem-data-jsons");
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
            "pointer": "/string1",
            "type": "string"
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
            "type": "object",
            "optional": ["nested"],
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
                    "count": 5
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
                    "count": 5,
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

TEST_CASE("polymorphism", "[inject]")
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
                },
                {
                    "othertype": "ciao"
                },
                {
                    "othertype": "ciao2",
                    "otheroptional": "donotchangethis"
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
        },
        {
            "pointer": "/list1/*",
            "type": "object",
            "required": ["othertype"],
            "optional": ["otheroptional"]
        },
        {
            "pointer": "/list1/*/othertype",
            "type": "string"
        },
        {
            "pointer": "/list1/*/otheroptional",
            "type": "string",
            "default": "defaultstring"
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
                },
                {
                    "othertype": "ciao",
                    "otheroptional": "defaultstring"
                },
                {
                    "othertype": "ciao2",
                    "otheroptional": "donotchangethis"
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

TEST_CASE("null_as_nan", "[validator][inject]")
{
    nlohmann::json rules = R"(
        [
            {
                "pointer": "/",
                "type": "object",
                "required": ["f1"],
                "optional": ["f2"]
            },
            {
                "pointer": "/f1",
                "type": "float"
            },
            {
                "pointer": "/f2",
                "type": "int",
                "default": null
            }
        ]
        )"_json;

    nlohmann::json input = R"(
        {
            "f1": null,
            "f2": null
        }
        )"_json;

    JSE jse;
    jse.strict = true;

    bool r = jse.verify_json(input, rules);
    REQUIRE(r);

    input.erase("f2");
    r = jse.verify_json(input, rules);
    REQUIRE(r);

    const json return_json = jse.inject_defaults(input, rules);
    CHECK(return_json["f1"].is_null());
    CHECK(return_json["f2"].is_null());

    input["f2"] = std::nan("");
}