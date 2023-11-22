import pathlib
import argparse
import json

from unittest import skip
from mako.template import Template
from mako.lookup import TemplateLookup

import sys
sys.path.append(str(pathlib.Path(__file__).parents[1] / "scripts"))
from inject_include import inject_include  # noqa


class json_navigator:
    def __init__(self, ijson):
        self.ijson = ijson

    def rules_from_pointer(self, pointer):
        return [x for x in self.ijson if x['pointer'] == pointer]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input", "-i", help="Path to the input specification", required=True)
    parser.add_argument(
        "--output", "-o", help="Path to the output html file", required=True)
    parser.add_argument(
        "--title", "-t", help="Title", default="JSON Specification")
    parser.add_argument(
        "--include-dirs", nargs="+", help="Directories to search for includes",
        default=None)
    args = parser.parse_args()

    if args.include_dirs is None:
        args.include_dirs = [pathlib.Path(args.spec_path).parent]

    # Template Lookup
    mylookup = TemplateLookup(directories=['.'])

    print("01 - Loading specification")
    with open(args.input) as input_file:
        input = json.load(input_file)

    input = inject_include(input, args.include_dirs)

    # print(input)

    print("02 - Templating")
    if args.output.endswith(".md"):
        mako_file = pathlib.Path(__file__).parent.absolute() / "main.md.mako"
    else:
        mako_file = pathlib.Path(__file__).parent.absolute() / "main.html.mako"
    mako_template = Template(filename=str(mako_file))
    body = mako_template.render(
        title=args.title, ijson=json_navigator(input), attributes={"count": 0})

    print("03 - Writing to output")
    with open(args.output, 'w') as f:
        f.write(body)
