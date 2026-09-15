"""Rebuild the same four-instance composition against refined v2 sources."""
import os
path=os.path.join(os.path.dirname(os.path.abspath(__file__)),'build_linked_scene.py')
with open(path,encoding='utf-8') as f:code=f.read()
code=code.replace('_v1','_v2').replace('linked-assets.json','linked-assets-v2.json')
code=code.replace('s.cycles.samples=64','s.cycles.samples=128')
code=code.replace('s.render.resolution_x=1800;s.render.resolution_y=1200','s.render.resolution_x=2400;s.render.resolution_y=1600')
code=code.replace('aim(cam,(0,1.4,3.25))','aim(cam,(0,1.4,3.0))')
code=code.replace('s.world.use_nodes=True;p=s.world.node_tree.nodes.get(\'Background\')','s.world.use_nodes=True;p=next(n for n in s.world.node_tree.nodes if n.type==\'BACKGROUND\')')
exec(compile(code,path,'exec'))
