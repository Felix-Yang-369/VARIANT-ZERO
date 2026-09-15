"""Reference-led, multi-view static asset. Run: blender -b -t 8 -P this.py -- enemy2 clay|detail"""
import os
helper=os.path.join(os.path.dirname(os.path.abspath(__file__)),'build_enemies.py')
exec(compile(open(helper,encoding='utf-8').read().split("\nif kind=='enemy0':")[0],helper,'exec'))
import bmesh
from mathutils import noise
CLAY='clay' in sys.argv
random.seed(412)
VIEWS=os.path.join(OUT,'v4_views');os.makedirs(VIEWS,exist_ok=True)
samples=[]
def pbs(m):return next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
def relief(o,amount=.015):
 tex=bpy.data.textures.get('v4 • bark microrelief')
 if tex is None:tex=bpy.data.textures.new('v4 • bark microrelief','CLOUDS');tex.noise_scale=.12;tex.noise_depth=2
 m=o.modifiers.new('树皮起伏','DISPLACE');m.texture=tex;m.texture_coords='GLOBAL';m.strength=amount
def curves(name,paths,r,ma):
 cu=bpy.data.curves.new(name,'CURVE');cu.dimensions='3D';cu.bevel_depth=r;cu.bevel_resolution=2
 for path in paths:
  if len(path)<2:continue
  sp=cu.splines.new('POLY');sp.points.add(len(path)-1);closed=(Vector(path[0])-Vector(path[-1])).length<.0001;sp.use_cyclic_u=closed
  for i,(p,co) in enumerate(zip(sp.points,path)):
   p.co=(*co,1);p.radius=1 if closed else .12+.88*max(0,sin(pi*i/(len(path)-1)))**.3
 o=bpy.data.objects.new(name,cu);C.objects.link(o);cu.materials.append(ma);return o
def frames(points,N):
 pp=[cat(points,i/N) for i in range(N+1)];result=[];prev=None
 for i,p in enumerate(pp):
  d=(pp[min(N,i+1)]-pp[max(0,i-1)]).normalized()
  a=d.cross(Vector((0,1,0))) if prev is None else prev-d*prev.dot(d)
  if a.length<.01:a=d.cross(Vector((1,0,0)))
  a.normalize();b=d.cross(a).normalized();result.append((p,a,b));prev=a
 return result
