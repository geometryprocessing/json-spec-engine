import argparse
import json
from collections import OrderedDict
import pathlib

import sys
sys.path.append(str(pathlib.Path(__file__).parent))
from inject_include import inject_include  # noqa


def find_rules(spec, pointer):
    return list(filter(lambda rule: rule["pointer"] == pointer, spec))


def generate_defaults(spec, root="/"):
    defaults = {}
    rules = find_rules(spec, root)

    default_rule = list(filter(lambda rule: "default" in rule, rules))
    assert (len(default_rule) <= 1)

    object_rules = list(filter(lambda rule: rule["type"] == "object", rules))

    if (default_rule and default_rule[0]["default"] is not None and not object_rules):
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
    parser.add_argument(
        "--include-dirs", nargs="+", help="Directories to search for includes",
        default=None)
    args = parser.parse_args()

    if args.include_dirs is None:
        args.include_dirs = [pathlib.Path(args.spec_path).parent]

    with open(args.spec_path) as f:
        spec = json.load(f)

    spec = inject_include(spec, args.include_dirs)

    defaults = generate_defaults(spec)

    with open(args.output, "w") as f:
        json.dump(defaults, f, indent=4)


if __name__ == "__main__":
    main()
