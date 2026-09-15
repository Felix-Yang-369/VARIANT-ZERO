"""Independent editable enemy reconstructions. Blender --background --python this.py -- enemy0|enemy2"""
import bpy, math, random, os, sys, json
from mathutils import Vector
from math import sin,cos,pi
BASE=os.path.dirname(os.path.abspath(__file__))
PROJECT=os.path.abspath(os.path.join(BASE,'..','..','..'))
kind=sys.argv[sys.argv.index('--')+1] if '--' in sys.argv else 'enemy0'
assert kind in ('enemy0','enemy2')
NAME={'enemy0':'苔行者','enemy2':'疾藤潜行者'}[kind]
OUT=os.path.join(BASE,kind+'_'+NAME);os.makedirs(OUT,exist_ok=True)
random.seed(207 if kind=='enemy0' else 209)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
for c in list(bpy.data.collections):bpy.data.collections.remove(c)
def coll(name):
 c=bpy.data.collections.new(name);bpy.context.scene.collection.children.link(c);return c
BODY=coll('01 • '+NAME+' | 木质躯体');FACE=coll('02 • 木面与紫色生命光');LEAF=coll('03 • 破叶斗篷' if kind=='enemy0' else '03 • 疾风紫叶')
MOSS=coll('04 • 苔藓与共生植物');DETAIL=coll('05 • 枝角菌菇与饰物' if kind=='enemy0' else '05 • 卷藤与根爪');STUDIO=coll('90 • 预览地面灯光相机 (非角色)');REF=coll('99 • 参考图 (隐藏)')
C=BODY
def put(o,name,mat=None):
 o.name=name
 for col in list(o.users_collection):col.objects.unlink(o)
 C.objects.link(o)
 if mat:o.data.materials.append(mat)
 return o
def mesh(name,v,f,mat):
 me=bpy.data.meshes.new(name);me.from_pydata(v,[],f);me.update();o=bpy.data.objects.new(name,me);C.objects.link(o);me.materials.append(mat)
 for p in me.polygons:p.use_smooth=True
 return o
def mat(name,color,rough=.5,metal=0,emit=0):
 m=bpy.data.materials.new(name);m.diffuse_color=(*color,1);m.use_nodes=True;p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*color,1);p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
 if emit:p.inputs['Emission Color'].default_value=(*color,1);p.inputs['Emission Strength'].default_value=emit
 return m
def texture(m,a,b,scale=5,bump=.25,wood=False):
 ns=m.node_tree.nodes;ls=m.node_tree.links;p=ns.get('Principled BSDF')
 tc=ns.new('ShaderNodeTexCoord');mp=ns.new('ShaderNodeVectorMath');mp.operation='MULTIPLY';mp.inputs[1].default_value=(3.5,3.5,.45) if wood else (1,1,1);ls.new(tc.outputs['Generated'],mp.inputs[0])
 n=ns.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=scale;n.inputs['Detail'].default_value=5;n.inputs['Roughness'].default_value=.75;ls.new(mp.outputs[0],n.inputs['Vector'])
 r=ns.new('ShaderNodeValToRGB');r.color_ramp.elements[0].position=.23;r.color_ramp.elements[0].color=(*a,1);r.color_ramp.elements[1].position=.78;r.color_ramp.elements[1].color=(*b,1);ls.new(n.outputs['Fac'],r.inputs[0]);ls.new(r.outputs[0],p.inputs['Base Color'])
 bu=ns.new('ShaderNodeBump');bu.inputs['Strength'].default_value=bump;bu.inputs['Distance'].default_value=.08 if wood else .025;ls.new(n.outputs['Fac'],bu.inputs['Height']);ls.new(bu.outputs[0],p.inputs['Normal'])
 bark=wood
 return m