def tube(name,points,rs,ma=WOOD,moss=False,grooves=True):
 N=max(64,(len(points)-1)*16);K=32;fr=frames(points,N);vv=[];ff=[];phase=random.uniform(0,6)
 for i,(p,a,b) in enumerate(fr):
  t=i/N;r=radius(rs,t)
  for j in range(K):
   th=j*2*pi/K;rr=r*(1+.045*sin(th*5+t*6+phase)+.025*sin(th*9-t*17));n=a*cos(th)+b*sin(th);v=p+n*rr;vv.append(v)
   if moss and 2<i<N-2 and not CLAY and not (name.startswith('足爪') and i/N>.58):
    field=noise.noise_vector(v*4.7)[0]+.35*n.z+.13*sin(t*11+phase)
    if field>-.13 and random.random()<.80:
     for repeat in range(3):samples.append((v+n*.009,n,random.uniform(.80,1.4)))
 for i in range(N):
  for j in range(K):q=i*K+j;ff.append((q,i*K+(j+1)%K,(i+1)*K+(j+1)%K,q+K))
 ff.extend([tuple(range(K-1,-1,-1)),tuple(N*K+j for j in range(K))])
 o=mesh(name,vv,ff,ma)
 uv=o.data.uv_layers.new(name='GrowthUV')
 for polygon in o.data.polygons:
  for li in polygon.loop_indices:
   vi=o.data.loops[li].vertex_index;uv.data[li].uv=(vi%K/K,vi//K/N)
 if not CLAY:relief(o,.009)
 if grooves and not CLAY:
  paths=[]
  for j in range(9):
   path=[];start=random.randrange(0,13);end=random.randrange(48,65)
   for k in range(start,end):
    t=k/64;i=round(t*N);p,a,b=fr[i];th=j*2*pi/9+.09*sin(t*9+phase+j);rr=radius(rs,t)*1.008
    path.append(p+(a*cos(th)+b*sin(th))*rr)
   paths.append(path)
  curves(name+' • 深浅木纤维',paths,.0035,RIDGE)
 return o
def limb(name,points,rs):
 tube(name+' • 扭结主根',points,rs,WOOD,True)
 fr=frames(points,36)
 for k in range(2):
  pp=[];rr=[]
  for i,(p,a,b) in enumerate(fr):
   t=i/36;th=k*pi+sin(t*3.1+k)*1.6+t*2.2;r=radius(rs,t)
   pp.append(p+(a*cos(th)+b*sin(th))*r*.84);rr.append(r*(.20+.06*sin(pi*t))+.006)
  tube(name+' • 不规则交织藤根',pp[::2],rr[::2],WOOD,True)
def new_bark(ma,colors):
 ns=ma.node_tree.nodes;ls=ma.node_tree.links;ns.clear();out=ns.new('ShaderNodeOutputMaterial');p=ns.new('ShaderNodeBsdfPrincipled');ls.new(p.outputs[0],out.inputs[0]);p.inputs['Roughness'].default_value=.70
 tc=ns.new('ShaderNodeTexCoord');mp=ns.new('ShaderNodeVectorMath');mp.operation='MULTIPLY';mp.inputs[1].default_value=(2.4,1,1) if ma==MASK else (5,.38,1);ls.new(tc.outputs['UV'],mp.inputs[0])
 n=ns.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=13;n.inputs['Detail'].default_value=5;ls.new(mp.outputs[0],n.inputs[0])
 ramp=ns.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.20;ramp.color_ramp.elements[0].color=(*colors[0],1);ramp.color_ramp.elements[1].position=.80;ramp.color_ramp.elements[1].color=(*colors[1],1);ls.new(n.outputs['Fac'],ramp.inputs[0]);ls.new(ramp.outputs[0],p.inputs['Base Color'])
 bump=ns.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.32;bump.inputs['Distance'].default_value=.029;ls.new(n.outputs['Fac'],bump.inputs['Height']);ls.new(bump.outputs[0],p.inputs['Normal'])
new_bark(WOOD,[(.012,.006,.003),(.17,.070,.025)])
new_bark(MASK,[(.14,.073,.029),(.48,.32,.15)])
pbs(PURPLE).inputs['Roughness'].default_value=.40
# A forward-leaning chest with counter-rotated hips, two differently posed arms.
C=BODY
torso=[(.38,.34,1.56),(.14,.15,1.94),(-.28,-.11,2.30),(-.64,-.38,2.61)]
limb('躯干',torso,[.34,.38,.43,.33])
arms=[([(-.40,-.20,2.43),(-.99,-.32,2.00),(-1.43,-.63,1.65),(-1.94,-.87,1.79)],[.30,.27,.25,.25]), ([(.05,.19,2.40),(.78,.48,2.58),(1.51,.38,2.45),(2.14,.04,2.14)],[.29,.27,.25,.26])]
legs=[([(.18,.13,1.63),(-.56,-.19,1.12),(-.91,-.46,.68),(-1.17,-.68,.29)],[.36,.34,.25,.27]), ([(.56,.49,1.53),(1.12,.74,1.02),(1.61,.87,1.23),(2.05,.62,.78)],[.34,.30,.25,.24])]
for i,(p,r) in enumerate(arms):limb('藤臂 %d'%i,p,r)
for i,(p,r) in enumerate(legs):limb('根腿 %d'%i,p,r)
# Radially splayed, tapered claws. Individual paths differ in curl, length and depth.
for hand,(points,rs) in enumerate(arms):
 p=Vector(points[-1]);uv('掌部 • 根瘤',p,(.32,.27,.27),WOOD)
 for j in range(3):
  sp=(j-1)*.33;length=[.70,1.16,.91][j]
  if hand==0:
   offsets=[(sp*.45,0,-.04),(sp,-.13,.15),(sp-.10,-.24,.40*length),(sp-.04,-.44,.58*length),(sp+.13,-.61,.47*length),(sp+.20,-.63,.25*length)]
  else:
   offsets=[(sp*.4,0,.02),(sp,-.12,-.17),(sp+.08,-.26,-.38*length),(sp+.01,-.43,-.54*length),(sp-.15,-.53,-.42*length),(sp-.18,-.54,-.24*length)]
  from mathutils import Matrix
  turn=Matrix.Rotation([-.42,.04,.18][j],3,'Z');pp=[p+turn@Vector(q)+Vector((0,j*.055,0)) for q in offsets];tube('手爪 %d.%d'%(hand,j),pp,[r*[.90,1.04,.92][j] for r in [.16,.17,.145,.095,.044,.001]],WOOD,False)
for foot,(points,rs) in enumerate(legs):
 p=Vector(points[-1]);uv('足掌 • 根结',p,(.39,.34,.20),WOOD)
 for j in range(3):
  sp=(j-1)*.35;L=[.80,1.04,.90][j]
  pp=[p+Vector(q) for q in [(sp*.45,.08,.01),(sp,-.20,.11),(sp-.14,-.48*L,.22),(sp-.18,-.73*L,.15),(sp-.11,-.86*L,-.04),(sp,-.87*L,-.11)]]
  tube('足爪 %d.%d'%(foot,j),pp,[.18,.19,.16,.103,.050,.001],WOOD,True)
# Local head coordinates; tilt around X lets the face pitch forward naturally.
HC=Vector((-.84,-.53,3.01));HS=Vector((.79,.66,.86))*1.10
from mathutils import Matrix
ROT=Matrix.Rotation(math.radians(9),3,'X')@Matrix.Rotation(math.radians(-7),3,'Y')
def hp(p):return HC+ROT@(Vector(p)*1.10)
C=MOSS
mossbase=texture(mat('苔藓 • 深层绒毯',(.13,.23,.012),.95),(.023,.06,.002),(.20,.32,.006),32,.45)
head=uv('头颅 • 非球形苔壳',HC,HS,mossbase,96,64);head.rotation_euler=ROT.to_euler()
if not CLAY:relief(head,.055)
if not CLAY:
 for i in range(12500):
  z=random.uniform(-1,1);a=random.uniform(0,2*pi);rr=math.sqrt(1-z*z);n=Vector((rr*cos(a),rr*sin(a),z));p=Vector((n.x*HS.x,n.y*HS.y,n.z*HS.z))
  if n.y<-.57 and abs(n.x)<.65 and -.55<n.z<.48:continue
  samples.append((HC+ROT@p,ROT@n,random.uniform(.55,1.2)))
# A curved, jagged mask split into overlapping growth plates; two genuine apertures.
C=FACE
def my(x,z):return -.775+.44*(abs(x)/.79)**1.8+.22*max(0,(-z-.08)/.70)**1.3+.29*max(0,(z-.28)/.56)**1.5
def eye(sg,t,scale=1):
 a=sg*.63;x=.32*cos(t)*scale;z=.185*sin(t)*abs(sin(t))**.60*scale
 return Vector((sg*.335+x*cos(a)-z*sin(a),0,-.09+x*sin(a)+z*cos(a)))
def in_eye(x,z,sg,scale=1):
 dx=x-sg*.335;dz=z+.09;a=sg*.63;u=dx*cos(a)+dz*sin(a);v=-dx*sin(a)+dz*cos(a)
 return abs(u)<.32*scale and abs(v)<.185*scale*max(0,1-(u/(.32*scale))**2)**.8
def valid_face(x,z):
 t=(z+.79)/1.58
 width=.025+.65*max(0,min(1,(z+.79)/.52))**.9 if z<-.27 else .74
 top=.20+.36*max(0,1-abs(x)/.8)**.72+.045*sin(x*29)
 return abs(x)<width and z<top and not any(in_eye(x,z,sg,1.07) for sg in [-1,1])
NX=144;NZ=156;verts=[];byplate={}
for k in range(NZ+1):
 z=-.79+1.58*k/NZ
 for j in range(NX+1):x=-.80+1.60*j/NX;verts.append((x,my(x,z),z))
for k in range(NZ):
 for j in range(NX):
  x=-.80+1.60*(j+.5)/NX;z=-.79+1.58*(k+.5)/NZ
  if not valid_face(x,z):continue
  spread=.50+.50*(z+.79)/1.58
  plate=math.floor((x/spread+.045*sin(z*5)+.10*z)/.16);a=k*(NX+1)+j;byplate.setdefault(plate,[]).append((a,a+1,a+NX+2,a+NX+1))
for plate,faces in byplate.items():
 vv=[hp((x,y-.009*sin(plate*2.4)-.004*sin(z*23+plate),z)) for x,y,z in verts];o=mesh('面甲 • 交叠木片 %02d'%plate,vv,faces,MASK)
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS')
 for k in range(3):
  boundary=[v for v in bm.verts if v.is_boundary];updates={v:v.co.lerp(sum((e.other_vert(v).co for e in v.link_edges),Vector())/len(v.link_edges),.2) for v in boundary}
  for v,co in updates.items():v.co=co
 bm.to_mesh(o.data);bm.free()
 uvlay=o.data.uv_layers.new(name='GrowthUV')
 for loop in o.data.loops:
  p=ROT.inverted()@(o.data.vertices[loop.vertex_index].co-HC);uvlay.data[loop.index].uv=(p.x+.8,(p.z+.79)/1.58)
 sub=o.modifiers.new('木片曲面','SUBSURF');sub.levels=1;sol=o.modifiers.new('木片厚度','SOLIDIFY');sol.thickness=.045
# Convex dark sockets and pointed, translucent-looking purple life cores.
p=pbs(EYE);p.inputs['Base Color'].default_value=(.014,.0001,.049,1);p.inputs['Emission Color'].default_value=(.027,.0002,.16,1);p.inputs['Emission Strength'].default_value=.45;p.inputs['Roughness'].default_value=.40;p.inputs['Specular IOR Level'].default_value=.15
core=texture(mat('眼睛 • 尖叶晶核',(.46,.005,.95),.28,emit=2.4),(.16,.001,.5),(.68,.09,1),16,.1)
corep=pbs(core);corep.inputs['Emission Color'].default_value=(.35,.004,.95,1)
attr=core.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='CrystalGlow';core.node_tree.links.new(attr.outputs['Color'],corep.inputs['Base Color']);core.node_tree.links.new(attr.outputs['Color'],corep.inputs['Emission Color']);corep.inputs['Emission Strength'].default_value=3.0
for sg in [-1,1]:
 paths=[];vv=[];ff=[];N=80;R=18
 for k in range(R+1):
  r=k/R
  for j in range(N):
   p=eye(sg,2*pi*j/N,1.075);center=Vector((sg*.335,0,-.09));p=center+(p-center)*r;p.y=my(p.x,p.z)+.025-.035*(1-r*r);vv.append(hp(p))
 for k in range(R):
  for j in range(N):a=k*N+j;ff.append((a,k*N+(j+1)%N,(k+1)*N+(j+1)%N,a+N))
 mesh('眼窝 • 深紫晶腔',vv,ff,EYE)
 path=[]
 for j in range(101):p=eye(sg,j*2*pi/100,1.06);p.y=my(p.x,p.z)-.005;path.append(hp(p))
 curves('眼窝 • 内嵌边缘',[path],.012,DARK)
 vv=[];ff=[]
 for k in range(R+1):
  r=k/R
  for j in range(N):
   # Rotate the inner pointed leaf toward vertical; it has volume, not a flat disk.
   a=j*2*pi/N;x=.095*cos(a)*abs(cos(a))**.65*r;z=.238*sin(a)*r;ang=-sg*.39
   xx=sg*.335+x*cos(ang)-z*sin(ang);zz=-.09+x*sin(ang)+z*cos(ang);yy=my(xx,zz)-.015-.045*(1-r*r);vv.append(hp((xx,yy,zz)))
 for k in range(R):
  for j in range(N):a=k*N+j;ff.append((a,k*N+(j+1)%N,(k+1)*N+(j+1)%N,a+N))
 o=mesh('眼睛 • 尖叶生命核',vv,[tuple(reversed(f)) for f in ff],core)
 attr=o.data.color_attributes.new(name='CrystalGlow',type='FLOAT_COLOR',domain='POINT')
 for k in range(R+1):
  t=k/R;v=(1-t)**1.4
  for j in range(N):attr.data[k*N+j].color=(.045+.60*v,.0005+.08*v*v,.18+.80*v,1)
 if not CLAY:
  for j in range(16):
   x=sg*.335+random.uniform(-.14,.14);z=-.09+random.uniform(-.15,.15)
   uv('眼睛 • 微晶',hp((x,my(x,z)-.026,z)),(.003,.003,.004),HOT,8,6)
# Asymmetric temple roots: one climbs and curls, the other forks across the forehead.
C=DETAIL
localpaths=[([(.28,.01,-.67),(.66,-.40,-.30),(.78,-.48,.16),(.73,-.56,.56),(.45,-.63,.63),(.38,-.69,.44),(.55,-.69,.34),(.63,-.68,.43),(.55,-.67,.48)],[.10,.13,.11,.085,.070,.055,.040,.020,.001]),([(-.24,.05,-.65),(-.65,-.28,-.26),(-.72,-.38,.16),(-.56,-.41,.46),(-.38,-.32,.76),(-.48,-.26,.97),(-.68,-.35,.85)],[.09,.115,.10,.08,.065,.035,.001]),([(-.52,-.35,.35),(-.30,-.54,.55),(-.06,-.45,.66),(.19,-.30,.81),(.27,-.12,.95)],[.08,.06,.053,.037,.001])]
for i,(p,r) in enumerate(localpaths):tube('颅藤 • 连续分叉 %d'%i,[hp(v) for v in p],r,WOOD,False)
# Parallel-transport leaf frames retain a pointed tip through three-dimensional curls.
def ribbon(name,controls,width,twist=0,phase=0,ma=PURPLE):
 N=88;K=20;fr=frames(controls,N)
 def point(t,u):
  q=min(N-1,int(t*N));f=t*N-q;p,a,b=fr[q];p2,a2,b2=fr[q+1];p=p.lerp(p2,f);a=a.lerp(a2,f).normalized();b=b.lerp(b2,f).normalized();ang=phase+twist*t
  side=a*cos(ang)+b*sin(ang);normal=-a*sin(ang)+b*cos(ang);w=width*max(0,sin(pi*t))**1.05*(1.18-.42*t)
  return p+side*u*w+normal*w*(.21*u*u+.018*sin(t*30+u*5))
 vv=[];ff=[]
 for i in range(N+1):
  for j in range(K+1):vv.append(point(i/N,j/K*2-1))
 for i in range(N):
  for j in range(K):a=i*(K+1)+j;ff.append((a,a+1,a+K+2,a+K+1))
 o=mesh(name,vv,ff,ma);bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.00005);bm.to_mesh(o.data);bm.free()
 sub=o.modifiers.new('叶面细分','SUBSURF');sub.levels=2;sol=o.modifiers.new('叶肉双面','SOLIDIFY');sol.thickness=.009
 if not CLAY:
  def surface(t,u,side):
   dt=point(min(.9999,t+.001),u)-point(max(.0001,t-.001),u);du=point(t,u+.001)-point(t,u-.001)
   return point(t,u)+du.cross(dt).normalized()*.009*side
  vein=PVEIN if ma==PURPLE else GVEIN;main=[];secondary=[];fine=[]
  for side in [-1,1]:
   main.append([surface(i/100,0,side) for i in range(2,99)])
   for sg in [-1,1]:
    for k in range(2,16):
     t=k/19;secondary.append([surface(min(.98,t+.11*j/12),sg*.92*j/12,side) for j in range(13)])
     for h in [.3,.58]:fine.append([surface(min(.98,t+.06*h+.055*j/7),sg*(h+.2*j/7),side) for j in range(8)])
  curves(name+' • 中脉',main,.007,vein);curves(name+' • 支脉',secondary,.0028,vein);curves(name+' • 细脉',fine,.0010,vein)
 return o
