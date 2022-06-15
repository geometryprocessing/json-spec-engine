//////////////////////////////////////////////////////////////////////////
#include <sjv/sjv.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
//////////////////////////////////////////////////////////////////////////

using namespace sjv;

TEST_CASE("single_rule", "[validator]")
{
    json input = R"( 
        {
            "field1": "string", 
            "field2": 42 
        }
        )"_json;

    json rules = R"( 
        [
        {
            "pointer": "/",
            "type": "dict",
            "key_fields": ["field1"],
            "mandatory_fields": ["field1"]
        }
        ]
        )"_json;

    sjv::sjv sjv;

    REQUIRE(sjv.verify_json(input,rules));
}
