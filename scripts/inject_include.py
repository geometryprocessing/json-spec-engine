import json
import pathlib


def prepend_pointer(pointer, key):
    if key == "/":
        return pointer
    elif pointer == "/":
        return key
    else:
        return key + pointer


def inject_include(rules: dict, dirs=None):
    if dirs is None:
        dirs = []
    dirs.append("")  # adding default path
    dirs = [pathlib.Path(d) for d in dirs]

    # max 10 levels of nesting to avoid infinite loops
    current = rules

    for _ in range(10):
        # check if the rules have any include
        include_present = any(rule["type"] == "include" for rule in current)

        # if there are no includes, return the current ones
        if not include_present:
            return current

        enriched = []
        # otherwise, do a round of replacement
        for rule in current:
            # copy all rules that are not include
            if rule["type"] != "include":
                enriched.append(rule)
                continue

            # if the rule is an include, expand the node with a copy of the included file
            replaced = False
            # the include file could be in any of the include directories
            for dir in dirs:
                spec_file = rule["spec_file"]
                f = dir / spec_file
                # check if the file exists
                if f.is_file():
                    with open(f, 'r') as ifs:
                        include_rules = json.load(ifs)

                    # loop over all rules to add the prefix
                    for i_rule in include_rules:
                        prefix = rule["pointer"]
                        pointer = i_rule["pointer"]
                        new_pointer = prepend_pointer(pointer, prefix)
                        i_rule["pointer"] = new_pointer

                    # save modified rules
                    for i_rule in include_rules:
                        enriched.append(i_rule)

                    # one substitution is enough, give up the search over include dirs
                    replaced = True
                    break

            if not replaced:
                pointer = rule["pointer"]
                raise RuntimeError(
                    f"Failed to replace the include rule: {pointer}")

        # now that we replaced the include, copy it back to current
        current = enriched

    raise RuntimeError("Reached maximal 10 levels of include recursion.")