C=LEAF
# Inner, intermediate and outer plumes have different depths and sweep angles.
plumes=[
 ([(-.50,.14,.62),(-.60,.12,1.12),(-.37,.28,1.54),(-.19,.30,1.75)],.29,.25,.0),
 ([(-.16,.16,.76),(.04,.29,1.24),(.61,.38,1.52),(1.14,.54,1.63)],.32,.38,.1),
 ([(.12,.17,.68),(.69,.49,1.12),(1.49,.85,.85),(2.10,1.00,1.02),(2.19,1.00,1.55),(1.96,1.00,1.68)],.39,-.40,.1),
 ([(.00,.43,.65),(.44,.94,1.22),(1.00,1.40,1.43),(1.52,1.57,1.56)],.32,.60,.50),
 ([(.24,.48,.48),(.92,.76,.59),(1.56,1.09,.15),(2.07,1.18,.34)],.34,-.55,.45),
 ([(.11,.66,.34),(.65,1.13,.31),(1.35,1.45,-.16),(1.83,1.63,.02)],.30,.5,1.0),
 ([(-.31,.51,.60),(-.47,1.13,1.14),(.09,1.67,1.30),(.70,1.87,1.44)],.29,.45,.55),
 ([(-.66,.06,.49),(-.78,.13,.88),(-.67,.22,1.20),(-.78,.40,1.38)],.20,-.30,.1),
 ([(.30,-.10,.51),(.41,-.49,.92),(1.03,-.39,.87),(1.48,-.13,.41)],.31,-.4,-.25)]
