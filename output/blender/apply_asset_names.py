"""Apply current project names without changing geometry or rendering."""
import bpy, os, json
out=os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(out,'asset-names.json'),encoding='utf-8') as f:names=json.load(f)
s=bpy.context.scene
before={o.as_pointer():(o.type,tuple(o.matrix_world[:]),len(o.data.vertices) if o.type=='MESH' else None) for o in s.objects}
renames={
 '01 • Conservatory | 温室建筑':'01 • 原初生态研究所 | 温室建筑',
 '05 • V-001 原豆母体':'05 • V-001 原豆母体 | 豆科',
 'CAM 01 • Complete sanctuary':'CAM 01 • 原初生态研究所全景',
 'CAM 02 • Seed and podium':'CAM 02 • 原豆母体与培养台',
 'CAM 03 • Character front':'CAM 03 • 原豆母体正面',
}
for bank in (bpy.data.collections,bpy.data.objects):
    for old,new in renames.items():
        item=bank.get(old)
        if item:item.name=new
for o in s.objects:
    if o.name.startswith('V-001 •'):o.name=o.name.replace('V-001 •','V-001 原豆母体 •',1)
s.name='原初生态研究所 · V-001 原豆母体'
s['Scene Name']=names['scene']['name'];s['Scene English']=names['scene']['english']
s['Character ID']='V-001';s['Character Name']='原豆母体';s['Family']='legume / 豆科';s['Growth Stage']='原初母体'
hero=bpy.data.collections['05 • V-001 原豆母体 | 豆科']
hero['species_id']='V-001';hero['species_name']='原豆母体';hero['family']='legume';hero['growth_stage']='原初母体'
s.render.filepath=os.path.join(out,names['previews'][0])
txt=bpy.data.texts.get('READ ME • 场景说明')
if txt:
    text=txt.as_string()
    if '正式命名' not in text:
        txt.write('\n正式命名：原初生态研究所 / PRIMORDIAL CONSERVATORY\n角色：V-001 原豆母体 / 豆科 / 原初母体\n命名依据：src/variant/ui/home.ts 与 src/variant/content.ts。\n')
after={o.as_pointer():(o.type,tuple(o.matrix_world[:]),len(o.data.vertices) if o.type=='MESH' else None) for o in s.objects}
assert before==after,'Unexpected geometry or transform changes'
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out,names['file']))
stats={'scene':s.name,'file':names['file'],'objects':len(s.objects),'mesh_objects':sum(o.type=='MESH' for o in s.objects),'cameras':[o.name for o in s.objects if o.type=='CAMERA'],'packed_images':[im.name for im in bpy.data.images if im.packed_file],'character':names['character']}
with open(os.path.join(out,'scene-manifest.json'),'w',encoding='utf-8') as f:json.dump(stats,f,ensure_ascii=False,indent=2)
print('RENAMED_WITH_GEOMETRY_UNCHANGED',json.dumps(stats,ensure_ascii=False),flush=True)
