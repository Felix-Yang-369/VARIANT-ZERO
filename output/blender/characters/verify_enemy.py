import bpy,os,json
s=bpy.context.scene;name=s['character_name'];key=s['asset_key'];out=os.path.dirname(bpy.data.filepath)
root=bpy.data.collections.get(name+' • 角色资产')
checks={
 'correct_name':(key,name) in [('enemy0','苔行者'),('enemy2','疾藤潜行者')],
 'independent_scene':len(bpy.data.scenes)==1,
 'organized_character':bool(root) and len(root.children)==5,
 'has_real_meshes':bool(root) and sum(o.type=='MESH' for o in root.all_objects)>20,
 'packed_reference':any(im.packed_file and im.name==key+'.png' for im in bpy.data.images),
 'no_external_textures':all(im.packed_file for im in bpy.data.images if im.source=='FILE'),
 'gpu_render_setting':s.render.engine=='CYCLES' and s.cycles.device=='GPU',
 'two_cameras':sum(o.type=='CAMERA' for o in s.objects)==2,
 'previews_exist':all(os.path.isfile(os.path.join(out,name+suffix)) for suffix in ['_三维预览_v1.png','_背面预览_v1.png']),
}
result={'character':name,'checks':checks,'passed':all(checks.values())}
with open(os.path.join(out,'verification.json'),'w',encoding='utf-8') as f:json.dump(result,f,ensure_ascii=False,indent=2)
print('VERIFIED',json.dumps(result,ensure_ascii=False));assert result['passed']
