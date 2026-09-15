import bpy,json,math
from pathlib import Path
R=Path(r'D:\Projects\VariantZeroUE\ArtSource\Moth\v1')
bpy.ops.wm.open_mainfile(filepath=str(R/'Moth_bound.blend'))
m=bpy.data.objects['SK_Moth'];arm=bpy.data.objects['Moth_Rig'];scene=bpy.context.scene
assert len(arm.data.bones)==8 and all(v.groups for v in m.data.vertices)
def positions(frame):
 scene.frame_set(frame);bpy.context.view_layer.update();obj=m.evaluated_get(bpy.context.evaluated_depsgraph_get());mesh=obj.to_mesh();p=[v.co.copy() for v in mesh.vertices];obj.to_mesh_clear();return p
a=positions(1);b=positions(5);c=positions(31)
loop=max((x-y).length for x,y in zip(a,c));motion=max((x-y).length for x,y in zip(a,b))
assert loop<.0001 and motion>.01 and all(math.isfinite(v) for p in b for v in p)
assert max(p.length for p in b)<1
scene.frame_set(12);scene.render.filepath=str(R/'Moth_reopened.png');bpy.ops.render.render(write_still=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);bpy.ops.import_scene.fbx(filepath=str(R/'SK_Moth.fbx'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];rigs=[o for o in bpy.context.scene.objects if o.type=='ARMATURE'];assert meshes and rigs
assert len(rigs[0].data.bones)>=8 and meshes[0].data.uv_layers and all(v.groups for v in meshes[0].data.vertices)
report={'saved_scene_reopened':True,'bones':8,'loop_error_m':loop,'motion_m':motion,'fbx_reimport_bones':len(rigs[0].data.bones),'fbx_vertices':len(meshes[0].data.vertices),'status':'PASS'}
(R/'verification.json').write_text(json.dumps(report,indent=2));print('VZ_MOTH_ROUNDTRIP_PASS',report,flush=True)
