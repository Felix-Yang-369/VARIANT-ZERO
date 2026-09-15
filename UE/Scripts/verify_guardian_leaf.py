import bpy,json
from pathlib import Path
OUT=Path(r'D:\Projects\VariantZeroUE\ArtSource\GuardianLeaf')
bpy.ops.wm.open_mainfile(filepath=str(OUT/'GuardianLeaf.blend'))
o=bpy.data.objects['SM_GuardianLeaf'];o.data.calc_loop_triangles();before=len(o.data.loop_triangles);dims=list(o.dimensions)
assert before==2248 and len(o.data.materials)==2
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(OUT/'SM_GuardianLeaf.fbx'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(meshes)==1
o=meshes[0];o.data.calc_loop_triangles();after=len(o.data.loop_triangles)
assert after==before and all(abs(a-b)<.001 for a,b in zip(dims,o.dimensions))
(OUT/'verification.json').write_text(json.dumps({'blend_reopened':True,'fbx_roundtrip':True,'triangles':after,'dimensions_m':list(o.dimensions)},indent=2))
print('VZ_GUARDIAN_LEAF_VERIFY_PASS')
