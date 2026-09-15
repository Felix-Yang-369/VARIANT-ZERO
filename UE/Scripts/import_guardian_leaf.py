import unreal,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');DEST='/Game/VariantZero/Guardians'
path=DEST+'/SM_GuardianLeaf'
if unreal.EditorAssetLibrary.does_asset_exist(path):raise RuntimeError('Preserve existing imported mesh')
task=unreal.AssetImportTask();task.filename=str(ROOT/'ArtSource/GuardianLeaf/SM_GuardianLeaf.fbx');task.destination_path=DEST;task.destination_name='SM_GuardianLeaf';task.automated=True;task.save=True
ui=unreal.FbxImportUI();ui.automated_import_should_detect_type=False;ui.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;ui.import_as_skeletal=False;ui.import_materials=False;ui.import_textures=False;ui.static_mesh_import_data.combine_meshes=True;ui.static_mesh_import_data.auto_generate_collision=False
task.options=ui;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(path);assert mesh
slots=[]
for i,slot in enumerate(mesh.get_editor_property('static_materials')):
    name=str(slot.material_slot_name);slots.append(name)
    mesh.set_material(i,unreal.load_asset('/Game/VariantZero/Environment/Materials/'+('M_Biolight' if 'Vein' in name else 'M_LivingLeaf')))
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
b=mesh.get_bounds();dims=[b.box_extent.x*2,b.box_extent.y*2,b.box_extent.z*2];assert 90<dims[2]<110
(ROOT/'Docs/guardian-leaf-import.json').write_text(json.dumps({'mesh':path,'dimensions_cm':dims,'material_slots':slots,'collision':'none; gameplay remains on guardian core'},indent=2))
unreal.log('VZ_GUARDIAN_LEAF_IMPORT_PASS')
