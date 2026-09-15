import bpy,os,json
s=bpy.context.scene;path=bpy.data.filepath;out=os.path.dirname(path)
if s.get('asset_key'):
    s.render.filepath=os.path.join(out,s['character_name']+'_三维精修预览_v2.png')
    bpy.ops.wm.save_as_mainfile(filepath=path)
checks={
 'versioned_file':path.endswith('_v2.blend'),
 'revision_recorded':bool(s.get('asset_revision')),
 'real_added_geometry':any(o.type=='MESH' and o.name.startswith('v2 •') for o in s.objects),
 'gpu_setting':s.render.engine=='CYCLES' and s.cycles.device=='GPU',
 'packed_reference':any(im.packed_file for im in bpy.data.images),
 'no_external_texture_dependency':all(im.packed_file for im in bpy.data.images if im.source=='FILE'),
 'v1_preserved':os.path.isfile(path.replace('_v2.blend','_v1.blend')),
 'v2_render_path':'v2.png' in s.render.filepath,
}
report={'file':os.path.basename(path),'checks':checks,'passed':all(checks.values()),'objects':len(s.objects),'vertices':sum(len(o.data.vertices) for o in s.objects if o.type=='MESH')}
with open(os.path.join(out,'verification-v2.json'),'w',encoding='utf-8') as f:json.dump(report,f,ensure_ascii=False,indent=2)
print('V2_VERIFICATION',json.dumps(report,ensure_ascii=False));assert report['passed']