for i,(p,w,tw,ph) in enumerate(plumes):ribbon('叶冠 • 分层卷叶 %02d'%i,[hp(v) for v in p],w,tw,ph)
for k in range(6):
 z=2.63-k*.20;x=.12+k*.12;y=.38+k*.025;sg=(-1)**k
 ribbon('背部 • 错层紫叶 %02d'%k,[(x,y,z),(x+.25+sg*.10,y+.42,z+.08),(x+.57,y+.87,z-.26),(x+.84,y+1.12,z-.13)],.22+(k%3)*.035,.25+sg*.4,.35+sg*.55)
for sg in [-1,1]:
 for j in range(3):
  p=[(sg*.53,-.02,-.36-j*.12),(sg*.75,-.19,-.61-j*.15),(sg*.83,-.31,-.97-j*.15)]
  ribbon('颈部 • 覆盖叶',[hp(v) for v in p],.21+(.025 if sg>0 else 0),-sg*.4,.1)
# Small companion leaves break the woody silhouette around joints.
for i,(p,vec,w) in enumerate([((-.96,-.72,3.51),(-.32,.03,.40),.15),((-.10,-.61,3.06),(.34,.04,.20),.17),((.72,.42,2.54),(.58,.05,.27),.20),((1.62,.21,2.15),(.29,.04,.25),.13),((-.80,-.40,.70),(-.22,-.04,.30),.14),((1.57,.72,1.28),(.42,.02,.22),.17),((.56,.36,1.63),(.45,.17,.10),.18)]):
 p=Vector(p);v=Vector(vec);ribbon('共生新叶 %d'%i,[p,p+v*.45+Vector((0,-.08,.07)),p+v],w,.2,0,GREEN)
