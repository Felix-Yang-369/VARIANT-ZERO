import bpy,json,math
from pathlib import Path
out=Path(r'D:\Projects\VariantZeroUE\ArtSource\Chuya\v1')
bpy.ops.wm.open_mainfile(filepath=str(out/'Chuya_bound_v1.blend'))
mesh=bpy.data.objects['SK_Chuya'];arm=bpy.data.objects['Chuya_Rig'];scene=bpy.context.scene
checks={'armature_modifier':any(m.type=='ARMATURE' and m.object==arm for m in mesh.modifiers),'all_vertices_weighted':all(v.groups and abs(sum(g.weight for g in v.groups)-1)<.001 for v in mesh.data.vertices),'eleven_bones':len(arm.data.bones)==11,'uv_present':len(mesh.data.uv_layers)>0,'textures_packed':all(i.packed_file for i in bpy.data.images if i.name.startswith('T_Chuya_')),'fbx_exports_present':all((out/(name+'.fbx')).exists() for name in ['SK_Chuya','A_Chuya_Idle','A_Chuya_Walk'])}
positions=[]
for frame in [1,16,31,46]:
    scene.frame_set(frame);deps=bpy.context.evaluated_depsgraph_get();evaluated=mesh.evaluated_get(deps);coords=[v.co.copy() for v in evaluated.data.vertices];positions.append(coords)
checks['animation_deforms_mesh']=max((a-b).length for a,b in zip(positions[0],positions[1]))>.001
checks['finite_deformation']=all(math.isfinite(v) for coords in positions for p in coords for v in p)
checks['bounded_deformation']=max((a-b).length for coords in positions[1:] for a,b in zip(positions[0],coords))<.25
(out/'reopen-verification.json').write_text(json.dumps({'checks':checks,'passed':all(checks.values())},indent=2),encoding='utf-8')
assert all(checks.values()),checks
print('VZ_CHUYA_REOPEN_PASS',checks)
