import unreal,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');DEST='/Game/VariantZero/Ecology'
tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary;assets=unreal.EditorAssetLibrary
manifest=json.loads((ROOT/'ArtSource/PolyHaven/manifest.json').read_text(encoding='utf-8-sig'))
def load_file(path,name,options=None):
    if assets.does_asset_exist(DEST+'/'+name):return unreal.load_asset(DEST+'/'+name)
    t=unreal.AssetImportTask();t.filename=str(path);t.destination_path=DEST;t.destination_name=name;t.automated=True;t.save=True
    if options:t.options=options
    tools.import_asset_tasks([t]);a=[unreal.load_asset(p) for p in t.imported_object_paths]
    if not a:raise RuntimeError('Import failed '+str(path))
    return a[0]
def static_mesh(path,name):
    ui=unreal.FbxImportUI();ui.automated_import_should_detect_type=False;ui.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    ui.import_as_skeletal=False;ui.import_materials=False;ui.import_textures=False;ui.static_mesh_import_data.combine_meshes=True
    ui.static_mesh_import_data.auto_generate_collision=False
    m=load_file(path,name,ui)
    return m
result={};textures={}
for entry in manifest:
    kind=entry['kind'];name=entry['asset']
    if kind=='mesh':result[name]=static_mesh(entry['file'],'SM_'+name);continue
    tex=load_file(entry['file'],'T_'+name+'_'+kind)
    if kind!='Diffuse':tex.set_editor_property('srgb',False)
    if kind=='nor_dx':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
    tex.set_editor_property('max_texture_size',2048);assets.save_loaded_asset(tex)
    textures[name,kind]=tex
for name in ['fern_02','boulder_01','rock_01','forest_ground_04']:
    path=DEST+'/M_'+name
    m=unreal.load_asset(path) if assets.does_asset_exist(path) else tools.create_asset('M_'+name,DEST,unreal.Material,unreal.MaterialFactoryNew())
    lib.delete_all_material_expressions(m)
    for kind,prop in [('Diffuse',unreal.MaterialProperty.MP_BASE_COLOR),('nor_dx',unreal.MaterialProperty.MP_NORMAL),('Rough',unreal.MaterialProperty.MP_ROUGHNESS),('Alpha',unreal.MaterialProperty.MP_OPACITY_MASK)]:
        if (name,kind) not in textures:continue
        node=lib.create_material_expression(m,unreal.MaterialExpressionTextureSample);node.texture=textures[name,kind]
        node.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if kind=='Diffuse' else unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if kind=='nor_dx' else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
        lib.connect_material_property(node,'RGB' if kind in ['Diffuse','nor_dx'] else 'R',prop)
    if name=='fern_02':m.set_editor_property('two_sided',True);m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
    lib.recompile_material(m);assets.save_loaded_asset(m)
    if name in result:
        mesh=result[name]
        for i in range(len(mesh.get_editor_property('static_materials'))):mesh.set_material(i,m)
        assets.save_loaded_asset(mesh)
for name in ['FieldScanner','InstituteTerrain']:
    result[name]=static_mesh(ROOT/'ArtSource/Ecology'/('SM_'+name+'.fbx'),'SM_'+name)
scanner=result['FieldScanner']
material_names={'Ceramic':'M_Ivory','Grip':'M_Soil','LivingCore':'M_Biolight','Brass':'M_Brass'}
for i,slot in enumerate(scanner.get_editor_property('static_materials')):
    label=str(slot.material_slot_name)
    match=next((v for k,v in material_names.items() if k in label),'M_Ivory')
    scanner.set_material(i,unreal.load_asset('/Game/VariantZero/Environment/Materials/'+match))
assets.save_loaded_asset(scanner)
result['InstituteTerrain'].set_material(0,unreal.load_asset(DEST+'/M_forest_ground_04'));assets.save_loaded_asset(result['InstituteTerrain'])
# Independent material instances retain the source armor detail and avoid tinting skin/hair.
hero=unreal.load_asset('/Game/ParagonLtBelica/Characters/Heroes/Belica/Meshes/Belica')
for index,name in [(0,'MI_EcologistUpper'),(1,'MI_EcologistLower'),(2,'MI_EcologistPad')]:
    parent=hero.get_editor_property('materials')[index].material_interface
    m=unreal.load_asset(DEST+'/'+name) if assets.does_asset_exist(DEST+'/'+name) else tools.create_asset(name,DEST,unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
    lib.set_material_instance_parent(m,parent)
    available=[str(p) for p in lib.get_vector_parameter_names(parent)]
    for param,color in {'PrimaryArmorColor':(.55,.65,.51),'RubberTint':(.025,.07,.045),'LeatherTint':(.08,.13,.07),'TrimHoseTint':(.05,.3,.16),'DeepEmissiveColor':(.02,.42,.24),'ShallowEmissiveColor':(.03,.65,.38),'Accent MetalTint1':(.31,.23,.08)}.items():
        if param in available:lib.set_material_instance_vector_parameter_value(m,param,unreal.LinearColor(*color,1))
    lib.update_material_instance(m);assets.save_loaded_asset(m)
report={n:{'path':m.get_path_name(),'bounds_cm':[m.get_bounds().box_extent.x*2,m.get_bounds().box_extent.y*2,m.get_bounds().box_extent.z*2]} for n,m in result.items()}
(ROOT/'Docs/ecology-import.json').write_text(json.dumps(report,indent=2),encoding='utf8')
unreal.log('VZ_ECOLOGY_IMPORT PASS')
