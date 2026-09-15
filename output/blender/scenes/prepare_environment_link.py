"""Add an asset collection to the existing source without moving any geometry."""
import bpy
s=bpy.context.scene
before={o.as_pointer():o.matrix_world.copy() for o in s.objects}
name='原初生态研究所 • 环境资产'
root=bpy.data.collections.get(name)
if root is None:
    root=bpy.data.collections.new(name);s.collection.children.link(root)
    for n in ['01 • 原初生态研究所 | 温室建筑','02 • Cultivation aperture | 椭圆培养装置','03 • Living network | 藤蔓与植物','04 • Glass podium | 玻璃培养台']:
        c=bpy.data.collections[n]
        root.children.link(c)
        if c.name in s.collection.children:s.collection.children.unlink(c)
root['asset_type']='environment';root['includes']='温室 / 椭圆培养装置 / 植被 / 培养台'
assert before=={o.as_pointer():o.matrix_world.copy() for o in s.objects}
bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
print('ENVIRONMENT_COLLECTION_READY',len(root.all_objects),flush=True)
