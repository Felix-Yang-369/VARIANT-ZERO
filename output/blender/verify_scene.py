"""Run inside Blender after opening the saved .blend."""
import bpy, json, os
s=bpy.context.scene
required=['01 • 原初生态研究所 | 温室建筑','04 • Glass podium | 玻璃培养台','05 • V-001 原豆母体 | 豆科']
checks={
    'official_scene_name':s.name=='原初生态研究所 · V-001 原豆母体',
    'official_character_name':s.get('Character ID')=='V-001' and s.get('Character Name')=='原豆母体',
    'official_file_name':os.path.basename(bpy.data.filepath)=='原初生态研究所_V-001_原豆母体_v1.blend',
    'single_scene':len(bpy.data.scenes)==1,
    'three_subjects_have_geometry':all(any(o.type=='MESH' for o in bpy.data.collections[n].objects) for n in required),
    'three_reference_images_packed':sum(bool(i.packed_file) for i in bpy.data.images)>=3,
    'three_cameras':sum(o.type=='CAMERA' for o in s.objects)==3,
    'main_camera_active':s.camera.name.startswith('CAM 01'),
    'reference_boards_hidden':bpy.data.collections['07 • Packed concept references (hidden)'].hide_render,
    'no_external_file_textures':all(i.packed_file for i in bpy.data.images if i.source=='FILE'),
}
result={'checks':checks,'passed':all(checks.values()),'objects':len(s.objects),'collections':{n:len(bpy.data.collections[n].objects) for n in required}}
with open(os.path.join(os.path.dirname(bpy.data.filepath),'verification.json'),'w',encoding='utf-8') as f:json.dump(result,f,ensure_ascii=False,indent=2)
print('VERIFICATION',json.dumps(result,ensure_ascii=False))
assert result['passed'], 'Scene integrity check failed'
