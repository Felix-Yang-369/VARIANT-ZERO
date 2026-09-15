"""Dedicated reference-driven reconstruction; reuse low-level geometry helpers only."""
import os
helper=os.path.join(os.path.dirname(os.path.abspath(__file__)),'build_enemies.py')
with open(helper,encoding='utf-8') as f:prefix=f.read().split("\nif kind=='enemy0':")[0]
exec(compile(prefix,helper,'exec'))
assert kind=='enemy2'
from mathutils import noise
samples=[]
def pbs(m):return next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
def volume_noise(o,strength=.02,scale=.15):
 tex=bpy.data.textures.get('v3 • organic relief')
 if tex is None:tex=bpy.data.textures.new('v3 • organic relief',type='CLOUDS');tex.noise_scale=scale;tex.noise_depth=2
 mod=o.modifiers.new('真实表面起伏','DISPLACE');mod.texture=tex;mod.texture_coords='GLOBAL';mod.strength=strength
def multi_curve(name,paths,r,ma):
 cu=bpy.data.curves.new(name,'CURVE');cu.dimensions='3D';cu.bevel_depth=r;cu.bevel_resolution=2
 for path in paths:
  sp=cu.splines.new('POLY');sp.points.add(len(path)-1)
  for p,co in zip(sp.points,path):p.co=(*co,1)
 ob=bpy.data.objects.new(name,cu);C.objects.link(ob);cu.materials.append(ma);return ob
def bundle(name,points,rs):
 o=branch(name+' • 芯根',points,[r*.68 for r in rs],WOOD,False);volume_noise(o,.025)
 for k in range(3):
  pp=[];rr=[]
  for i in range(25):
   t=i/24;p,a,b=frame(points,t);r=radius(rs,t);angle=k*2*pi/3+t*2*pi*.78+.11*sin(t*17+k)
   pp.append(p+(a*cos(angle)+b*sin(angle))*r*.40);rr.append(r*.60)
  ob=branch(name+' • 螺旋生长木束',pp,rr,WOOD,False);volume_noise(ob,.018)
 # Patchy continuous moss along the dorsal side, not even rings around every limb.
 for i in range(900):
  t=random.random();p,a,b=frame(points,t);r=radius(rs,t);angle=random.uniform(0,2*pi);n=a*cos(angle)+b*sin(angle)
  if n.z<-.25 or sin(t*17+angle*2)<-.2:continue
  samples.append((p+n*r*1.04,n))
 # Raised, continuous moss islands connect the fine surface fronds.
 for begin,end in [(.12,.43),(.53,.92)]:
  vv=[];ff=[];NN=36;KK=10
  for i in range(NN+1):
   t=begin+(end-begin)*i/NN;p,a,b=frame(points,t);rr=radius(rs,t)
   angle=math.atan2(b.z,a.z)
   for j in range(KK+1):
    th=angle+(j/KK-.5)*1.65;normal=a*cos(th)+b*sin(th);relief=.018*sin(pi*i/NN)*sin(pi*j/KK)
    pp=p+normal*(rr+relief);vv.append(pp)
    if i%2==0 and j%2==0:
     for repeat in range(5):samples.append((pp+normal*.007,normal))
  for i in range(NN):
   for j in range(KK):q=i*(KK+1)+j;ff.append((q,q+1,q+KK+2,q+KK+1))
  patch=mesh(name+' • 连续苔衣',vv,ff,MOSSMATS[1]);volume_noise(patch,.019)
 return o
# Bark gets fine directional fissures in addition to modeled growth ridges.
for ma in [WOOD,RIDGE,MASK]:
 ns=ma.node_tree.nodes;ls=ma.node_tree.links;p=pbs(ma);p.inputs['Roughness'].default_value=.65
 tex=ns.new('ShaderNodeTexNoise');tex.inputs['Scale'].default_value=110;tex.inputs['Detail'].default_value=4
 b=ns.new('ShaderNodeBump');b.inputs['Strength'].default_value=.4;b.inputs['Distance'].default_value=.012
 if p.inputs['Normal'].links:ls.new(p.inputs['Normal'].links[0].from_socket,b.inputs['Normal'])
 ls.new(tex.outputs['Fac'],b.inputs['Height']);ls.new(b.outputs[0],p.inputs['Normal'])
