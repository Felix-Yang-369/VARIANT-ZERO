"""Create a master with four linked collection instances and local presentation controls."""
import bpy,os,json,math
from mathutils import Vector
BASE=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT=os.path.join(BASE,'scenes','原初生态研究所_三角色展示');os.makedirs(OUT,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
for c in list(bpy.data.collections):bpy.data.collections.remove(c)
s=bpy.context.scene;s.name='原初生态研究所 • 三角色 Link 总场景'
assets=bpy.data.collections.new('01 • 链接资产 (选中实例调整位置)');s.collection.children.link(assets)
local=bpy.data.collections.new('02 • 总场景灯光与相机');s.collection.children.link(local)
sources=[
 ('环境 • 原初生态研究所','原初生态研究所_V-001_原豆母体_v1.blend','原初生态研究所 • 环境资产',(0,0,0),1,0),
 ('角色 • V-001 原豆母体','原初生态研究所_V-001_原豆母体_v1.blend','05 • V-001 原豆母体 | 豆科',(0,0,0),1,0),
 ('角色 • 苔行者','characters/enemy0_苔行者/苔行者_enemy0_v1.blend','苔行者 • 角色资产',(-3.1,-1.05,.03),.84,-.12),
 ('角色 • 疾藤潜行者','characters/enemy2_疾藤潜行者/疾藤潜行者_enemy2_v1.blend','疾藤潜行者 • 角色资产',(2.8,-.55,.03),.80,.08),
]
manifest=[]
for name,file,collection,loc,scale,rotation in sources:
    path=os.path.join(BASE,file)
    with bpy.data.libraries.load(path,link=True) as (available,target):
        assert collection in available.collections,(path,collection)
        target.collections=[collection]
    c=target.collections[0]
    o=bpy.data.objects.new(name,None);assets.objects.link(o);o.instance_type='COLLECTION';o.instance_collection=c;o.location=loc;o.scale=(scale,)*3;o.rotation_euler.z=rotation;o.empty_display_size=.6
    o['source_collection']=collection;o['source_file']=file
    manifest.append({'instance':name,'source_file':file,'source_collection':collection})
def put(o,name):
    o.name=name
    for c in list(o.users_collection):c.objects.unlink(o)
    local.objects.link(o);return o
def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(.8,-11.0,5.4));cam=put(bpy.context.object,'CAM • 三角色与研究所全景');cam.data.lens=25;cam.data.clip_end=200;aim(cam,(0,1.4,3.25));s.camera=cam
def area(name,loc,power,color,size,target=(0,0,2)):
    bpy.ops.object.light_add(type='AREA',location=loc);o=put(bpy.context.object,name);o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,target)
area('主光 • 温室晨光',(3,-1,11),1800,(1,.88,.66),7)
area('补光 • 三角色正面',(-2,-7,5.5),1050,(.78,.89,1),7)
area('轮廓 • 研究所后方',(0,7,9),1700,(1,.88,.67),6)
area('生物青 • 侧光',(-5,2,5),350,(.25,.85,1),5)
bpy.ops.object.light_add(type='SUN');sun=put(bpy.context.object,'日光 • 穹顶');sun.data.energy=1.2;sun.data.angle=.16;sun.rotation_euler=(.4,-.35,-.5)
s.world.use_nodes=True;p=s.world.node_tree.nodes.get('Background');p.inputs['Color'].default_value=(.22,.31,.24,1);p.inputs['Strength'].default_value=.24
s.render.engine='CYCLES';s.cycles.samples=64;s.cycles.use_denoising=True
exec(compile(open(os.path.join(BASE,'configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s.render.resolution_x=1800;s.render.resolution_y=1200;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.render.image_settings.color_mode='RGB';s.view_settings.view_transform='AgX'
s.render.filepath=os.path.join(OUT,'原初生态研究所_三角色总场景_预览_v1.png')
s.unit_settings.system='METRIC'
s['assembly_type']='Linked collection instances: three characters and one environment'
text=bpy.data.texts.new('使用说明 • Link 总场景')
text.write('三角色 + 原初生态研究所\n\n01 集合中只有四个本地实例控制对象。选中后 G 移动、R 旋转、S 缩放。\n实际模型通过 Link 链接到三个源 .blend，修改源文件并保存后，重新打开本文件或在 Outliner 的 Blender File 视图中 Reload 对应 Library。\n不必为整体摆位建立库覆盖；需要修改内部骨骼或对象时再使用 Library Override。\n原豆站在培养台中央，苔行者居左，疾藤潜行者居右。灯光和相机统一使用总场景本地设置。\n链接使用相对路径，请连同 output/blender 的目录结构一起保留或移动；本总场景不是单文件打包资产。\n角色目前为静态首版，没有新增动画或战斗行为。\n')
for sc in bpy.data.screens:
    for a in sc.areas:
        if a.type=='VIEW_3D':a.spaces.active.region_3d.view_perspective='CAMERA';a.spaces.active.shading.type='MATERIAL'
for lib in bpy.data.libraries:
    lib.filepath='//'+os.path.relpath(bpy.path.abspath(lib.filepath),OUT).replace('\\','/')
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,'原初生态研究所_三角色_Link总场景_v1.blend'))
with open(os.path.join(OUT,'linked-assets.json'),'w',encoding='utf-8') as f:json.dump(manifest,f,ensure_ascii=False,indent=2)
print('LINKED_MASTER_SAVED',len(manifest),flush=True)
bpy.ops.render.render(write_still=True)
