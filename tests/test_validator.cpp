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
            "type": "object"
        },
        {
            "pointer": "/string1",
            "type": "string"
        }
        ]
        )"_json;

    sjv::SJV sjv;

    bool b;
    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(b);

    rules[1]["options"][0] = "blah";

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(!b);

    rules[1]["options"][1] = "teststring";

    b = sjv.verify_json(input,rules);
    INFO(sjv.log2str());
    REQUIRE(b);

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
    WARN(s);
    REQUIRE(r);
}