WOOD=texture(mat('树皮 • 深青枯木' if kind=='enemy0' else '树皮 • 扭曲棕藤',(.10,.17,.13),.73),(.016,.037,.038) if kind=='enemy0' else (.045,.023,.012),(.16,.27,.23) if kind=='enemy0' else (.36,.21,.09),13,.65,True)
RIDGE=texture(mat('木纹 • 断面与凸起',(.12,.095,.05),.8),(.022,.020,.012),(.18,.12,.055),12,.5,True)
MASK=texture(mat('面甲 • 原木种核',(.39,.28,.14),.56),(.17,.105,.044),(.64,.48,.26),9,.45,True)
DARK=mat('眼窝 • 深紫凹陷',(.016,.006,.028),.6)
EYE=mat('眼睛 • 枯潮紫光',(.42,.005,.83),.23,emit=3)
HOT=mat('眼睛 • 内核高光',(.92,.18,1),.2,emit=5)
LEAFMAT=texture(mat('叶片 • 苔绿旧叶',(.14,.23,.06),.72),(.024,.084,.057),(.32,.40,.07),7,.5)
EDGE=mat('叶片 • 枯黄叶缘',(.38,.37,.065),.7)
PURPLE=texture(mat('叶片 • 紫罗兰疾风叶',(.17,.008,.31),.4),(.028,.001,.075),(.24,.009,.28),6,.3)
PVEIN=mat('叶脉 • 暗玫紫',(.23,.029,.16),.5)
GVEIN=mat('叶脉 • 橄榄金',(.33,.38,.075),.65)
GREEN=texture(mat('新芽 • 翠绿',(.17,.36,.04),.5),(.017,.095,.013),(.36,.49,.04),5,.2)
FUNGI=texture(mat('菌盖 • 紫晶',(.32,.017,.48),.3),(.12,.005,.19),(.5,.044,.52),7,.15)
GILL=mat('菌褶 • 淡紫',(.46,.19,.33),.68);CREAM=mat('花瓣 • 象牙白',(.85,.81,.58),.4);POLLEN=mat('花心 • 琥珀',(.8,.38,.022),.45)
MOSSMATS=[mat('苔藓 • 色阶 %02d'%i,c,.9) for i,c in enumerate([(.045,.085,.006),(.12,.22,.009),(.24,.35,.012),(.35,.43,.024),(.065,.17,.02)])]
def uv(name,loc,scale,ma,seg=40,rings=24):
 bpy.ops.mesh.primitive_uv_sphere_add(segments=seg,ring_count=rings,location=loc);o=put(bpy.context.object,name,ma);o.scale=scale
 for p in o.data.polygons:p.use_smooth=True
 return o
def line(name,pts,r,ma,closed=False):
 cu=bpy.data.curves.new(name,'CURVE');cu.dimensions='3D';cu.bevel_depth=r;cu.bevel_resolution=3;sp=cu.splines.new('POLY');sp.points.add(len(pts)-1)
 for p,v in zip(sp.points,pts):p.co=(*v,1)
 sp.use_cyclic_u=closed;o=bpy.data.objects.new(name,cu);C.objects.link(o);cu.materials.append(ma);return o
def cat(points,t):
 x=min(t,.999999)*(len(points)-1);i=int(x);u=x-i;p=[Vector(points[max(0,min(len(points)-1,j))]) for j in (i-1,i,i+1,i+2)]
 return .5*((2*p[1])+(-p[0]+p[2])*u+(2*p[0]-5*p[1]+4*p[2]-p[3])*u*u+(-p[0]+3*p[1]-3*p[2]+p[3])*u*u*u)
def frame(points,t):
 p=cat(points,t);d=(cat(points,min(1,t+.002))-cat(points,max(0,t-.002))).normalized();ax=d.cross(Vector((0,1,0))).normalized()
 if ax.length<.1:ax=d.cross(Vector((1,0,0))).normalized()
 return p,ax,d.cross(ax).normalized()
def radius(rs,t):
 x=min(.999999,t)*(len(rs)-1);i=int(x);u=x-i;return rs[i]*(1-u)+rs[i+1]*u
