## _*_ coding: utf-8 _*_
## ---
## template: no_toc.html
## ---
<!--Automatically generated using JSON Spec Engine-->
<%def name="render_pointer(pointer_name, prefix)">
${make_rules(rs=ijson.rules_from_pointer(pointer_name), prefix=prefix)}
</%def>
## ============================================================================
<%def name="make_rules(rs, prefix)">
<%
if len(rs) > 0:
    rs[0]['pointer_last'] = rs[0]['pointer'].rsplit('/', 1)[-1]
    if rs[0]['pointer_last'] == '*':
        tmp = rs[0]['pointer'].replace('/*','')
        rs[0]['pointer_last'] = tmp.rsplit('/', 1)[-1]
%>
% if len(rs) == 1: ## single rules for a pointer use a cell
${make_rule(r=rs[0], prefix=prefix, tag="!!! json-spec" if len(prefix) == 0 else "??? json-spec")}
% elif len(rs) > 1: ## multiple rules use a collapsible block
${prefix}??? json-spec "`${rs[0]['pointer']}`"
${prefix}    ```
${prefix}    ${rs[0]['pointer']}
${prefix}    ```
## ${prefix}    <h2>Description</h2>
## ${prefix}    ${rs[0]['doc']}
% for rule in rs:
${make_rule(r=rule, prefix=prefix + "    ", tag="===")}
% endfor
% endif
</%def>
## ============================================================================
<%def name="make_rule(r, prefix, tag)">
<%
if not 'doc' in r:
    r['doc'] = 'FIXME: Missing documentation in the specification.'
r['pointer_last'] = r['pointer'].rsplit('/', 1)[-1]
if r['pointer_last'] == '*':
    tmp = r['pointer'].replace('/*','')
    r['pointer_last'] = tmp.rsplit('/', 1)[-1]
r['pointer_short'] = r['pointer']
%>
## ----------------------------------------------------------------------------
% if r['type'] == "list": ## List
${prefix}${tag} "${r.get("type_name", f"`{r['pointer_short']}` (`{r['type']}`)")}"
% if tag != "===":
${prefix}    ```
${prefix}    ${r['pointer']}
${prefix}    ```
%endif
${prefix}    <h2>Description</h2>
${prefix}    ${r['doc']}
${render_pointer(
    pointer_name = (r['pointer']+'*') if r['pointer'] == '/' else (r['pointer']+'/'+'*'),
    prefix = prefix + "    ")}
## ----------------------------------------------------------------------------
% else:
${prefix}${tag} "${r.get("type_name", f"`{r['pointer_short']}` (`{r['type']}`)")}"
% if tag != "===":
${prefix}    ```
${prefix}    ${r['pointer']}
${prefix}    ```
%endif
% if "type_name" in r:
${prefix}
${prefix}    **Type**: ${r["type_name"]}
% endif
${prefix}    <h2>Description</h2>
${prefix}    ${r['doc']}
% if "default" in r:
${prefix}
${prefix}    **Default**: `${repr(r["default"])}`
% endif
% if "min" in r or "max" in r:
${prefix}
${prefix}    **Range:** `[${r.get("min", "-inf")}, ${r.get("max", "inf")}]`
% endif
% if "extensions" in r:
${prefix}
${prefix}    **Extensions:** `${r["extensions"]}`
% endif
% if "options" in r:
${prefix}
${prefix}    **Options:** `${r["options"]}`
% endif
% if 'required' in r:
${prefix}    <h2>Required</h2>
% for fname in r['required']:
${render_pointer(
    pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname),
    prefix = prefix + "    ")}
% endfor
% endif
% if 'optional' in r:
${prefix}    <h2>Optional</h2>
% for fname in r['optional']:
${render_pointer(
    pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname),
    prefix = prefix + "    ")}
% endfor
% endif
% endif
</%def>
${render_pointer(pointer_name='/', prefix='')}