# Dense, small fronds follow sampled woody surfaces rather than fixed moss strips.
if not CLAY:
 C=MOSS;vv=[];ff=[];mi=[]
 for p,n,scale in samples:
  n=n.normalized();a=n.cross(Vector((0,0,1))).normalized()
  if a.length<.1:a=Vector((1,0,0))
  b=n.cross(a)
  for k in range(7):
   ang=random.uniform(0,2*pi);base=p+(a*cos(ang)+b*sin(ang))*random.uniform(0,.032);h=random.uniform(.014,.041)*scale;d=(n*.75+a*cos(ang)*.50+b*sin(ang)*.50).normalized();side=d.cross(n).normalized();w=h*.19
   mid=base+d*h*.5;tip=base+d*h;q=len(vv);vv.extend([base,mid-side*w,mid+n*w*.4,mid+side*w,tip]);ff.extend([(q,q+1,q+2),(q,q+2,q+3),(q+1,q+4,q+2),(q+2,q+4,q+3)]);mi.extend([random.randrange(5)]*4)
 o=mesh('苔藓 • 非均匀表面细叶',vv,ff,MOSSMATS[0])
 for ma in MOSSMATS[1:]:o.data.materials.append(ma)
 for poly,idx in zip(o.data.polygons,mi):poly.material_index=idx