def branch(name,points,rs,ma=WOOD,grain=True):
 N=max(40,(len(points)-1)*18);K=20;v=[];f=[];phase=random.uniform(0,6)
 for i in range(N+1):
  t=i/N;p,a,b=frame(points,t);r=radius(rs,t)
  for j in range(K):
   th=2*pi*j/K;rr=r*(1+.105*sin(th*7+t*13+phase)+.045*sin(th*11-t*21));v.append(p+rr*(a*cos(th)+b*sin(th)))
 for i in range(N):
  for j in range(K):a=i*K+j;f.append((a,i*K+(j+1)%K,(i+1)*K+(j+1)%K,a+K))
 f.extend([tuple(range(K-1,-1,-1)),tuple(N*K+j for j in range(K))]);o=mesh(name,v,f,ma)
 if grain:
  for j in range(7):
   pts=[]
   for i in range(55):
    t=i/54;p,a,b=frame(points,t);th=j*2*pi/7+.1*sin(t*10+j);r=radius(rs,t)*1.025;pts.append(p+r*(a*cos(th)+b*sin(th)))
   line(name+' • 纵向树皮脊',pts,.006,RIDGE)
 return o
def bez(c,t):return Vector(c[0])*(1-t)**3+Vector(c[1])*3*t*(1-t)**2+Vector(c[2])*3*t*t*(1-t)+Vector(c[3])*t**3
def leaf(name,c,width,ma=LEAFMAT,torn=False,vein=True):
 N=48;K=16;v=[];f=[];holes=[(random.uniform(.35,.83),random.choice([-1,1])*random.uniform(.2,.72),random.uniform(.055,.105)) for _ in range(4)] if torn else []
 def sample(t,u):
  p=bez(c,t);d=bez(c,min(1,t+.002))-bez(c,max(0,t-.002));side=Vector((d.z,0,-d.x)).normalized()
  w=width*sin(pi*t)**.72
  if torn:w*=1-.15*abs(sin(t*63))-.06*sin(t*111)
  return p+side*(w*u)+Vector((0,w*(.26*u*u-.09*sin(pi*t)),0))
 for i in range(N+1):
  for j in range(K+1):v.append(sample(i/N,j/K*2-1))
 for i in range(N):
  for j in range(K):
   t=(i+.5)/N;u=(j+.5)/K*2-1
   if any(((t-h[0])/h[2])**2+((u-h[1])/(h[2]*2.7))**2<1 for h in holes):continue
   a=i*(K+1)+j;f.append((a,a+1,a+K+2,a+K+1))
 o=mesh(name,v,f,ma);so=o.modifiers.new('叶肉厚度','SOLIDIFY');so.thickness=.018
 sub=o.modifiers.new('柔和叶面','SUBSURF');sub.levels=1
 if vein:
  vm=PVEIN if ma==PURPLE else GVEIN
  line(name+' • 中脉',[sample(i/70,0)+Vector((0,-.025,0)) for i in range(71)],.013,vm)
  for sg in [-1,1]:
   for k in range(2,9):line(name+' • 支脉',[sample(k/12+.13*q/12,sg*q/12*.87)+Vector((0,-.022,0)) for q in range(13)],.006,vm)
 if torn:
  for sg in [-1,1]:line(name+' • 破损枯边',[sample(i/100,sg) for i in range(101)],.011,EDGE)
 return o
patches=[]
def moss_path(points,rs,count=360):
 for _ in range(count):
  t=random.random();p,a,b=frame(points,t);theta=random.uniform(0,2*pi);r=radius(rs,t)
  normal=a*cos(theta)+b*sin(theta)
  if normal.z<-.35 and random.random()<.7:continue
  patches.append((p+normal*r*random.uniform(.97,1.12),random.uniform(.025,.067)))
def moss_blob(center,scale,count):
 for _ in range(count):
  z=random.uniform(-1,1);a=random.uniform(0,2*pi);r=math.sqrt(1-z*z)
  patches.append((Vector(center)+Vector((r*cos(a)*scale[0],r*sin(a)*scale[1],z*scale[2])),random.uniform(.023,.065)))
def braid(name,points,r,turns=4):
 for phase in [0,pi]:
  pts=[]
  for i in range(121):
   t=i/120;p,a,b=frame(points,t);th=t*turns*2*pi+phase;pts.append(p+r*(a*cos(th)+b*sin(th)))
  branch(name,pts[::3],[.045]*len(pts[::3]),RIDGE,False)
