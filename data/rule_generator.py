import json
from xml.etree.ElementPath import prepare_descendant

default = None
with open('./default.json', "r") as f:
    default = json.load(f)
    
rules = []

def parse(default:json, prepending = ''):
    for k, v in default.items():
        if type(v) == dict:
            key = prepending + "/" + k
            tmp = {}
            tmp["pointers"] = key
            tmp['default'] = 'null'
            tmp['type'] = 'object'
            tmp['optional'] = [*v.keys()]
            tmp['doc'] = "//TODO"
            rules.append(tmp)
            parse(v, prepending=key)
        else:
            key = prepending + "/" + k
            tmp = {}
            tmp['pointers'] = key
            if v == "":
                tmp["default"] = 'null'
                tmp['type'] = 'string'
            elif type(v) == list:
                tmp['default'] = v
                tmp['type'] = 'list'
            elif type(v) == int:
                tmp['default'] = v
                tmp['type'] = 'int'
            elif type(v) == float:
                tmp['default'] = v
                tmp['type'] = 'float'
            elif type(v) == bool:
                tmp['default'] = v
                tmp['type'] = 'bool'
            tmp['doc'] = '//TODO'
            rules.append(tmp)
parse(default)
with open('tmp_rules.json', 'w') as f:
    json.dump(rules, f, indent=4)