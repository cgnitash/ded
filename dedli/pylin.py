
from ete3 import Tree , TreeStyle , NodeStyle , faces , AttrFace
import csv

#t = Tree("(A:1,(B:1,(E:1,D:1):0.5):0.5);" )
#t.render("mytree.png", w=183, units="mm")

t = Tree("(0);" )

all_orgs = []
with open('testlineage.csv') as csvfile:
    reader = csv.DictReader(csvfile)
    for org in reader:
        all_orgs.append((org['id'],org['ancestor_id'],org['recorded_at'],org['on_lod']))
with open('testunpruned.csv') as csvfile:
    reader = csv.DictReader(csvfile)
    for org in reader:
        all_orgs.append((org['id'],org['ancestor_id'],'101','0'))

all_orgs.sort(key=lambda t: int(t[0]))
#print (all_orgs)


style = NodeStyle()
style["fgcolor"] = "#00f000"
#style["bgcolor"] = "lightred"
#style["size"] = 10
style["shape"] = "circle"
style["vt_line_color"] = "#0000aa"
style["hz_line_color"] = "#0000aa"
style["vt_line_width"] = 50
style["hz_line_width"] = 50
style["vt_line_type"] = 2 # 0 solid, 1 dashed, 2 dotted
style["hz_line_type"] = 2

for org in all_orgs:
    #print (org)
    new_node = Tree('(' + org[0]  + ');') #+ ':' + str(int(org[2])^1)
    if (org[3] == '1'):
        #print (org)
        new_node.set_style(style) 
    anc = t.search_nodes(name=org[1])[0]
    anc.add_child(new_node)

#for n in t.traverse():
    #print (n.name)
    #if all_orgs[int(n.name)][3] == '1':
        #n.img_style = style2

style2 = NodeStyle()
style2["fgcolor"] = "#000000"
style2["shape"] = "circle"
style2["vt_line_color"] = "#0000aa"
style2["hz_line_color"] = "#0000aa"
style2["vt_line_width"] = 5
style2["hz_line_width"] = 5
style2["vt_line_type"] = 1 # 0 solid, 1 dashed, 2 dotted
style2["hz_line_type"] = 1
for l in t.iter_leaves():
    l.img_style = style2

ts = TreeStyle()
ts.show_leaf_name = True
#ts.rotation = 90
ts.mode = 'c'
ts.arc_start = 180
ts.arc_span = 350
#t.show(tree_style=ts)
#t.show()


t = Tree()
t.populate(8)
style2 = NodeStyle()
style2["fgcolor"] = "darkred"
style2["shape"] = "circle"
style2["vt_line_color"] = "green"
style2["hz_line_color"] = "red"
style2["vt_line_width"] = 5
style2["hz_line_width"] = 5
style2["vt_line_type"] = 1 # 0 solid, 1 dashed, 2 dotted
style2["hz_line_type"] = 1
for l in t.iter_leaves():
    l.img_style = style2
ts = TreeStyle()
ts.show_leaf_name = False
#
t.show(tree_style=ts)

