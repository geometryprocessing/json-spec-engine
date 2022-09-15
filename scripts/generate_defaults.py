import argparse
import json
from collections import OrderedDict


def find_rules(spec, pointer):
    return list(filter(lambda rule: rule["pointer"] == pointer, spec))


def generate_defaults(spec, root="/"):
    defaults = {}
    rules = find_rules(spec, root)

    default_rule = list(filter(lambda rule: "default" in rule, rules))
    assert(len(default_rule) <= 1)

    object_rules = list(filter(lambda rule: rule["type"] == "object", rules))

    if(default_rule and default_rule[0]["default"] is not None and not object_rules):
        return default_rule[0]["default"]
    else:
        for i, rule in enumerate(object_rules):
            for required in rule.get("required", []):
                ptr = ("" if root == "/" else root) + "/" + required
                if root == "/":
                    defaults[required] = "REQUIRED!"
                else:
                    defaults[ptr.split("/")[-1]] = "REQUIRED!"
            for optional in rule.get("optional", []):
                ptr = ("" if root == "/" else root) + "/" + optional
                subtree = generate_defaults(spec, ptr)
                if root == "/":
                    defaults[optional] = subtree
                else:
                    defaults[ptr.split("/")[-1]] = subtree
    r = OrderedDict()
    for k in sorted(defaults.keys()):
        r[k] = defaults[k]
    return r


def main():
    parser = argparse.ArgumentParser("Generate defaults.json from spec.json")
    parser.add_argument(
        "-i,--input", dest="spec_path", help="Path to the JSON schema")
    parser.add_argument(
        "-o,--output", dest="output", help="Path to the output file",
        default="defaults.json")
    args = parser.parse_args()

    with open(args.spec_path) as f:
        spec = json.load(f)

    defaults = generate_defaults(spec)

    with open(args.output, "w") as f:
        json.dump(defaults, f, indent=4)


if __name__ == "__main__":
    main()
