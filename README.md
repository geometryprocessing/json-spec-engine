[![Build](https://github.com/geometryprocessing/json-spec-engine/actions/workflows/continuous.yml/badge.svg?branch=main)](https://github.com/geometryprocessing/json-spec-engine/actions/workflows/continuous.yml)

# Json Spec Engine

JSE is a library to validate a json file containing a set of parameters and to complete it with default parameters. The specification of the json format is itself a json file. The library contains python scripts for generating documentation for the json format in html format and as a graph.

## Example

Given a json file as input:
```cpp
    json input = R"(
        {
            "string1": "teststring",
            "geometry":
            {
                "nested": 3
            }
        }
        )"_json;
```
and a set of rules to define its format
```cpp
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
```
The library can be used to test the validity of the json file:
```cpp
    JSE jse;

    bool r = jse.verify_json(input, rules);
    std::string s = jse.log2str();
```
any eventual error is reported in s.

Additionally, the library can automatically complete the json with the default parameters in the specification:
```cpp
    json return_json = jse.inject_defaults(input, rules);
```
generating the following json
```cpp
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
```

## Rules

Every rule must have a property called "pointer", that refers to an entry in the input, and a type, that specified the type of the entry. A list in the hierarchy is represented with a * character.

For a json to be file to be valid, every entry in the json must match *one and only one* rule. Each rule matches an input depending on its type and on rule-specific parameters.

The specification supports polymorphic types: it is possible to specify multiple rules for the same pointer. In this case the file is valid as long as any of the rules is valid.

### "type": "object"

The entry must be an object. The object can contain a set of required fields (required), and a set of optional fields. Note that every optional field must provide a default value in the spec. Additionally, it is possible to specify a type-name, in that case the object must contain a field called type with the matching name.

Optional parameters: 
* required: list of fields required for this object to be valid
* optional: list of fields that are optional. Every optional field must have at least one rule with a default
* type-name: the object must have a type field with the same name

### "type": "float"

The entry must be a floating point number, and optionally within a given range.

Optional parameters: 
* min: number
* max: number

### "type": "int" 

The entry must be an integer number, and optionally within a given range.

Optional parameters: 
* min: number
* max: number

### "type": "file"

The entry must contain a valid path to a file that exists on the filesystem. Optionally, it is possible to specify valid extensions as a list of strings.

Optional parameters: 
* extensions: [".txt",".msh"]

### "type": "folder"

The entry must contain a valid path to a folder that exists on the filesystem.

### "type": "string"

The entry must be a string. Optionally, it is possible to restrict the valid strings to a set of predefined ones.

Optional parameters: 
* options: ["these","are","options"]

### "type": "list"

The entry must be a list. Optionally, it is possible to restrict the length of the list.

Optional parameters: 
* min: minimal size
* max: maximal size

### "type": "bool"

The entry must be a boolean value.

### "type": "include"

Mandatory parameter: 
* spec_file: ["spec.json"]

The entry must be valid with respect to the json spec provided in spec_file. spec_file can be a global path, or relative to the paths provided in the advanced option jse.include_directories

## Defaults

Every rule associated with an optional parameter of an object must specify a default value, using the default field.

# Advanced options
```cpp
jse.strict              // DEFAULT: false - if strict == false, a json is valid even if it has entries not validated by a rule
jse.skip_file_check     // DEFAULT: true - disable checking for existance of file and folders
jse.boxing_primitive    // DEFAULT: true - always try to convert any type t to a list of t for the purpose of finding a valid rule
jse.include_directories // DEFAULT: empty - list of strings containing additional paths where json spec are possibly located
```

## Authors
Zachary Ferguson,
Zhongshi Jiang,
Teseo Schneider,
Daniele Panozzo