# Asset interface and studio.
root=coll('疾藤潜行者 • 角色资产')
for col in [BODY,FACE,LEAF,MOSS,DETAIL]:bpy.context.scene.collection.children.unlink(col);root.children.link(col)
root['asset_key']='enemy2';root['revision']='v4';root.asset_mark()
C=STUDIO
floor=mat('影棚 • 中性灰',(.047,.047,.047),.85)
bpy.ops.mesh.primitive_plane_add(size=2000,location=(0,0,-.16));put(bpy.context.object,'影棚地面',floor)
bpy.ops.mesh.primitive_uv_sphere_add(segments=48,ring_count=24,radius=80);put(bpy.context.object,'连续影棚背景',floor)
def aim(o,p):o.rotation_euler=(Vector(p)-o.location).to_track_quat('-Z','Y').to_euler()
def camera(name,pos,target,lens=55):
 bpy.ops.object.camera_add(location=pos);o=put(bpy.context.object,name);o.data.lens=lens;aim(o,target);return o
hero=camera('CAM • 原图三分之四',(6,-12,5.3),(.12,.10,2.20),59)
cams=[]
for i in range(8):
 a=i*pi/4;cams.append(camera('CAM • 环绕 %03d'%(i*45),(11*sin(a),-11*cos(a),3.3),(.2,.3,2.2),51))
