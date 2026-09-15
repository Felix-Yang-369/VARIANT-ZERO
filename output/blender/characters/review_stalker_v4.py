"""V4 review renders, round-trip verification, and real library-link smoke test."""
import bpy, os, json, math, random
from mathutils import Vector, noise
BASE=os.path.dirname(os.path.abspath(__file__));OUT=os.path.join(BASE,'enemy2_疾藤潜行者');VIEWS=os.path.join(OUT,'v4_views')
PATH=os.path.join(OUT,'疾藤潜行者_enemy2_v4.blend')
bpy.ops.wm.open_mainfile(filepath=PATH)
s=bpy.context.scene;root=bpy.data.collections['疾藤潜行者 • 角色资产']
exec(compile(open(os.path.join(BASE,'..','configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
# Low-relief bark flakes on exposed root surfaces, saved as editable geometry.
if '树皮 • 不规则翘起片层' not in bpy.data.objects:
 random.seed(918);vv=[];ff=[];uvcoords=[]
 for o in list(root.all_objects):
  if '扭结主根' not in o.name or o.type!='MESH':continue
  verts=o.data.vertices;K=32;N=len(verts)//K-1
  for k in range(110):
   row=random.randrange(5,N-5);j=random.randrange(K);q=row*K+j;p=verts[q].co
   if noise.noise_vector(p*4.7)[0]>-.10:continue
   center=sum((verts[row*K+h].co for h in range(K)),Vector())/K;n=(p-center).normalized();d=(verts[q+K].co-verts[q-K].co).normalized();a=d.cross(n).normalized()
   length=random.uniform(.09,.19);w=random.uniform(.018,.035);i=len(vv)
   vv.extend([p-d*length*.5+n*.003,p-d*length*.22-a*w+n*.005,p+d*length*.4-a*w*.65+n*.005,p+d*length*.5+n*.022,p+d*length*.30+a*w*.8+n*.005,p-d*length*.24+a*w+n*.005,p+n*.023])
   for h in range(6):ff.append((i+h,i+(h+1)%6,i+6))
   offset=random.random();uvcoords.extend([(offset+u*.03,v) for u,v in [(0,0),(-1,.25),(-.65,.8),(0,1),(.8,.75),(1,.24),(0,.5)]])
 me=bpy.data.meshes.new('非均匀树皮片层');me.from_pydata(vv,[],ff);me.update();ob=bpy.data.objects.new('树皮 • 不规则翘起片层',me);root.children[0].objects.link(ob)
 me.materials.append(bpy.data.materials['树皮 • 扭曲棕藤']);uv=me.uv_layers.new(name='GrowthUV')
 for loop in me.loops:uv.data[loop.index].uv=uvcoords[loop.vertex_index]
 for poly in me.polygons:poly.use_smooth=True
s.render.resolution_x=1000;s.render.resolution_y=1000;s.cycles.samples=48
hero=bpy.data.objects['CAM • 原图三分之四'];s.camera=hero
for name in ['主光','补光','背光']:bpy.data.objects[name].data.color=(1,1,1)
s.render.filepath=os.path.join(VIEWS,'neutral_hero.png');bpy.ops.render.render(write_still=True)
for i in range(8):
 s.camera=bpy.data.objects['CAM • 环绕 %03d'%(i*45)];s.render.filepath=os.path.join(VIEWS,'neutral_%03d.png'%(i*45));bpy.ops.render.render(write_still=True)
 print('REVIEW_ANGLE_DONE',i*45,flush=True)
s.render.resolution_x=1300;s.render.resolution_y=1300;s.cycles.samples=64
for key in ['面部','根爪','叶冠']:
 s.camera=bpy.data.objects['CAM • '+key+'近景'];s.render.filepath=os.path.join(VIEWS,'close_'+key+'.png');bpy.ops.render.render(write_still=True)
 print('REVIEW_CLOSE_DONE',key,flush=True)
# Reference-inspired warm light and purple rim; neutral views remain unchanged.
bpy.data.objects['主光'].data.color=(1,.79,.47);bpy.data.objects['主光'].data.energy=1650;bpy.data.objects['主光'].data.size=3
bpy.data.objects['补光'].data.color=(.56,.66,1);bpy.data.objects['补光'].data.energy=380
bpy.data.objects['背光'].data.color=(.58,.12,1);bpy.data.objects['背光'].data.energy=1500
s.camera=hero;s.render.resolution_x=1800;s.render.resolution_y=1800;s.cycles.samples=128
s.render.filepath=os.path.join(OUT,'疾藤潜行者_v4_精修预览.png')
root['revision']='v4 • multiview reconstruction';root.asset_data.description='疾藤潜行者：前倾奔跑、分片木面、厚根爪与分层后掠紫叶。静态高细节角色，按根集合链接。'
s['revision']='v4 • multiview reconstruction';s['delivery_scope']='Static posed model, no rig/LOD; existing scene links not replaced.'
bpy.data.orphans_purge(do_recursive=True)
bpy.ops.wm.save_as_mainfile(filepath=PATH)
bpy.ops.wm.open_mainfile(filepath=PATH)
s=bpy.context.scene;root=bpy.data.collections['疾藤潜行者 • 角色资产'];members=list(root.all_objects)
assert len(root.children)==5 and all(o.type not in {'CAMERA','LIGHT'} for o in members)
assert len([o for o in s.objects if o.type=='CAMERA'])==12
assert not any(o.library for o in members)
refimages=[im for im in bpy.data.images if im.source=='FILE' and im.users]
assert len(refimages)==2 and all(im.packed_file for im in refimages)
assert all(math.isfinite(c) for o in members for c in o.matrix_world.translation)
report={'reopened':True,'asset_file':PATH,'asset_collection':root.name,'groups':[c.name for c in root.children],
 'objects':len(members),'meshes':sum(o.type=='MESH' for o in members),'curves':sum(o.type=='CURVE' for o in members),
 'base_vertices':sum(len(o.data.vertices) for o in members if o.type=='MESH'),
 'base_faces':sum(len(o.data.polygons) for o in members if o.type=='MESH'),
 'packed_references':[im.name for im in refimages],'backend':bpy.context.preferences.addons['cycles'].preferences.compute_device_type,
 'device':s.cycles.device,'neutral_angles':list(range(0,360,45)),'closeups':['面部','根爪','叶冠']}
bpy.ops.render.render(write_still=True)
print('V4_HERO_COMPLETE',flush=True)
# A genuinely separate Blender scene links the collection from the saved asset file.
bpy.ops.wm.read_factory_settings(use_empty=True)
with bpy.data.libraries.load(PATH,link=True) as (src,dst):
 assert '疾藤潜行者 • 角色资产' in src.collections
 dst.collections=['疾藤潜行者 • 角色资产']
linked=dst.collections[0];assert linked.library and len(linked.all_objects)==report['objects']
instance=bpy.data.objects.new('LINK_TEST • 疾藤潜行者',None);instance.instance_type='COLLECTION';instance.instance_collection=linked;bpy.context.scene.collection.objects.link(instance)
bpy.context.view_layer.update()
deps=bpy.context.evaluated_depsgraph_get();instances=[x for x in deps.object_instances if x.is_instance]
assert len(instances)>=report['objects']
testfile=os.path.join(VIEWS,'疾藤潜行者_v4_Link验证.blend');bpy.ops.wm.save_as_mainfile(filepath=testfile)
bpy.ops.wm.open_mainfile(filepath=testfile)
instance=bpy.data.objects['LINK_TEST • 疾藤潜行者'];assert instance.instance_collection.library
report['link_test']={'saved_and_reopened':True,'file':testfile,'evaluated_instances':len(instances),'relative_library_path':instance.instance_collection.library.filepath}
with open(os.path.join(OUT,'verification-v4.json'),'w',encoding='utf-8') as f:json.dump(report,f,ensure_ascii=False,indent=2)
print('V4_REVIEW_AND_LINK_VERIFIED',json.dumps(report,ensure_ascii=False),flush=True)