def mushroom(pos,size):
 x,y,z=pos
 branch('共生紫菌 • 菌柄',[(x,y,z),(x+.03,y,z+size*.55),(x+.04,y,z+size)], [.07*size,.065*size,.09*size],RIDGE,False)
 vv=[(x+.04,y,z+size*1.18)];ff=[];N=36;R=10
 for i in range(1,R+1):
  t=i/R;r=size*.6*t
  for j in range(N):
   a=j*2*pi/N;vv.append((x+.04+r*cos(a),y+r*sin(a),z+size*(1.18-.5*t*t)))
 for j in range(N):ff.append((0,1+j,1+(j+1)%N))
 for i in range(R-1):
  for j in range(N):a=1+i*N+j;ff.append((a,1+i*N+(j+1)%N,1+(i+1)*N+(j+1)%N,a+N))
 o=mesh('共生紫菌 • 伞盖',vv,ff,FUNGI);sol=o.modifiers.new('菌盖厚度','SOLIDIFY');sol.thickness=.035
 for j in range(18):
  a=j*2*pi/18;line('共生紫菌 • 放射菌褶',[(x+.04+.10*size*cos(a),y+.10*size*sin(a),z+.91*size),(x+.04+.56*size*cos(a),y+.56*size*sin(a),z+.69*size)],.008,GILL)
 for j in range(13):
  a=random.uniform(0,2*pi);r=random.uniform(.07,.47)*size;t=r/(size*.6)
  uv('共生紫菌 • 菌盖斑点',(x+.04+r*cos(a),y+r*sin(a),z+size*(1.18-.5*t*t)+.012),(.018*size,.018*size,.009*size),CREAM,12,8)
def sprout(pos,size=.25):
 x,y,z=pos
 for sg in [-1,1]:leaf('苔间新芽',[(x,y,z),(x+sg*size*.6,y-.04,z+size*.3),(x+sg*size,y,z+size*.7),(x+sg*size*.7,y,z+size)],size*.27,GREEN,False,False)
def flower(pos,size=.12):
 x,y,z=pos
 for i in range(5):
  a=i*2*pi/5;ob=uv('苔间白花 • 花瓣',(x+cos(a)*size*.7,y-.02,z+sin(a)*size*.7),(size*.54,.035,size*.78),CREAM,20,12);ob.rotation_euler.y=pi/2-a
 uv('苔间白花 • 花心',(x,y-.065,z),(.048,.035,.048),POLLEN,16,10)
def eye_oval(pos,sz,angle=0):
 x,y,z=pos;o=uv('紫光眼窝',(x,y,z),(sz[0]*1.28,.095,sz[1]*1.19),DARK);o.rotation_euler.y=angle
 o=uv('紫光眼球',(x,y-.07,z),(sz[0],.065,sz[1]),EYE);o.rotation_euler.y=angle
 uv('紫光瞳核',(x-.023,y-.125,z+.03),(sz[0]*.33,.025,sz[1]*.5),HOT,24,16)
