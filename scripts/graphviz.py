import matplotlib.cm
import random
import matplotlib
import json
import pydot


with open('data/default_rules.json')as fp:
    rules = json.load(fp)

def parse_tree(rules, mark_required = False):
    tree = dict()
    for r in rules:
        pt = r['pointer']
        pt = pt.replace('/*','')

        path = pt.split('/')
        if pt == '/':
            path = ['']

        subtree = tree
        for k in path:
            if k not in subtree:
                subtree[k] = dict()
            subtree = subtree[k]
        req, opt = (getter(r,'required'), getter(r, 'optional'))
        for k in req:
            if k not in subtree:
                subtree[k] = {}
            if mark_required:
                subtree[k]['R'] = '0'
        for k in opt:
            if k not in subtree:
                subtree[k] = {}
    return tree

def rename(node):
    for k in list(node.keys()):
        v = node[k]
        if k == 'R':
            continue
        if k in rename.used_names:
            rename.used_names[k] += 1
            node[k+' '*(used_names[k])] = v
            node.pop(k)
        else:
            rename.used_names[k] = 0
        rename(v)
rename.used_names = dict()




def draw(parent_name, child_name, req = False):
    c = matplotlib.colors.rgb2hex(matplotlib.cm.tab10(random.random()))
    if req:
        args = dict(color='black')
    else:
        args = dict(color=c, style='dashed')
    edge = pydot.Edge(parent_name, child_name, **args)
    graph.add_edge(edge)

def visit(node, parent=None):
    for k, v in node.items():
        if k == 'R':
            continue
        if isinstance(v, dict):
            if parent:
                draw(parent, k, 'R' in v)
            visit(v, k)
        else:
            draw(parent, k)


tree = parse_tree(rules, True)
rename(tree)
graph = pydot.Dot(graph_type='graph',rankdir='LR')
visit(tree[''], 'PolyFEM')
graph.write_png('example1_graph.png')