# Main silhouette: oversized moss hood, compact core, powerful three-root hands/feet.
C=BODY
torso=[(.30,.20,1.65),(.12,.12,2.10),(-.10,.02,2.47),(-.38,-.04,2.89)]
bundle('躯干',torso,[.27,.30,.36,.31])
arms=[([(-.34,-.02,2.67),(-.83,-.11,2.30),(-1.18,-.33,2.08),(-1.48,-.58,2.23)],[.23,.21,.18,.23]), ([(.13,.24,2.66),(.71,.44,2.65),(1.28,.32,2.48),(1.56,.10,2.14)],[.22,.20,.18,.23])]
legs=[([(.13,.12,1.8),(-.47,-.05,1.24),(-.71,-.25,.64),(-.95,-.49,.22)],[.27,.26,.19,.23]), ([(.42,.25,1.74),(.91,.46,1.16),(1.34,.67,1.42),(1.76,.68,.97)],[.25,.23,.18,.22])]
for points,rs in arms:bundle('藤臂',points,rs)
for points,rs in legs:bundle('根腿',points,rs)
for hand,(points,rs) in enumerate(arms):
 p=Vector(points[-1]);left=hand==0
 uv('手掌 • 木结',p,(.27,.23,.23),WOOD)
 for j in range(3):
  spread=(j-1)*.32;q=p+Vector((spread*.45,-.04,(j-1)*.02))
  if left:pp=[q,p+Vector((spread,-.20,.16)),p+Vector((spread-.04,-.33,.42)),p+Vector((spread+.04,-.49,.56)),p+Vector((spread+.17,-.59,.43)),p+Vector((spread+.20,-.61,.26))]
  else:pp=[q,p+Vector((spread,-.14,-.16)),p+Vector((spread+.07,-.28,-.39)),p+Vector((spread+.01,-.45,-.50)),p+Vector((spread-.12,-.57,-.40)),p+Vector((spread-.15,-.59,-.21))]
  ob=branch('手爪 • 分离弯钩',pp,[.14,.135,.105,.074,.035,.0008],WOOD,True);volume_noise(ob,.006)
for leg,(points,rs) in enumerate(legs):
 p=Vector(points[-1]);uv('足掌 • 交织木结',p,(.31,.27,.18),WOOD)
 for j in range(3):
  spread=(j-1)*.34;q=p+Vector((spread*.45,.03,.015));pp=[q,p+Vector((spread,-.18,.07)),p+Vector((spread-.11,-.43,.10)),p+Vector((spread-.10,-.62,.25)),p+Vector((spread+.01,-.70,.36)),p+Vector((spread+.08,-.72,.29))]
  ob=branch('足爪 • 厚根趾',pp,[.15,.15,.115,.071,.032,.0008],WOOD,True);volume_noise(ob,.006)
# Concealed woody skull with a thick living moss shell.
HC=Vector((-.43,-.08,3.36));HS=Vector((.76,.57,.84))
C=MOSS
mossbase=texture(mat('苔藓 • 绒毯连续体',(.15,.24,.009),.95),(.025,.075,.002),(.22,.35,.012),28,.65)
head=uv('头部 • 苔藓覆盖体',HC,HS,mossbase,96,64);volume_noise(head,.08)
for i in range(7000):
 z=random.uniform(-1,1);a=random.uniform(0,2*pi);r=math.sqrt(1-z*z);n=Vector((r*cos(a),r*sin(a),z));p=HC+Vector((n.x*HS.x,n.y*HS.y,n.z*HS.z))
 if n.y<-.55 and n.z<.35 and abs(n.x)<.60:continue
 samples.append((p,n))
# Shield mask defined as a convex wooden sheet with actual almond eye apertures.
C=FACE
def eye_coords(sg,t,u=1,scale=1):
 # Pointed leaf contour. Outer tips rise toward temples.
 x=.30*cos(t)*scale;z=.15*sin(t)*abs(sin(t))**.32*scale;ang=sg*.49
 return Vector((-.43+sg*.31+x*cos(ang)-z*sin(ang),-.674,3.29+x*sin(ang)+z*cos(ang)))
def in_eye(x,z,sg,scale=1):
 dx=x-(-.43+sg*.31);dz=z-3.29;a=sg*.49;u=dx*cos(a)+dz*sin(a);v=-dx*sin(a)+dz*cos(a)
 if abs(u)>.30*scale:return False
 bound=.15*scale*max(0,1-(u/(.30*scale))**2)**.66
 return abs(v)<bound
