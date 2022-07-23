import pathlib
import argparse
import json

from unittest import skip
from mako.template import Template
from mako.lookup import TemplateLookup


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
    args = parser.parse_args()

    # Template Lookup
    mylookup = TemplateLookup(directories=['.'])

    print("01 - Loading specification")
    with open(args.input) as input_file:
        input = json.load(input_file)

    # print(input)

    print("02 - Templating")
    mako_file = pathlib.Path(__file__).parent.absolute() / "main.mako"
    mako_template = Template(filename=str(mako_file))
    body = mako_template.render(
        title=args.title, ijson=json_navigator(input), attributes={"count": 0})

    print("03 - Writing to output")
    with open(args.output, 'w') as f:
        f.write(body)
