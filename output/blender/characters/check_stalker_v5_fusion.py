import bpy, bmesh, os, json
out=os.path.join(os.path.dirname(os.path.abspath(__file__)),'enemy2_疾藤潜行者')
bpy.ops.wm.open_mainfile(filepath=os.path.join(out,'疾藤潜行者_enemy2_v5.blend'))
objects=[o for o in bpy.data.collections['疾藤潜行者 • 角色资产'].all_objects if '连续融合根结' in o.name]
assert len(objects)==4
results=[]
for ob in objects:
 bm=bmesh.new();bm.from_mesh(ob.data)
 remaining=set(bm.verts);components=[]
 while remaining:
  first=remaining.pop();stack=[first];count=0
  while stack:
   v=stack.pop();count+=1
   for edge in v.link_edges:
    other=edge.other_vert(v)
    if other in remaining:remaining.remove(other);stack.append(other)
  components.append(count)
 open_edges=sum(not e.is_manifold for e in bm.edges)
 results.append({'name':ob.name,'connected_components':len(components),'component_vertex_counts':components,'nonmanifold_edges':open_edges})
 assert len(components)==1,(ob.name,components)
 assert open_edges==0,(ob.name,open_edges)
 bm.free()
with open(os.path.join(out,'fusion-verification-v5.json'),'w',encoding='utf-8') as f:json.dump(results,f,ensure_ascii=False,indent=2)
print('FUSED_MESH_CHECK_PASS',json.dumps(results,ensure_ascii=False),flush=True)