close=[camera('CAM • 面部近景',(1,-6,3.7),HC,75),camera('CAM • 根爪近景',(-4,-5,3.2),(-1.65,-.92,2.24),70),camera('CAM • 叶冠近景',(3,-6,6.8),(-.20,.30,3.65),60)]
def area(name,pos,power,color,size):
 bpy.ops.object.light_add(type='AREA',location=pos);o=put(bpy.context.object,name);o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,(0,0,2.2));return o
lights=[area('主光',(-4,-4,7),1250,(1,1,1),4),area('补光',(4,-4,4),500,(1,1,1),4),area('背光',(2,4,6),1400,(1,1,1),3)]
s=bpy.context.scene;s.name='疾藤潜行者 • v4';s.camera=hero;s.world.use_nodes=True
world=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');world.inputs['Color'].default_value=(.06,.06,.06,1);world.inputs['Strength'].default_value=.35
s.render.engine='CYCLES';s.cycles.samples=48;s.cycles.use_denoising=True
exec(compile(open(os.path.join(BASE,'..','configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s.render.resolution_x=1100;s.render.resolution_y=1100;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.render.image_settings.color_mode='RGBA';s.view_settings.view_transform='AgX'
C=REF
for file in [os.path.join(PROJECT,'public','art','enemy2.png'),os.path.join(OUT,'references','疾藤潜行者_侧后结构参考_ImageGen_v1.png')]:
 im=bpy.data.images.load(file);im.pack();o=bpy.data.objects.new('参考 • '+os.path.basename(file),None);REF.objects.link(o);o.empty_display_type='IMAGE';o.data=im;o.empty_display_size=5;o.location=(10,0,3)
REF.hide_render=True;REF.hide_viewport=True
s['asset_key']='enemy2';s['revision']='v4';s['reference_priority']='enemy2 original; generated reference is supplementary'
for screen in bpy.data.screens:
 for a in screen.areas:
  if a.type=='VIEW_3D':a.spaces.active.region_3d.view_perspective='CAMERA';a.spaces.active.shading.type='MATERIAL'
if CLAY:
 s.view_layers[0].material_override=mat('灰模审查',(.40,.40,.40),.8)
 bpy.ops.wm.save_as_mainfile(filepath=os.path.join(VIEWS,'v4_clay.blend'))
 for name,cam in [('clay_hero',hero),('clay_side',cams[2]),('clay_back',cams[4])]:
  s.camera=cam;s.render.filepath=os.path.join(VIEWS,name+'.png');bpy.ops.render.render(write_still=True)
else:
 bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,'疾藤潜行者_enemy2_v4.blend'))
 s.render.filepath=os.path.join(VIEWS,'detail_hero.png');bpy.ops.render.render(write_still=True)
print('V4_BUILD_COMPLETE', 'clay' if CLAY else 'detail',len(root.all_objects),flush=True)
