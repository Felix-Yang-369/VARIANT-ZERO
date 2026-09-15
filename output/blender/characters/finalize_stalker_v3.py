"""Reopen, validate, save and render the independent reconstruction."""
import bpy, os, json, math
from mathutils import Vector
base=os.path.dirname(os.path.abspath(__file__))
out=os.path.join(base,'enemy2_疾藤潜行者')
path=os.path.join(out,'疾藤潜行者_enemy2_v3.blend')
bpy.ops.wm.open_mainfile(filepath=path)
s=bpy.context.scene
root=bpy.data.collections['疾藤潜行者 • 角色资产']
members=list(root.all_objects)
assert len(root.children)==5
assert sum(o.type=='MESH' for o in members)>50
assert len([o for o in s.objects if o.type=='CAMERA'])==4
assert all(not o.library for o in members)
images=[im for im in bpy.data.images if im.source=='FILE' and im.users]
assert len(images)>=2 and all(im.packed_file for im in images)
assert all(math.isfinite(v) for o in members for v in o.matrix_world.translation)
exec(compile(open(os.path.join(base,'..','configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s['revision']='v3 • detailed static reconstruction'
s['reference_note']='Original enemy2.png is authoritative; ImageGen side/back sheet is supplementary interpretation.'
root['revision']='v3 • detailed static reconstruction'
root.asset_mark();root.asset_data.description='疾藤潜行者 / enemy2；独立静态高细节角色。按根集合 Link；摄影棚与参考图不属于角色集合。'
s.camera=bpy.data.objects['CAM 01 • 三分之四']
s.render.resolution_x=1600;s.render.resolution_y=1846;s.render.resolution_percentage=100
s.cycles.samples=128
s.render.filepath=os.path.join(out,'疾藤潜行者_v3_精修预览.png')
bpy.data.orphans_purge(do_recursive=True)
bpy.ops.wm.save_as_mainfile(filepath=path)
# Reopen the newly saved delivery, rather than validating only the in-memory scene.
bpy.ops.wm.open_mainfile(filepath=path)
s=bpy.context.scene;root=bpy.data.collections['疾藤潜行者 • 角色资产'];members=list(root.all_objects)
report={'file':path,'reopened':True,'asset_collection':root.name,'asset_objects':len(members),
 'mesh_objects':sum(o.type=='MESH' for o in members),'curve_objects':sum(o.type=='CURVE' for o in members),
 'base_mesh_vertices':sum(len(o.data.vertices) for o in members if o.type=='MESH'),
 'base_mesh_faces':sum(len(o.data.polygons) for o in members if o.type=='MESH'),
 'groups':[c.name for c in root.children],'packed_reference_images':[im.name for im in bpy.data.images if im.source=='FILE' and im.packed_file],
 'render_backend':bpy.context.preferences.addons['cycles'].preferences.compute_device_type,
 'render_device':s.cycles.device,'cameras':[o.name for o in s.objects if o.type=='CAMERA'],
 'limitations':['Static posed reconstruction; no rig or animation','High-detail source; no game-ready retopology or baked LOD','ImageGen side and back views are inferred design reference, not official orthographic views']}
with open(os.path.join(out,'verification-v3.json'),'w',encoding='utf-8') as f:json.dump(report,f,ensure_ascii=False,indent=2)
print('VERIFIED_REOPEN',json.dumps(report,ensure_ascii=False),flush=True)
bpy.ops.render.render(write_still=True)
print('DELIVERY_RENDER_COMPLETE',flush=True)
