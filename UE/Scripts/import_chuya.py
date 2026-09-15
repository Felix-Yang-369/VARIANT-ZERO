import unreal, json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');SRC=ROOT/'ArtSource/Chuya/v1';DEST='/Game/VariantZero/Characters/Chuya'
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
tools=unreal.AssetToolsHelpers.get_asset_tools()
def task(file,name,options=None):
    t=unreal.AssetImportTask();t.filename=str(SRC/file);t.destination_path=DEST;t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=True
    if options:t.options=options
    tools.import_asset_tasks([t])
    paths=t.imported_object_paths
    unreal.log('VZ_IMPORTED '+str(paths))
    if not paths:raise RuntimeError('Empty import: '+file)
    return [unreal.load_asset(p) for p in paths]
ui=unreal.FbxImportUI();ui.set_editor_property('automated_import_should_detect_type',False);ui.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_SKELETAL_MESH)
ui.import_as_skeletal=True;ui.import_mesh=True;ui.import_animations=False;ui.import_materials=False;ui.import_textures=False;ui.create_physics_asset=True
assets=task('SK_Chuya.fbx','SK_Chuya',ui)
mesh=next(x for x in assets if isinstance(x,unreal.SkeletalMesh));skeleton=mesh.get_editor_property('skeleton')
if not skeleton:raise RuntimeError('Imported mesh has no skeleton')
unreal.EditorAssetLibrary.save_loaded_asset(skeleton,False)
physics=mesh.get_editor_property('physics_asset')
if physics:unreal.EditorAssetLibrary.save_loaded_asset(physics,False)
for name in ['A_Chuya_Idle','A_Chuya_Walk']:
    if unreal.EditorAssetLibrary.does_asset_exist(DEST+'/'+name):
        existing=unreal.load_asset(DEST+'/'+name)
        if existing.get_editor_property('skeleton') == skeleton:
            continue  # Preserve verified animation data; revisions should use a new asset version.
        raise RuntimeError('Existing animation has an invalid skeleton; preserve it before a fresh import: '+name)
    ui=unreal.FbxImportUI();ui.automated_import_should_detect_type=False;ui.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION
    ui.import_as_skeletal=True;ui.import_mesh=False;ui.import_animations=True;ui.skeleton=skeleton;ui.import_materials=False;ui.import_textures=False
    animations=task(name+'.fbx',name,ui)
    if not animations[0].get_editor_property('skeleton'):raise RuntimeError('Animation skeleton not retained: '+name)
textures={}
for channel in ['BaseColor','Normal','Roughness','Emission']:
    tex=task('T_Chuya_'+channel+'.png','T_Chuya_'+channel)[0];textures[channel]=tex
    if channel in ['Normal','Roughness']:tex.set_editor_property('srgb',False)
    if channel=='Normal':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP);tex.set_editor_property('flip_green_channel',True)
    unreal.EditorAssetLibrary.save_loaded_asset(tex)
material=unreal.load_asset(DEST+'/M_Chuya') or tools.create_asset('M_Chuya',DEST,unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(material)
for channel,prop in [('BaseColor',unreal.MaterialProperty.MP_BASE_COLOR),('Normal',unreal.MaterialProperty.MP_NORMAL),('Roughness',unreal.MaterialProperty.MP_ROUGHNESS),('Emission',unreal.MaterialProperty.MP_EMISSIVE_COLOR)]:
    n=lib.create_material_expression(material,unreal.MaterialExpressionTextureSample);n.texture=textures[channel]
    n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if channel=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR if channel=='Roughness' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
    lib.connect_material_property(n,'RGB' if channel!='Roughness' else 'R',prop)
material.set_editor_property('two_sided',True);lib.set_material_usage(material,unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH);lib.recompile_material(material);unreal.EditorAssetLibrary.save_loaded_asset(material)
slots=mesh.get_editor_property('materials');slot=slots[0];slot.material_interface=material;slots[0]=slot;mesh.set_editor_property('materials',slots)
sub=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
if not sub.regenerate_lod(mesh,3,False,False):raise RuntimeError('Could not generate companion LODs')
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
unreal.EditorAssetLibrary.save_directory(DEST,False,True)
report={'mesh':mesh.get_path_name(),'skeleton':skeleton.get_path_name(),'materials':len(slots),'lod_vertices':[sub.get_num_verts(mesh,i) for i in range(3)],'animations':[str(a.asset_name) for a in unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_path(DEST) if 'AnimSequence' in str(a.asset_class_path)],'status':'imported candidate; runtime deformation check pending'}
(SRC/'ue-import-report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');unreal.log('VZ_CHUYA_IMPORT_OK '+str(report))
