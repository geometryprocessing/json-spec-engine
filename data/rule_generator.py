import json
from xml.dom.pulldom import default_bufsize
from xml.etree.ElementPath import prepare_descendant

default = None
with open('./default.json', "r") as f:
    default = json.load(f)
    
rules = []
tmp = {}
tmp["pointers"] = '/'
tmp['type'] = 'object'
tmp['optional'] = [*default.keys()]
tmp['doc'] = "Root of the configuration file."
rules.append(tmp)
def parse(default:json, prepending = ''):
    for k, v in default.items():
        if type(v) == dict:
            key = prepending + "/" + k
            tmp = {}
            tmp["pointer"] = key
            tmp['default'] = None
            tmp['type'] = 'object'
            tmp['optional'] = [*v.keys()]
            tmp['doc'] = "//TODO"
            rules.append(tmp)
            parse(v, prepending=key)
        else:
            key = prepending + "/" + k
            tmp = {}
            tmp['pointers'] = key
            if type(v)==str:
                tmp["default"] = v
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
            elif v is None:
                tmp['default'] = None
                tmp['type'] = 'object'
            tmp['doc'] = '//TODO'
            rules.append(tmp)
parse(default)
with open('default_rules.json', 'w') as f:
    json.dump(rules, f, indent=4)