if kind=='enemy0':
 # Stout hunched moss husk with a walking stance and oversized hooded head.
 branch('苔行者 • 弯曲木躯',[(0,.17,1.0),(.10,.15,1.8),(-.04,.2,2.48),(-.17,.12,2.83)],[.40,.59,.57,.4])
 head=uv('苔行者 • 圆形木面',(-.18,-.21,3.12),(.66,.53,.68),WOOD,64,40)
 for sg in [-1,1]:
  if sg<0:pts=[(-.31,.1,1.2),(-.49,-.03,.83),(-.65,-.27,.37),(-.79,-.52,.18)]
  else:pts=[(.35,.18,1.18),(.58,.4,.72),(.52,.46,.34),(.65,.14,.16)]
  branch('苔行者 • 根腿',pts,[.29,.27,.23,.25]);moss_path(pts,[.29,.27,.23,.25],430)
  foot=uv('苔行者 • 木质足掌',pts[-1],(.36,.44,.19),WOOD)
  for k in range(3):
   x=pts[-1][0]+(k-1)*.18
   branch('苔行者 • 根趾',[(x,pts[-1][1]+.03,.2),(x,pts[-1][1]-.29,.16),(x,pts[-1][1]-.4,.10)],[.115,.12,.06])
  pts=[(sg*.47,.14,2.46),(sg*.86,.0,2.07),(sg*1.08,-.17,1.69),(sg*1.15,-.25,1.47)]
  branch('苔行者 • 弯垂木臂',pts,[.24,.23,.19,.22]);moss_path(pts,[.24,.23,.19,.22],400)
  braid('苔行者 • 缠腕根绳',pts[1:3],.22,1.5)
  for k in range(3):
   x=sg*1.15+(k-1)*.16
   branch('苔行者 • 弯钩木指',[(x,-.29,1.56),(x+sg*.035,-.37,1.24),(x-sg*.06,-.49,1.08),(x-sg*.11,-.53,1.18)],[.11,.10,.075,.015])
 C=FACE
 for sg in [-1,1]:eye_oval((-.18+sg*.25,-.705,3.11),(.112,.19),-sg*.12)
 C=LEAF
 for sg in [-1,1]:
  leaf('苔行者 • 破叶兜帽', [(-.14+sg*.05,-.06,3.78),(sg*.73,-.31,3.78),(sg*.88,-.73,2.65),(sg*.82,-.70,2.15)],.42,LEAFMAT,True)
  leaf('苔行者 • 胸肩残叶',[(sg*.24,-.24,2.69),(sg*.73,-.54,2.57),(sg*.65,-.59,2.03),(sg*.98,-.37,1.83)],.28,LEAFMAT,True)
 leaf('苔行者 • 额前垂叶',[(-.44,-.2,3.72),(-.59,-.55,3.62),(-.48,-.71,3.48),(-.64,-.75,3.29)],.3,LEAFMAT,True)
 for k in range(7):
  x=(k-3)*.19
  leaf('苔行者 • 背面破叶披风',[(x,.30,2.9),(x*2,.96,2.65),(x*2.3,1.0,1.75),(x*2.4+.12*sin(k),.84,1.22+.15*sin(k))],.34,LEAFMAT,True)
 C=DETAIL
 for sg in [-1,1]:
  pts=[(-.11+sg*.23,.02,3.62),(sg*.47,.06,4.02),(sg*.56,.14,4.32),(sg*.66,.19,4.52)]
  branch('苔行者 • 鹿角状枯枝',pts,[.17,.14,.1,.035]);moss_path(pts,[.17,.14,.1,.035],180)
  branch('苔行者 • 枝角分叉',[pts[1],(sg*.8,.11,4.09),(sg*.92,.07,4.25)],[.10,.065,.018])
  leaf('枝角 • 大叶',[(sg*.56,.15,4.35),(sg*.85,.11,4.53),(sg*1.19,.07,4.34),(sg*1.31,.01,4.14)],.23,GREEN)
 # Braided necklace and hollow wooden seed pendant.
 neck=[(-.51,-.43,2.64),(-.31,-.69,2.4),(.0,-.71,2.32),(.39,-.52,2.52)]
 braid('苔行者 • 根绳项链',neck,.035,7)
 uv('苔行者 • 木种护符',(-.12,-.78,2.27),(.18,.10,.21),RIDGE)
 uv('苔行者 • 护符孔洞',(-.12,-.869,2.28),(.070,.021,.084),DARK)
 for pos,size in [((-.62,.04,3.6),.29),((.57,.15,3.1),.28),((-.85,-.02,1.74),.18),((.61,.25,.77),.22),((.26,.52,3.61),.19),((.83,-.12,1.83),.16)]:mushroom(pos,size)
 flower((-.31,-.43,3.78),.13);flower((.45,-.57,2.61),.10)
 moss_blob((-.14,.01,3.68),(.56,.44,.17),1100)
 moss_blob((.02,.25,2.15),(.60,.52,.60),800)
 for pos in [(-.56,-.36,3.61),(.61,-.37,2.68),(.81,-.28,1.72),(-.56,-.35,.38),(.43,.31,.68),(.3,.56,2.71)]:sprout(pos,.20)