def mask_y(x,z):return -.76+.48*(abs(x+.43)/.73)**1.8+.025*(z-3.25)**2+.29*max(0,(3.25-z)/.63)**1.5+.30*max(0,(z-3.55)/.48)**1.5
vs=[];fs=[];NX=128;NZ=136
for iz in range(NZ+1):
 z=2.62+1.41*iz/NZ
 for ix in range(NX+1):
  x=-1.15+1.44*ix/NX;vs.append((x,mask_y(x,z),z))
for iz in range(NZ):
 for ix in range(NX):
  x=-1.15+1.44*(ix+.5)/NX;z=2.62+1.41*(iz+.5)/NZ;t=(z-2.62)/1.41
  width=.070+.72*sin(pi*t/2)**.55
  if abs(x+.43)>width or z>3.55+.47*max(0,1-abs(x+.43)/.73)**.85 or any(in_eye(x,z,sg,1.12) for sg in [-1,1]):continue
  a=iz*(NX+1)+ix;fs.append((a,a+1,a+NX+2,a+NX+1))
mask=mesh('面甲 • 镂空杏眼木壳',vs,fs,MASK)
import bmesh
bm=bmesh.new();bm.from_mesh(mask.data)
bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS')
for iteration in range(5):
 boundary=[v for v in bm.verts if v.is_boundary];updates={v:v.co.lerp(sum((e.other_vert(v).co for e in v.link_edges),Vector())/len(v.link_edges),.35) for v in boundary}
 for v,co in updates.items():v.co=co
bm.to_mesh(mask.data);bm.free()
sub=mask.modifiers.new('木壳柔和曲面','SUBSURF');sub.levels=1
sol=mask.modifiers.new('木面甲厚度','SOLIDIFY');sol.thickness=.08
bevel=mask.modifiers.new('雕刻边缘','BEVEL');bevel.width=.013;bevel.segments=3
volume_noise(mask,.011)
for sg in [-1,1]:
 # A recessed dark rim and a convex gemstone-like leaf eye inside the cutout.
 paths=[]
 for scale in [1.10]:
  path=[]
  for j in range(101):
   point=eye_coords(sg,2*pi*j/100,scale=scale);point.y=mask_y(point.x,point.z)+.003;path.append(point)
  paths.append(path)
 multi_curve('眼窝 • 刻入木壳边缘',paths,.011,DARK)
 vv=[];ff=[];R=20;N=96
 for r in range(R+1):
  t=r/R
  for j in range(N):
   boundary=eye_coords(sg,2*pi*j/N);center=Vector((-.43+sg*.31,-.71,3.29));p=center+(boundary-Vector((center.x,-.674,center.z)))*t;p.y=mask_y(p.x,p.z)+.020-.031*(1-t*t);vv.append(p)
 for r in range(R):
  for j in range(N):a=r*N+j;ff.append((a,r*N+(j+1)%N,(r+1)*N+(j+1)%N,a+N))
 ob=mesh('眼睛 • 杏仁紫晶',vv,ff,EYE)
# Layered fan-shaped grain follows the mask from forehead down into its pointed chin.
paths=[]
for sg in [-1,1]:
 for j in range(7):
  pp=[]
  for k in range(45):
   t=k/44;z=2.70+1.26*t;x=-.43+sg*(.027+(.13+j*.03)*t**.8)
   if z>3.55+.47*max(0,1-abs(x+.43)/.73)**.85 or any(in_eye(x,z,side,1.25) for side in [-1,1]):
    if len(pp)>2:paths.append(pp)
    pp=[];continue
   pp.append((x,mask_y(x,z)-.013-.004*sin(k*.7+j),z))
  if len(pp)>2:paths.append(pp)
