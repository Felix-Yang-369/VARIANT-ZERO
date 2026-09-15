import unreal, json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');SRC=ROOT/'ArtSource/Moth/v1';DEST='/Game/VariantZero/Characters/Moth'
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
assets=task('SK_Moth.fbx','SK_Moth',ui)
mesh=next(x for x in assets if isinstance(x,unreal.SkeletalMesh));skeleton=mesh.get_editor_property('skeleton')
if not skeleton:raise RuntimeError('Imported mesh has no skeleton')
unreal.EditorAssetLibrary.save_loaded_asset(skeleton,False)
physics=mesh.get_editor_property('physics_asset')
if physics:unreal.EditorAssetLibrary.save_loaded_asset(physics,False)
for name in ['A_Moth_Hover','A_Moth_Fly']:
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
for channel in ['BaseColor','Normal','ORM']:
    tex=task('T_Moth_'+channel+'.png','T_Moth_'+channel)[0];textures[channel]=tex
    tex.set_editor_property('max_texture_size',2048)
    if channel!='BaseColor':tex.set_editor_property('srgb',False)
    if channel=='Normal':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP);tex.set_editor_property('flip_green_channel',True)
    unreal.EditorAssetLibrary.save_loaded_asset(tex)
mat=unreal.load_asset(DEST+'/M_Moth') or tools.create_asset('M_Moth',DEST,unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(mat)
for channel in textures:
    n=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);n.texture=textures[channel]
    n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if channel=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR if channel=='ORM' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
    if channel=='ORM':
        for output,prop in [('R',unreal.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',unreal.MaterialProperty.MP_ROUGHNESS),('B',unreal.MaterialProperty.MP_METALLIC)]:lib.connect_material_property(n,output,prop)
    else:lib.connect_material_property(n,'RGB',unreal.MaterialProperty.MP_NORMAL if channel=='Normal' else unreal.MaterialProperty.MP_BASE_COLOR)
mat.set_editor_property('two_sided',True);lib.set_material_usage(mat,unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
slots=mesh.get_editor_property('materials');slots[0].material_interface=mat;mesh.set_editor_property('materials',slots)
sub=unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
if not sub.regenerate_lod(mesh,3,False,False):raise RuntimeError('Moth LOD generation failed')
unreal.EditorAssetLibrary.save_loaded_asset(mesh);unreal.EditorAssetLibrary.save_directory(DEST,False,True)
report={'mesh':mesh.get_path_name(),'skeleton':skeleton.get_path_name(),'lod_vertices':[sub.get_num_verts(mesh,i) for i in range(3)],'animations':['A_Moth_Hover','A_Moth_Fly'],'status':'imported candidate; runtime verification pending'}
(SRC/'ue-import-report.json').write_text(json.dumps(report,indent=2));unreal.log('VZ_MOTH_IMPORT_OK '+str(report))