else:
 # Lean asymmetrical running silhouette, three-finger root claws and swept plume.
 torso=[(.35,.12,1.8),(.16,.08,2.26),(-.03,-.04,2.63),(-.28,-.11,2.97)]
 branch('疾藤潜行者 • 前倾躯干',torso,[.27,.34,.38,.29]);moss_path(torso,[.27,.34,.38,.29],1100)
 # Long forward planted leg and trailing bent leg.
 legs=[[(.16,.06,1.95),(-.52,-.11,1.34),(-.77,-.18,.60),(-1.05,-.42,.19)],[(.51,.22,1.86),(1.08,.37,1.23),(1.55,.65,1.51),(1.95,.70,1.03)]]
 for pts in legs:
  branch('疾藤潜行者 • 奔跑根腿',pts,[.23,.20,.14,.18]);moss_path(pts,[.23,.20,.14,.18],550);braid('疾藤潜行者 • 腿部盘藤',pts,.17,2.2)
  last=Vector(pts[-1])
  for k in range(3):
   p=last+Vector(((k-1)*.17,-.06,.02));branch('疾藤潜行者 • 根爪足趾',[p,p+Vector((-.17,-.19,-.035)),p+Vector((-.28,-.35,.02)),p+Vector((-.25,-.40,.15))],[.09,.09,.056,.008])
 arms=[[(-.27,-.02,2.64),(-.74,-.06,2.30),(-1.12,-.29,2.14),(-1.48,-.51,2.4)],[(.24,.22,2.73),(.89,.4,2.62),(1.38,.3,2.47),(1.69,.09,2.24)]]
 for pts in arms:
  branch('疾藤潜行者 • 伸展藤臂',pts,[.18,.15,.135,.16]);moss_path(pts,[.18,.15,.135,.16],450);braid('疾藤潜行者 • 臂部盘藤',pts,.15,2.4)
  p=Vector(pts[-1]);sg=-1 if p.x<0 else 1
  for k in range(3):
   q=p+Vector(((k-1)*.11,-.06,(k-1)*.09));branch('疾藤潜行者 • 镰形木指',[q,q+Vector((sg*.24,-.1,-.04)),q+Vector((sg*.27,-.23,-.23)),q+Vector((sg*.12,-.29,-.27))],[.085,.083,.049,.006])
 C=FACE
 head=uv('疾藤潜行者 • 尖底木面甲',(-.41,-.21,3.24),(.58,.39,.68),MASK,64,48)
 for v in head.data.vertices:
  if v.co.z<0:v.co.x*=1+.57*v.co.z
 # Almond-shaped concave sockets and luminous inner leaf eyes.
 for sg in [-1,1]:
  x=-.41+sg*.23;z=3.25
  o=uv('疾藤潜行者 • 杏形深眼窝',(x,-.562,z),(.185,.095,.245),DARK);o.rotation_euler.y=sg*.4
  o=uv('疾藤潜行者 • 紫晶叶瞳',(x,-.638,z),(.115,.042,.185),EYE);o.rotation_euler.y=sg*.4
  o=uv('疾藤潜行者 • 眼内光核',(x-.015,-.674,z+.02),(.027,.018,.105),HOT,24,16);o.rotation_euler.y=sg*.4
 # Layered bark facial contours from brow toward beak tip.
 for sg in [-1,1]:
  for k in range(4):
   pts=[(-.41+sg*(.39-.055*k),-.47,3.63-.02*k),(-.41+sg*(.20-.025*k),-.606,3.39),(-.41+sg*.045,-.52,2.75+.018*k)]
   branch('疾藤潜行者 • 面甲纵纹',pts,[.029,.025,.009],RIDGE,False)
 C=DETAIL
 for sg in [-1,1]:
  pts=[]
  for j in range(71):
   t=j/70;a=t*2*pi*1.25;r=.31*(1-t)+.018;pts.append((-.41+sg*(.46+r*cos(a)),-.32,3.49+r*sin(a)))
  branch('疾藤潜行者 • 太阳穴卷藤',pts[::2],[.065*(1-j/35)+.012 for j in range(36)],RIDGE,False)
 moss_blob((-.38,.0,3.59),(.57,.34,.39),1400)
 C=LEAF
 for k in range(9):
  x=-.76+k*.10;y=.02+(k%3)*.13;z=3.6+(k%2)*.14
  leaf('疾藤潜行者 • 后掠紫叶冠',[(x,y,z),(x+.25,y+.20,4.35+.13*sin(k)),(x+1.20,y+.38,4.41+.12*cos(k)),(x+1.79,y+.39,3.83+.19*sin(k))],.25+(k%3)*.05,PURPLE)
 # Long curled plume that lifts up at the very end.
 leaf('疾藤潜行者 • 卷曲主冠叶',[(-.12,.36,3.78),(.83,.57,3.17),(2.50,.59,3.98),(1.88,.49,4.63)],.38,PURPLE)
 for k in range(6):
  z=2.97-k*.19;x=.28+k*.065
  leaf('疾藤潜行者 • 背部疾风叶',[(x,.27,z),(x+.53,.52,z+.12),(x+1.03,.61,z-.26),(x+1.21,.59,z-.04)],.24,PURPLE)
 for sg in [-1,1]:leaf('疾藤潜行者 • 肩部紫叶',[(sg*.20-.13,-.01,2.92),(sg*.58-.13,-.19,2.74),(sg*.7,-.36,2.47),(sg*.64,-.32,2.16)],.25,PURPLE)
 C=DETAIL
 for pos in [(-.90,-.2,.61),(-.95,-.30,2.20),(1.53,.08,2.46),(.91,.3,1.34),(-.72,-.39,3.65),(.05,-.23,3.77),(.18,-.38,2.25)]:sprout(pos,.24)
 # Additional loose spirals articulate motion around the ankle and head.
 for center,r in [((-.65,-.2,.88),.26),((1.48,.27,2.52),.24)]:
  pts=[]
  for j in range(60):
   t=j/59;a=t*2*pi*1.35;rr=r*(1-t)+.01;pts.append(Vector(center)+Vector((rr*cos(a),-.06,rr*sin(a))))
  branch('疾藤潜行者 • 游离卷须',pts[::2],[.047*(1-j/29)+.006 for j in range(30)],RIDGE,False)