multi_curve('面甲 • 生长木纤维',paths,.0013,RIDGE)
# Deep purple eyes with internal fractured glow, not a flat white highlight.
p=pbs(EYE);ns=EYE.node_tree.nodes;ls=EYE.node_tree.links;p.inputs['Roughness'].default_value=.30;p.inputs['Coat Weight'].default_value=.12;p.inputs['Specular IOR Level'].default_value=.12;p.inputs['Emission Strength'].default_value=1.1
n=ns.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=13;n.inputs['Detail'].default_value=5
ramp=ns.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.34;ramp.color_ramp.elements[0].color=(.008,.0001,.032,1);ramp.color_ramp.elements[1].position=.85;ramp.color_ramp.elements[1].color=(.29,.003,.80,1)
ls.new(n.outputs['Fac'],ramp.inputs[0]);ls.new(ramp.outputs[0],p.inputs['Base Color']);ls.new(ramp.outputs[0],p.inputs['Emission Color'])
# A small leaf-shaped life core sits within each deep-violet socket.
core=mat('紫晶 • 内部叶形光核',(.34,.006,.85),.28,emit=4)
ns=core.node_tree.nodes;ls=core.node_tree.links;p=pbs(core)
n=ns.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=19;n.inputs['Detail'].default_value=4
r=ns.new('ShaderNodeValToRGB');r.color_ramp.elements[0].position=.22;r.color_ramp.elements[0].color=(.10,.001,.40,1);r.color_ramp.elements[1].position=.80;r.color_ramp.elements[1].color=(.70,.10,1,1)
ls.new(n.outputs['Fac'],r.inputs[0]);ls.new(r.outputs[0],p.inputs['Base Color']);ls.new(r.outputs[0],p.inputs['Emission Color']);p.inputs['Emission Strength'].default_value=2.1
for sg in [-1,1]:
 vv=[];ff=[];N=64
 center=Vector((-.43+sg*.31,0,3.29))
 vv.append((center.x,mask_y(center.x,center.z)-.028,center.z))
 for j in range(N):
  p=eye_coords(sg,j*2*pi/N,scale=.52);p.y=mask_y(p.x,p.z)-.022;vv.append(p)
 for j in range(N):ff.append((0,j+1,(j+1)%N+1))
 mesh('紫晶 • 发光叶形内核',vv,ff,core)
 for j in range(13):
  t=random.uniform(.08,.90);p=eye_coords(sg,t*2*pi,scale=random.uniform(.56,.85));p.y=mask_y(p.x,p.z)-.021
  uv('紫晶 • 星点内含物',p,(.004,.003,.006),core,8,6)
# Individually carved growth splits follow the convex face surface.
paths=[]
for sg in [-1,1]:
 for j in range(12):
  z0=2.76+j*.087;start=.035+.026*j;pp=[]
  for k in range(35):
   t=k/34;z=z0+t*.23;x=-.43+sg*(start+t*(.07+j*.012)+.011*sin(t*6+j))
   if z>3.55+.47*max(0,1-abs(x+.43)/.73)**.85 or any(in_eye(x,z,s,1.23) for s in [-1,1]):
    if len(pp)>2:paths.append(pp)
    pp=[];continue
   pp.append((x,mask_y(x,z)-.008,z))
  if len(pp)>2:paths.append(pp)
multi_curve('面甲 • 分层生长裂缝',paths,.0032,RIDGE)
# Individually twisted leaf surfaces, with ridge, secondary and tertiary vessels.
C=LEAF
def ribbon(name,controls,width,twist=0,phase=0,ma=PURPLE):
 N=72;K=18;vv=[];ff=[]
 def point(t,u):
  p=cat(controls,t);d=(cat(controls,min(1,t+.003))-cat(controls,max(0,t-.003))).normalized();side=Vector((d.z,0,-d.x)).normalized();normal=d.cross(side).normalized();a=phase+twist*t
  side2=side*cos(a)+normal*sin(a);normal2=-side*sin(a)+normal*cos(a);w=width*sin(pi*t)**.8*(1+.08*sin(t*19))
  return p+side2*u*w+normal2*(.26*u*u*w+.009*w*sin(t*25+u*7))
 def surface(t,u,sg):
  dt=point(min(.9999,t+.001),u)-point(max(.0001,t-.001),u);du=point(t,u+.001)-point(t,u-.001)
  normal=du.cross(dt).normalized()
  return point(t,u)+normal*.010*sg
 for i in range(N+1):
  for j in range(K+1):vv.append(point(i/N,j/K*2-1))
 for i in range(N):
  for j in range(K):a=i*(K+1)+j;ff.append((a,a+1,a+K+2,a+K+1))
 o=mesh(name,vv,ff,ma)
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.0001);bm.to_mesh(o.data);bm.free()
 sub=o.modifiers.new('柔韧叶面','SUBSURF');sub.levels=2
 sol=o.modifiers.new('叶肉双面厚度','SOLIDIFY');sol.thickness=.008
 vein=PVEIN if ma==PURPLE else GVEIN
 multi_curve(name+' • 双面主叶脉',[[surface(i/100,0,side) for i in range(1,100)] for side in [-1,1]],.009,vein)
 paths=[];fine=[]
 for sg in [-1,1]:
  for k in range(2,17):
   t=k/20
   for side in [-1,1]:
    paths.append([surface(min(.99,t+.13*j/16),sg*.93*j/16,side) for j in range(17)])
    for h in [.25,.5,.7]:fine.append([surface(min(.99,t+.08*h+.065*j/7),sg*(h+.20*j/7),side) for j in range(8)])
 multi_curve(name+' • 双面侧叶脉',paths,.0032,vein);multi_curve(name+' • 双面细脉',fine,.0012,vein)
 return o
