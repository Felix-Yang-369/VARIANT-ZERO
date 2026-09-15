"""Refine existing local source assets; write separate v2 files, preserving v1."""
import bpy,math,random,os,json
from mathutils import Vector,noise
from math import sin,cos,pi
BASE=os.path.dirname(os.path.abspath(__file__))
s=bpy.context.scene;SOURCE=bpy.data.filepath;OUT=os.path.dirname(SOURCE)
enemy=s.get('asset_key');random.seed(1329 if enemy=='enemy0' else 1341)
before={'objects':len(s.objects),'vertices':sum(len(o.data.vertices) for o in s.objects if o.type=='MESH')}
C=None
def principled(m):return next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
def material(name,color,rough=.5,metal=0):
 m=bpy.data.materials.new(name);m.use_nodes=True;m.diffuse_color=(*color,1);p=principled(m);p.inputs['Base Color'].default_value=(*color,1);p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal;return m
def mesh(name,v,f,ma):
 me=bpy.data.meshes.new(name);me.from_pydata(v,[],f);me.update();o=bpy.data.objects.new(name,me);C.objects.link(o);me.materials.append(ma)
 for p in me.polygons:p.use_smooth=True
 return o
def curves(name,paths,width,ma):
 cu=bpy.data.curves.new(name,'CURVE');cu.dimensions='3D';cu.bevel_depth=width;cu.bevel_resolution=2
 for path in paths:
  sp=cu.splines.new('POLY');sp.points.add(len(path)-1)
  for p,v in zip(sp.points,path):p.co=(*v,1)
 o=bpy.data.objects.new(name,cu);C.objects.link(o);cu.materials.append(ma);return o
def grain(m,strength=.38,scale=70):
 ns=m.node_tree.nodes;ls=m.node_tree.links;p=principled(m)
 old=list(p.inputs['Normal'].links)
 n=ns.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=scale;n.inputs['Detail'].default_value=3
 b=ns.new('ShaderNodeBump');b.inputs['Strength'].default_value=strength;b.inputs['Distance'].default_value=.015
 if old:ls.new(old[0].from_socket,b.inputs['Normal'])
 ls.new(n.outputs['Fac'],b.inputs['Height']);ls.new(b.outputs[0],p.inputs['Normal'])
def displacement(o,strength=.015):
 tex=bpy.data.textures.get('v2 • bark relief')
 if tex is None:tex=bpy.data.textures.new('v2 • bark relief',type='CLOUDS');tex.noise_scale=.13;tex.noise_depth=2
 mod=o.modifiers.new('v2 • 浅层树皮起伏','DISPLACE');mod.texture=tex;mod.strength=strength;mod.mid_level=.5;mod.texture_coords='GLOBAL'
def cells(m,a,b,scale=30):
 ns=m.node_tree.nodes;ls=m.node_tree.links;p=principled(m)
 vor=ns.new('ShaderNodeTexVoronoi');vor.feature='DISTANCE_TO_EDGE';vor.inputs['Scale'].default_value=scale
 ramp=ns.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.018;ramp.color_ramp.elements[0].color=(*a,1);ramp.color_ramp.elements[1].position=.12;ramp.color_ramp.elements[1].color=(*b,1)
 ls.new(vor.outputs['Distance'],ramp.inputs[0]);ls.new(ramp.outputs[0],p.inputs['Base Color'])
 grain(m,.18,180)
def micro_leaves(centers,ma,name,spread=.027):
 vs=[];fs=[]
 for p in centers:
  p=Vector(p)
  for k in range(7):
   a=random.uniform(0,2*pi);h=random.uniform(.016,.036);w=h*.17
   base=p+Vector((random.uniform(-spread,spread),random.uniform(-spread,spread),random.uniform(-.01,.01)))
   d=Vector((cos(a),sin(a),random.uniform(.3,1)));side=Vector((-sin(a),cos(a),0));mid=base+d*h*.53;tip=base+d*h
   i=len(vs);vs.extend([base,mid-side*w,mid+Vector((0,0,w*.6)),mid+side*w,tip]);fs.extend([(i,i+1,i+2),(i,i+2,i+3),(i+1,i+4,i+2),(i+2,i+4,i+3)])
 o=mesh(name,vs,fs,ma);return o