# Fine geometric moss: shared material groups, consolidated meshes (not particles or image cards).
C=MOSS
mv=[];mf=[];mi=[]
for center,size in patches:
 for k in range(4):
  p=Vector(center)+Vector((random.uniform(-size,size),random.uniform(-size,size),random.uniform(-size*.5,size*.5)))
  w=size*random.uniform(.31,.55);h=w*random.uniform(.8,1.4);offset=len(mv);color=random.randrange(5)
  for row in range(5):
   ph=pi*row/4
   for col in range(6):
    a=2*pi*col/6;mv.append(p+Vector((w*sin(ph)*cos(a),w*sin(ph)*sin(a),h*cos(ph))))
  for row in range(4):
   for col in range(6):
    a=offset+row*6+col;b=offset+row*6+(col+1)%6;mf.append((a,b,b+6,a+6));mi.append(color)
mo=mesh(NAME+' • 细密苔藓几何',mv,mf,MOSSMATS[0])
for ma in MOSSMATS[1:]:mo.data.materials.append(ma)
for p,i in zip(mo.data.polygons,mi):p.material_index=i
# Character-only collection root permits clean reuse in other scenes.
root=coll(NAME+' • 角色资产')
for col in [BODY,FACE,LEAF,MOSS,DETAIL]:
 bpy.context.scene.collection.children.unlink(col);root.children.link(col)
root['asset_key']=kind;root['display_name']=NAME;root['source_image']=kind+'.png';root['status']='静态概念重建 / 未绑定'
C=STUDIO
ground=mat('预览 • 深灰绿地面',(.025,.038,.035),.79)
bpy.ops.mesh.primitive_plane_add(size=2000,location=(0,0,-.07));put(bpy.context.object,'预览地面 (非角色)',ground)
backverts=[]
for i in range(33):
 t=i/32*pi/2;backverts.extend([(-100,7+7*sin(t),-.07+7*(1-cos(t))),(100,7+7*sin(t),-.07+7*(1-cos(t)))])
backverts.extend([(-100,14,100),(100,14,100)])
mesh('预览 • 无缝弧形背景 (非角色)',backverts,[(i*2,i*2+1,i*2+3,i*2+2) for i in range(33)],ground)
def aim(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
def camera(name,loc,target,lens):
 bpy.ops.object.camera_add(location=loc);o=put(bpy.context.object,name);o.data.lens=lens;o.data.clip_end=10000;aim(o,target);return o
cam=camera('CAM 01 • '+NAME+' 三分之四',(5,-12,5.5),(0,.1,2.25),72 if kind=='enemy0' else 65)
rear=camera('CAM 02 • '+NAME+' 背面',(-6.7,11,5.2),(0,.1,2.25),70 if kind=='enemy0' else 63)
def area(name,loc,target,power,color,size):
 bpy.ops.object.light_add(type='AREA',location=loc);o=put(bpy.context.object,name);o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,target)