plumes=[
 ([(-.82,.08,3.92),(-.77,.06,4.45),(-.29,.12,4.91),(-.08,.20,5.10)],.30,-.25,-.2),
 ([(-.51,.11,4.04),(-.10,.0,4.49),(.58,.23,4.53),(.81,.31,4.34)],.36,.8,.2),
 ([(-.13,.27,3.98),(.54,.40,4.24),(1.45,.58,4.02),(1.99,.57,4.45),(1.81,.54,4.85)],.38,-1.1,.35),
 ([(-.54,.40,4.01),(.05,.55,4.72),(.89,.71,4.79),(1.15,.77,4.51)],.32,.75,-.5),
 ([(-.12,.62,3.88),(.59,.85,4.06),(1.28,1.01,3.75),(1.76,1.00,3.92)],.33,.8,.9),
 ([(-.21,.52,3.72),(.41,.73,3.65),(1.08,.70,3.30),(1.59,.86,3.62)],.29,-.9,-.4),
 ([(-.8,.5,3.85),(-.44,.92,4.30),(.25,1.18,4.45),(.60,1.26,4.18)],.29,.8,.2),
 ([(-.91,-.01,3.92),(-1.06,.00,4.28),(-.88,.10,4.63),(-.92,.17,4.76)],.19,-.4,.4)]
for i,(control,w,tw,ph) in enumerate(plumes):ribbon('叶冠 • 独立卷叶 %02d'%i,control,w,tw,ph)
for k in range(5):
 z=3.04-k*.23
 ribbon('脊背 • 层叠紫叶',[(.15,.36,z),(.56,.76,z+.10),(.95,1.08,z-.15),(1.30,1.14,z-.03)],.24,-.65+k*.19,.2+k*.2)
for sg in [-1,1]:
 for j in range(3):ribbon('颈部 • 紫叶围领',[(-.43+sg*.43,-.02,3.13-j*.15),(-.43+sg*.69,-.25,2.95-j*.17),(-.43+sg*.82,-.40,2.56-j*.13)],.24,-sg*.4,0)
# Longer side tendrils organically merge into the moss hood, not mirrored antlers.
C=DETAIL
for sg in [-1,1]:
 pts=[]
 for j in range(91):
  t=j/90;r=.27*(1-t)+.017;a=t*2*pi*1.2
  pts.append((-.43+sg*(.61+r*cos(a)),-.45-.10*sin(pi*t),3.62+r*sin(a)))
 combined=[Vector((sg*.07-.43,-.02,2.67)),Vector((-.43+sg*.66,-.28,3.08)),Vector((-.43+sg*.86,-.40,3.40))]+[Vector(p) for p in pts[::3]]
 ob=branch('颞侧 • 连续盘绕木根',combined,[.055,.10,.09]+[.075*(1-j/30)+.012 for j in range(31)],WOOD,True)
for pos,size in [((-.98,-.10,3.94),.22),((.2,-.24,3.56),.25),((.56,.38,2.72),.35),((1.40,.10,2.36),.25),((-.93,-.40,.41),.29),((.90,.5,1.35),.27),((-.34,-.36,2.36),.23)]:sprout(pos,size)
# Actual moss rosettes and branched stems, oriented along body surfaces.
C=MOSS;mv=[];mf=[];mi=[]
for p,normal in samples:
 normal=normal.normalized();u=normal.cross(Vector((0,0,1))).normalized()
 if u.length<.1:u=Vector((1,0,0))
 v=normal.cross(u)
 for k in range(6):
  a=random.uniform(0,2*pi);base=p+(u*cos(a)+v*sin(a))*random.uniform(0,.025);height=random.uniform(.022,.053);direction=(normal*.7+u*cos(a)*.55+v*sin(a)*.55).normalized();side=direction.cross(normal).normalized();w=height*.16
  mid=base+direction*height*.55;tip=base+direction*height;i=len(mv);mv.extend([base,mid-side*w,mid+normal*w*.5,mid+side*w,tip]);mf.extend([(i,i+1,i+2),(i,i+2,i+3),(i+1,i+4,i+2),(i+2,i+4,i+3)]);mi.extend([random.randrange(5)]*4)
