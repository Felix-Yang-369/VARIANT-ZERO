import bpy,json
from pathlib import Path
source = Path(r'D:\Projects\零号变种\output\blender\原初生态研究所_V-001_原豆母体_v2.blend')
with bpy.data.libraries.load(str(source),link=False) as (src,dst):
    print('VZ_COLLECTIONS',json.dumps(src.collections,ensure_ascii=False))
    dst.collections=[n for n in src.collections if '原豆' in n or 'HERO' in n or n.startswith('05')]
report=[]
for c in dst.collections:
    if c:
        bpy.context.scene.collection.children.link(c)
        for o in c.all_objects:
            report.append({'name':o.name,'type':o.type,'location':list(o.location),'dimensions':list(o.dimensions),'vertices':len(o.data.vertices) if o.type=='MESH' else 0})
out=Path(r'D:\Projects\VariantZeroUE\ArtSource\Chuya')
out.mkdir(parents=True,exist_ok=True)
(out/'source-inspection.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print('VZ_OBJECT_COUNT',len(report))
