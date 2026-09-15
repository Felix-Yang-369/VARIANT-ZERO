import bpy,os,json
s=bpy.context.scene
instances=[o for o in s.objects if o.instance_type=='COLLECTION']
expected={'环境 • 原初生态研究所','角色 • V-001 原豆母体','角色 • 苔行者','角色 • 疾藤潜行者'}
checks={
 'four_collection_instances':{o.name for o in instances}==expected,
 'real_external_links':all(o.instance_collection and o.instance_collection.library for o in instances),
 'local_transform_controls':all(o.library is None for o in instances),
 'three_source_files':len(bpy.data.libraries)==3,
 'relative_links':all(lib.filepath.startswith('//') for lib in bpy.data.libraries),
 'source_files_exist':all(os.path.isfile(bpy.path.abspath(lib.filepath)) for lib in bpy.data.libraries),
 'source_studios_excluded':all('预览' not in o.name and not o.name.startswith('CAM ') for i in instances for o in i.instance_collection.all_objects),
 'local_lighting':all(o.library is None for o in s.objects if o.type=='LIGHT'),
 'local_camera':s.camera is not None and s.camera.library is None,
 'gpu_setting':s.render.engine=='CYCLES' and s.cycles.device=='GPU',
}
result={'checks':checks,'passed':all(checks.values()),'linked_instances':[{'name':o.name,'objects':len(o.instance_collection.all_objects),'source':o.instance_collection.library.filepath} for o in instances]}
with open(os.path.join(os.path.dirname(bpy.data.filepath),'verification.json'),'w',encoding='utf-8') as f:json.dump(result,f,ensure_ascii=False,indent=2)
print('LINK_VERIFICATION',json.dumps(result,ensure_ascii=False),flush=True)
assert result['passed']
