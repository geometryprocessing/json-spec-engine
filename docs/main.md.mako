## _*_ coding: utf-8 _*_
<!--Automatically generated using JSON Spec Engine-->
<%def name="render_pointer(pointer_name, prefix)">
${make_rules(rs=ijson.rules_from_pointer(pointer_name), prefix=prefix)}
</%def>
##
##
<%def name="make_rules(rs, prefix)">
<%
if len(rs) > 0:
    rs[0]['pointer_last'] = rs[0]['pointer'].rsplit('/', 1)[-1]
    if rs[0]['pointer_last'] == '*':
        tmp = rs[0]['pointer'].replace('/*','')
        rs[0]['pointer_last'] = tmp.rsplit('/', 1)[-1]
%>
##
% if len(rs) == 1: ## single rules for a pointer use a cell
${make_rule(r=rs[0], prefix=prefix)}
% endif
##
% if len(rs) > 1: ## multiple rules use a collapsible block
${prefix}${"!!!" if len(prefix) == 0 else "???"} summary "`${rs[0]['pointer']}`"
${prefix}    ```
${prefix}    ${rs[0]['pointer']}
${prefix}    ```
% for rule in rs:
${make_rule(r=rule, prefix=prefix + "    ")}
% endfor
% endif
</%def>
##
##
<%def name="make_rule(r, prefix)">
<%
if not 'doc' in r:
    r['doc'] = '<span class="todo">FIXME:</span> Missing documentation in the specification'
if r["doc"] == "TODO":
    r["doc"] = '<span class="todo">TODO</span>'
r['pointer_last'] = r['pointer'].rsplit('/', 1)[-1]
if r['pointer_last'] == '*':
    tmp = r['pointer'].replace('/*','')
    r['pointer_last'] = tmp.rsplit('/', 1)[-1]
r['pointer_short'] = r['pointer']
%>
## ----------------------------------------------------------------------------
% if r['type'] == "object": ## Object
${prefix}${"!!!" if len(prefix) == 0 else "???"} summary "${r.get("type_name", f"`{r['pointer_short']}` (`{r['type']}`)")}"
${prefix}    ```
${prefix}    ${r['pointer']}
${prefix}    ```
${prefix}    ${"##"} Description
${prefix}    ${r['doc']}
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% if "type_name" in r:
${prefix}
${prefix}    **Type**: ${r["type_name"]}
% endif
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% if "default" in r:
${prefix}
${prefix}    **Default**: ${repr(r["default"])}
% endif
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% if 'required' in r:
${prefix}    ${"##"} Required
% for fname in r['required']:
${render_pointer(
    pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname),
    prefix = prefix + "    ")}
% endfor
% endif
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% if 'optional' in r:
${prefix}    ${"##"} Optional
% for fname in r['optional']:
${render_pointer(
    pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname),
    prefix = prefix + "    ")}
% endfor
% endif
## ----------------------------------------------------------------------------
% elif r['type'] == "list": ## List
${render_pointer(
    pointer_name = (r['pointer']+'*') if r['pointer'] == '/' else (r['pointer']+'/'+'*'),
    prefix = prefix)}
## ----------------------------------------------------------------------------
% else:
${prefix}${"!!!" if len(prefix) == 0 else "???"} summary "`${r['pointer_short']}` (`${r["type"]}`)"
${prefix}    ```
${prefix}    ${r['pointer']}
${prefix}    ```
${prefix}    ${"##"} Description
${prefix}    ${r['doc']}
## - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
% if "min" in r or "max" in r:
${prefix}
${prefix}    **Range:** `(${r.get("min", "-inf")}, ${r.get("max", "inf")})`
% endif
% if "default" in r:
${prefix}
${prefix}    **Default**: `${repr(r["default"])}`
% endif
% if "extensions" in r:
${prefix}
${prefix}    **Extensions:** `${r["extensions"]}`
% endif
% if "options" in r:
${prefix}
${prefix}    **Options:** `${r["options"]}`
% endif
% endif
</%def>
${render_pointer(pointer_name='/', prefix='')}