def leaf_detail(o,rows,cols,ma):
 vs=o.data.vertices
 if len(vs)!=(rows+1)*(cols+1):return
 # Fine tertiary vessels lie on the existing curved lamina, not in a flat decal.
 paths=[]
 for i in range(4,rows-4,3):
  for sign in [-1,1]:
   mid=cols//2
   for j in range(2,mid-1,2):
    pts=[]
    for k in range(5):
     r=min(rows-1,i+k);c=max(1,min(cols-1,mid+sign*(j+k//2)))
     pts.append(vs[r*(cols+1)+c].co+Vector((0,-.012,0)))
    paths.append(pts)
 curves('v2 • '+o.name+' 微细叶脉',paths,.0017,ma)
 for v in vs:
  v.co.y+=.009*sin(v.co.x*37+v.co.z*21)*sin(v.co.z*29)
if enemy:
 name=s['character_name'];root=bpy.data.collections[name+' • 角色资产']
 BODY=bpy.data.collections['01 • '+name+' | 木质躯体'];C=BODY
 wood=bpy.data.materials['树皮 • 深青枯木' if enemy=='enemy0' else '树皮 • 扭曲棕藤']
 ridge=bpy.data.materials['木纹 • 断面与凸起']
 # Layered bark plates follow each limb's original swept cross sections.
 plates=[];faces=[]
 for o in list(BODY.objects):
  if o.type!='MESH':continue
  if '足掌' in o.name or '木面' in o.name:displacement(o,.026);continue
  nv=len(o.data.vertices)
  if nv%20 or nv<800:continue
  displacement(o,.022)
  rings=nv//20
  for j in range(0,20,3):
   start=random.randrange(1,5)
   while start<rings-5:
    length=random.randrange(6,14);end=min(rings-2,start+length);offset=len(plates)
    for i in range(start,end+1):
     center=sum((o.data.vertices[i*20+k].co for k in range(20)),Vector())/20
     t=(i-start)/max(1,end-start)
     for q in range(3):
      v=o.data.vertices[i*20+(j+q)%20].co;normal=(v-center).normalized();lift=.002+.006*sin(pi*t)*(.5+random.random()*.5)
      plates.append(v+normal*lift)
    for i in range(end-start):
     a=offset+i*3;faces.extend([(a,a+1,a+4,a+3),(a+1,a+2,a+5,a+4)])
    start=end+random.randrange(1,4)
 o=mesh('v2 • 不规则叠层树皮片',plates,faces,wood);sol=o.modifiers.new('树皮剥片厚度','SOLIDIFY');sol.thickness=.002
 for m in [wood,ridge,bpy.data.materials.get('面甲 • 原木种核')]:
  if m:grain(m,.46,100)
 # Replace grape-like placeholder moss with fine branching rosette geometry.
 C=bpy.data.collections['04 • 苔藓与共生植物'];old=C.objects.get(name+' • 细密苔藓几何')
 centers=[]
 if old:
  for i in range(0,len(old.data.vertices)-29,90):centers.append(sum((old.data.vertices[i+k].co for k in range(30)),Vector())/30)
  for i in range(0,len(old.data.vertices)-29,30):
   center=sum((old.data.vertices[i+k].co for k in range(30)),Vector())/30
   for k in range(30):
    delta=old.data.vertices[i+k].co-center;old.data.vertices[i+k].co=center+Vector((delta.x*1.15,delta.y*1.15,delta.z*.30))
  old.name='v2 • 绒毯状苔藓基底'
 moss=bpy.data.materials['苔藓 • 色阶 02'].copy();moss.name='v2 • 苔藓绒叶';cells(moss,(.07,.13,.005),(.22,.33,.012),8)
 if old:
  old.data.materials.clear();old.data.materials.append(moss)
  for face in old.data.polygons:face.material_index=0
 micro_leaves(centers,moss,'v2 • 细叶分枝苔藓')
 # Dark sockets and internally mottled crystalline eyes replace flat white highlights.
 for o in s.objects:
  if any(t in o.name for t in ['紫光瞳核','眼内光核']):o.hide_render=True;o.hide_viewport=True
  if o.type=='MESH' and any(t in o.name for t in ['紫光眼窝','杏形深眼窝']):o.scale.y*=.45
  if o.type=='CURVE' and '纵向树皮脊' in o.name:o.data.bevel_depth*=.48
 eye=bpy.data.materials['眼睛 • 枯潮紫光'];ns=eye.node_tree.nodes;ls=eye.node_tree.links;p=ns.get('Principled BSDF')
 p.inputs['Roughness'].default_value=.17;p.inputs['Coat Weight'].default_value=.75;p.inputs['Emission Strength'].default_value=3.0
 layer=ns.new('ShaderNodeLayerWeight');layer.inputs['Blend'].default_value=.5
 ramp=ns.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.12;ramp.color_ramp.elements[0].color=(.55,.025,.96,1);ramp.color_ramp.elements[1].position=.9;ramp.color_ramp.elements[1].color=(.019,0,.055,1)
 ls.new(layer.outputs['Facing'],ramp.inputs[0]);ls.new(ramp.outputs[0],p.inputs['Base Color']);ls.new(ramp.outputs[0],p.inputs['Emission Color'])
 grain(eye,.11,90)
 C=bpy.data.collections['03 • 破叶斗篷' if enemy=='enemy0' else '03 • 疾风紫叶']
 for o in list(C.objects):
  if o.type=='MESH':leaf_detail(o,48,16,bpy.data.materials['叶脉 • 暗玫紫' if enemy=='enemy2' else '叶脉 • 橄榄金'])
  elif o.type=='CURVE':o.data.bevel_depth*=.6
 for m in bpy.data.materials:
  if m.name.startswith('叶片'):grain(m,.22,160)
 if enemy=='enemy0':
  for o in s.objects:
   if '苔间白花' in o.name and o.location.z>3.6:o.location.z+=.18;o.location.y-=.10
  # A true volumetric leaf hood around the face with an open face aperture.
  vs=[];fs=[];N=100;K=30
  for i in range(N+1):
   a=i/N*2*pi
   for j in range(K+1):
    t=j/K;theta=.84+.055*sin(a*13)+t*1.65
    vs.append((-.18+.81*sin(theta)*cos(a),-.21-.67*cos(theta),3.12+.83*sin(theta)*sin(a)))
  for i in range(N):
   for j in range(K):
    if j<3 and i%19 in [0,1]:continue
    a=i*(K+1)+j;fs.append((a,a+K+1,a+K+2,a+1))
  ma=bpy.data.materials['叶片 • 苔绿旧叶'];hood=mesh('v2 • 立体破叶兜帽',vs,fs,ma);sol=hood.modifiers.new('兜帽叶肉','SOLIDIFY');sol.thickness=.023
  rim=[Vector(vs[i*(K+1)]) for i in range(N+1)];curves('v2 • 兜帽枯黄纤维边',[rim],.014,bpy.data.materials['叶片 • 枯黄叶缘'])
  C=bpy.data.collections['04 • 苔藓与共生植物'];tops=[Vector(vs[i*(K+1)+j])+Vector((0,0,.015)) for i in range(3,48,2) for j in range(3,K,2)]
  micro_leaves(tops,moss,'v2 • 兜帽附生苔藓',.035)
 else:
  C=bpy.data.collections['05 • 卷藤与根爪']
  for sg in [-1,1]:
   pts=[]
   for j in range(100):
    t=j/99;r=.20*(1-t)+.007;a=t*2*pi*1.38;pts.append((-.41+sg*(.46+r*cos(a)),-.59-.03*sin(a),3.54+r*sin(a)))
   curves('v2 • 额侧卷曲木须',[pts],.032,ridge)
  for o in bpy.data.collections['02 • 木面与紫色生命光'].objects:
   if o.type=='MESH' and '木面甲' in o.name:displacement(o,.018)
 s['asset_revision']='v2 • 叠层树皮 / 细叶苔藓 / 微叶脉 / 晶质眼睛'
 preview=name+'_三维精修预览_v2.png'
else:
 # Primordial seed: layered translucent tissue and nonuniform micro-venation.
 HERO=bpy.data.collections['05 • V-001 原豆母体 | 豆科'];C=HERO
 jade=bpy.data.materials['Leaf • polished primordial jade']
 cells(jade,(.10,.29,.135),(.065,.23,.11),60)
 p=jade.node_tree.nodes.get('Principled BSDF');p.inputs['Subsurface Weight'].default_value=.08;p.inputs['Coat Weight'].default_value=.45;p.inputs['Roughness'].default_value=.24
 core=bpy.data.materials['Seed • translucent ivory living tissue'];grain(core,.10,210)
 ma=material('v2 • 玉叶毛细脉',(.27,.53,.22),.4)
 for o in list(HERO.objects):
  if o.type=='MESH':
   leaf_detail(o,36,10,ma)
   if 'ivory seed body' in o.name:displacement(o,.007)
  if o.type=='CURVE' and 'lateral vein' in o.name:o.data.bevel_depth*=.55
 # Refine shared vegetation leaves with fine ribs, preserving instancing.
 for m in bpy.data.materials:
  if m.name.startswith('Canopy leaf'):grain(m,.26,65)
 ARCH=bpy.data.collections['01 • 原初生态研究所 | 温室建筑'];C=ARCH
 ivory=bpy.data.materials['Warm ivory • ceramic structural ribs'];dark=bpy.data.materials['Deep teal • engraved recess'];trim=bpy.data.materials['Champagne titanium • restrained warm trim']
 grain(ivory,.075,130)
 # Segmented manufactured column collars and radial base panels.
 for i in range(20):
  a=i*2*pi/20
  for z in [.78,3.30,6.65]:
   pts=[(11.8*cos(a)+.24*cos(t*2*pi/36),11.8*sin(a)+.24*sin(t*2*pi/36),z) for t in range(37)]
   curves('v2 • 分段柱箍',[pts],.022,trim)
  tangent=Vector((-sin(a),cos(a),0));center=Vector((11.75*cos(a),11.75*sin(a),1.15))
  curves('v2 • 立柱检修面板',[[center+tangent*.14+Vector((0,0,-.23)),center+tangent*.14+Vector((0,0,.23)),center-tangent*.14+Vector((0,0,.23)),center-tangent*.14+Vector((0,0,-.23))]],.012,dark)
 # More natural understory fronds along the perimeter, each a modeled leaf array.
 C=bpy.data.collections['03 • Living network | 藤蔓与植物'];green=material('v2 • 温室蕨叶',(.032,.13,.035),.64);grain(green,.18,70)
 verts=[];faces=[]
 for i in range(64):
  a=random.uniform(0,2*pi);r=random.uniform(10.7,11.5);origin=Vector((r*cos(a),r*sin(a),.42))
  for f in range(7):
   angle=a+f*2*pi/7;d=Vector((cos(angle),sin(angle),0));side=Vector((-sin(angle),cos(angle),0));L=random.uniform(.5,1.2)
   path=[origin+d*(L*t/24)+Vector((0,0,.9*sin(pi*.78*t/24))) for t in range(25)];curves('v2 • 林下蕨类叶轴',[path],.009,green)
   for j in range(3,23):
    w=.19*sin(pi*j/25)
    for sign in [-1,1]:
     p=path[j];end=p+side*w*sign+d*.08;v=len(verts);verts.extend([p,p+d*.035+side*w*.45*sign,end,p-d*.02+side*w*.45*sign]);faces.append((v,v+1,v+2,v+3))
 mesh('v2 • 温室蕨类羽状复叶',verts,faces,green)
 # Visible dielectric glazing with transparent shadow rays.
 glazing=bpy.data.materials['Conservatory glazing • clear'];ns=glazing.node_tree.nodes;ls=glazing.node_tree.links;ns.clear()
 out=ns.new('ShaderNodeOutputMaterial');bs=ns.new('ShaderNodeBsdfGlass');bs.inputs['Color'].default_value=(.88,.97,.95,1);bs.inputs['Roughness'].default_value=.075;bs.inputs['IOR'].default_value=1.12
 tr=ns.new('ShaderNodeBsdfTransparent');lp=ns.new('ShaderNodeLightPath');mix=ns.new('ShaderNodeMixShader');ls.new(lp.outputs['Is Shadow Ray'],mix.inputs[0]);ls.new(bs.outputs[0],mix.inputs[1]);ls.new(tr.outputs[0],mix.inputs[2]);ls.new(mix.outputs[0],out.inputs[0])
 floor=bpy.data.materials['Pale green mineral floor'];grain(floor,.08,220)
 s['asset_revision']='v2 • 玉质组织 / 毛细叶脉 / 构件分段 / 林下蕨类 / 屋顶玻璃'
 preview='原初生态研究所_原豆精修预览_v2.png'
 # Save the source scene with its overview camera, render hero detail separately.
def stats():return {'objects':len(s.objects),'vertices':sum(len(o.data.vertices) for o in s.objects if o.type=='MESH')}
s.cycles.samples=96
exec(compile(open(os.path.join(BASE,'configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
target=SOURCE.replace('_v1.blend','_v2.blend');assert target!=SOURCE
s['refinement_source']=os.path.basename(SOURCE)
s.render.filepath=os.path.join(OUT,preview)
note=bpy.data.texts.new('v2 • 精修说明')
note.write(s['asset_revision']+'\n\nv1 原文件保留。本文件为 v2 精修资产，集合名称保持兼容，可由 Link 总场景直接引用。\n增加了几何与材质细节；仍为风格化静态模型，没有新增骨骼、动画、UV 烘焙或游戏低模。\n')
bpy.ops.wm.save_as_mainfile(filepath=target)
report={'source':SOURCE,'output':target,'before':before,'after':stats(),'revision':s['asset_revision']}
with open(os.path.join(OUT,'refinement-v2.json'),'w',encoding='utf-8') as f:json.dump(report,f,ensure_ascii=False,indent=2)
print('V2_SAVED',json.dumps(report,ensure_ascii=False),flush=True)
if not enemy:s.camera=bpy.data.objects['CAM 02 • 原豆母体与培养台']
s.render.resolution_x=1300;s.render.resolution_y=1500;s.render.filepath=os.path.join(OUT,preview)
bpy.ops.render.render(write_still=True)
print('V2_PREVIEW_COMPLETE',flush=True)