ob=mesh('苔藓 • 密生立体细叶',mv,mf,MOSSMATS[0])
for ma in MOSSMATS[1:]:ob.data.materials.append(ma)
for face,index in zip(ob.data.polygons,mi):face.material_index=index
root=coll('疾藤潜行者 • 角色资产')
for col in [BODY,FACE,LEAF,MOSS,DETAIL]:bpy.context.scene.collection.children.unlink(col);root.children.link(col)
root['asset_key']='enemy2';root['revision']='v3 • reference reconstruction'
C=STUDIO;floor=mat('影棚 • 暗绿灰',(.026,.032,.030),.8)
bpy.ops.mesh.primitive_plane_add(size=2000,location=(0,0,-.10));put(bpy.context.object,'影棚地面',floor)
# Enclosing dome removes the distant world/floor horizon in every camera.
bpy.ops.mesh.primitive_uv_sphere_add(segments=64,ring_count=32,radius=80,location=(0,0,0));backdrop=put(bpy.context.object,'影棚 • 连续背景',floor)
def aim(o,p):o.rotation_euler=(Vector(p)-o.location).to_track_quat('-Z','Y').to_euler()
def camera(name,loc,target,lens):
 bpy.ops.object.camera_add(location=loc);o=put(bpy.context.object,name);o.data.lens=lens;o.data.clip_end=5000;aim(o,target);return o
cameras=[camera('CAM 01 • 三分之四',(6,-12,5.8),(0,.1,2.55),62),camera('CAM 02 • 真侧面',(-12,0,3.1),(0,.1,2.55),56),camera('CAM 03 • 背面',(-5,12,5.4),(0,.2,2.55),60),camera('CAM 04 • 面甲近景',(1,-6.5,4.1),(-.43,-.1,3.42),72)]
def area(name,loc,energy,color,size):
 bpy.ops.object.light_add(type='AREA',location=loc);o=put(bpy.context.object,name);o.data.energy=energy;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,(0,0,2.5))
area('主光',(-4,-4,7),1050,(1,.80,.48),4);area('补光',(4,-3,4),380,(.38,.65,1),5);area('轮廓',(2,4,6),950,(.6,.28,1),3);area('顶光',(-2,2,7),650,(1,.95,.58),3)
s=bpy.context.scene;s.name='疾藤潜行者 • v3 重建';s.camera=cameras[0];s.world.use_nodes=True
world=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');world.inputs['Color'].default_value=(.03,.04,.045,1);world.inputs['Strength'].default_value=.28
s.render.engine='CYCLES';s.cycles.samples=64;s.cycles.use_denoising=True
exec(compile(open(os.path.join(BASE,'..','configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s.render.resolution_x=1300;s.render.resolution_y=1500;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.render.image_settings.color_mode='RGBA';s.view_settings.view_transform='AgX'
C=REF
for file in [os.path.join(PROJECT,'public','art','enemy2.png'),os.path.join(OUT,'references','疾藤潜行者_侧后结构参考_ImageGen_v1.png')]:
 im=bpy.data.images.load(file);im.pack();o=bpy.data.objects.new('参考 • '+os.path.basename(file),None);REF.objects.link(o);o.empty_display_type='IMAGE';o.data=im;o.empty_display_size=5;o.location=(10,0,3)
REF.hide_render=True;REF.hide_viewport=True
s['asset_key']='enemy2';s['character_name']='疾藤潜行者';s['revision']='v3 — reconstruction in progress'
for screen in bpy.data.screens:
 for a in screen.areas:
  if a.type=='VIEW_3D':a.spaces.active.region_3d.view_perspective='CAMERA';a.spaces.active.shading.type='MATERIAL'
path=os.path.join(OUT,'疾藤潜行者_enemy2_v3.blend');s.render.filepath=os.path.join(OUT,'疾藤潜行者_v3_01.png');bpy.ops.wm.save_as_mainfile(filepath=path)
print('V3_GEOMETRY_SAVED',len(s.objects),sum(len(o.data.vertices) for o in s.objects if o.type=='MESH'),flush=True)
for i,cam in enumerate(cameras):
 s.camera=cam;s.render.filepath=os.path.join(OUT,'疾藤潜行者_v3_%02d.png'%(i+1));bpy.ops.render.render(write_still=True)
print('V3_VIEWS_COMPLETE',flush=True)