area('主光 • 温暖林间日光',(-4,-5,7),(0,0,2),900,(1,.79,.47),4)
area('补光 • 柔和青蓝',(4,-3,4),(0,0,2),270,(.35,.70,1),5)
area('轮廓光 • 枯潮紫',(2,4,6),(0,0,2.5),750,(.72,.22,1),3)
area('顶光 • 苔藓金绿',(-2,2,7),(0,0,2),600,(1,.91,.53),3)
s=bpy.context.scene;s.name=NAME+' • '+kind;s.camera=cam;s.world.use_nodes=True;s.world.node_tree.nodes.get('Background').inputs['Color'].default_value=(.1,.14,.17,1);s.world.node_tree.nodes.get('Background').inputs['Strength'].default_value=.25
s.render.engine='CYCLES';s.cycles.samples=64;s.cycles.use_denoising=True
exec(compile(open(os.path.join(BASE,'..','configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s.render.resolution_x=1200;s.render.resolution_y=1400;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.render.image_settings.color_mode='RGBA';s.view_settings.view_transform='AgX';s.unit_settings.system='METRIC'
C=REF;im=bpy.data.images.load(os.path.join(PROJECT,'public','art',kind+'.png'));im.pack();ob=bpy.data.objects.new('参考 • '+kind+'.png',None);REF.objects.link(ob);ob.empty_display_type='IMAGE';ob.data=im;ob.empty_display_size=5;ob.location=(8,0,3);ob.rotation_euler=(pi/2,0,0);REF.hide_render=True;REF.hide_viewport=True
s['character_name']=NAME;s['asset_key']=kind;s['name_source']='src/model.ts + src/lore.ts';s['reference']=kind+'.png';s['scope']='实际三维几何 / 静态姿态 / 未绑定 / 非低模游戏资产'
txt=bpy.data.texts.new('说明 • '+NAME);txt.write(NAME+' / '+kind+'\n\n角色所有部件在“'+NAME+' • 角色资产”集合。地面、灯光与相机在 90 集合，可单独隐藏。参考图已打包且隐藏。\n名称取自 src/model.ts 与 src/lore.ts。\n依据单视角图片制作的完整静态三维概念重建，背面为补充设计；尚未绑定骨骼、烘焙纹理或制作游戏低模。\n渲染请在偏好设置中启用 OptiX 并选 RTX 5060；自动脚本会在每次进程启动时明确启用 GPU。\n')
for screen in bpy.data.screens:
 for a in screen.areas:
  if a.type=='VIEW_3D':a.spaces.active.region_3d.view_perspective='CAMERA';a.spaces.active.shading.type='MATERIAL'
path=os.path.join(OUT,NAME+'_'+kind+'_v1.blend');s.render.filepath=os.path.join(OUT,NAME+'_三维预览_v1.png')
bpy.ops.wm.save_as_mainfile(filepath=path)
stats={'name':NAME,'key':kind,'file':path,'objects':len(s.objects),'mesh_objects':sum(o.type=='MESH' for o in s.objects),'vertices':sum(len(o.data.vertices) for o in s.objects if o.type=='MESH'),'reference_packed':bool(im.packed_file),'renderer':'Cycles / OptiX / RTX 5060','rigged':False}
with open(os.path.join(OUT,'asset-manifest.json'),'w',encoding='utf-8') as f:json.dump(stats,f,ensure_ascii=False,indent=2)
print('ASSET_SAVED',json.dumps(stats,ensure_ascii=False),flush=True)
bpy.ops.render.render(write_still=True)
s.camera=rear;s.render.resolution_x=900;s.render.resolution_y=1100;s.render.filepath=os.path.join(OUT,NAME+'_背面预览_v1.png');bpy.ops.render.render(write_still=True)
print('DONE',NAME,flush=True)
