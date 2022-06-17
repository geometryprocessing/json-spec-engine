//////////////////////////////////////////////////////////////////////////
#include <sjv/sjv.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <filesystem>
//////////////////////////////////////////////////////////////////////////

using namespace sjv;

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

    sjv::SJV sjv;

    bool b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
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

    sjv::SJV sjv;

    bool b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
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

    sjv::SJV sjv;

    bool b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(b);

    input["field1"] = 40.5;

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
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

    sjv::SJV sjv;
    bool b;

    sjv.cwd = std::filesystem::current_path();
    sjv.strict = true;
    sjv.skip_file_check = false;
    b = sjv.verify_json(input,rules);
    
    INFO(sjv.log2str());
    REQUIRE(!b);

    input["file1"] = "CMakeCache.txt";

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(b);

    rules[1]["extensions"][0] = ".msh";

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
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

    sjv::SJV sjv;

    bool b;
    // b = sjv.verify_json(input,rules);
    // INFO(sjv.log2str());
    // REQUIRE(b);

    rules[1]["options"][0] = "blah";

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(!b);

    // rules[1]["options"][1] = "teststring";

    // b = sjv.verify_json(input,rules);
    // INFO(sjv.log2str());
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

    sjv::SJV sjv;

    sjv.strict = true;
    REQUIRE(!sjv.verify_json(input,rules));
    sjv.strict = false;
    REQUIRE(sjv.verify_json(input,rules));

    rules[0]["required"][1] = "randomstring";

    REQUIRE(!sjv.verify_json(input,rules));

}

TEST_CASE("file_01", "[validator]")
{
    std::ifstream ifs1("../data/input_01.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/rules_01.json");
    json rules = json::parse(ifs2);

    sjv::SJV sjv;

    sjv.strict = true;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("pushbox", "[validator]")
{
    std::ifstream ifs1("../data/push_box.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    sjv::SJV sjv;

    sjv.strict = true;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("screw", "[validator]")
{
    std::ifstream ifs1("../data/screw.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    sjv::SJV sjv;

    sjv.strict = true;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("slingshot", "[validator]")
{
    std::ifstream ifs1("../data/sling_shots.json");
    json input = json::parse(ifs1);

    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    sjv::SJV sjv;

    sjv.strict = true;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);
}

TEST_CASE("polyfem-data", "[validator]"){
    std::ifstream ifs2("../data/default_rules.json");
    json rules = json::parse(ifs2);

    std::string path("../data/polyfem-data-jsons");
    std::string ext(".json");
    for (auto &p : std::filesystem::recursive_directory_iterator(path))
    {
        if (p.path().extension() == ext){
            if(p.path().stem().string()=="common") continue;
            INFO(p.path().string());
            std::ifstream ifs1(p.path().string());
            json input = json::parse(ifs1);
            sjv::SJV sjv;

            sjv.strict = true;

            bool r = sjv.verify_json(input,rules); 
            std:: string s = sjv.log2str();
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

    sjv::SJV sjv;

    sjv.strict = false;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);

    json return_json = sjv.inject_defaults(input,rules); 

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

    sjv::SJV sjv;

    sjv.strict = false;

    bool r = sjv.verify_json(input,rules); 
    std:: string s = sjv.log2str();
    INFO(s);
    REQUIRE(r);

    json return_json = sjv.inject_defaults(input,rules); 

    INFO(return_json);
    REQUIRE(return_json == output);
}
