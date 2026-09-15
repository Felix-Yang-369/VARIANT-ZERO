"""VARIANT ZERO / editable geometric reconstruction of the three supplied concepts.
Run: blender --background --factory-startup --python build_sanctuary.py
"""
import bpy, math, random, os, json
from mathutils import Vector
from math import sin, cos, pi
random.seed(709)
OUT = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(OUT))
os.makedirs(OUT, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for c in list(bpy.data.collections):
    if c.name != 'Collection': bpy.data.collections.remove(c)
default = bpy.data.collections.get('Collection')
default.name = '00 • Scene controls'
def collection(name):
    c = bpy.data.collections.new(name); bpy.context.scene.collection.children.link(c); return c
ARCH=collection('01 • 原初生态研究所 | 温室建筑')
PORTAL=collection('02 • Cultivation aperture | 椭圆培养装置')
GREEN=collection('03 • Living network | 藤蔓与植物')
POD=collection('04 • Glass podium | 玻璃培养台')
HERO=collection('05 • V-001 原豆母体 | 豆科')
LIGHT=collection('06 • Cameras and daylight')
REF=collection('07 • Packed concept references (hidden)')
current=ARCH
def put(o, name, mat=None):
    o.name=name
    for c in list(o.users_collection): c.objects.unlink(o)
    current.objects.link(o)
    if mat: o.data.materials.append(mat)
    return o
def material(name, color, metallic=0, rough=.35, transmission=0, sub=0, emission=0):
    m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF')
    p.inputs['Base Color'].default_value=(*color,1)
    p.inputs['Metallic'].default_value=metallic; p.inputs['Roughness'].default_value=rough
    p.inputs['Transmission Weight'].default_value=transmission
    p.inputs['Subsurface Weight'].default_value=sub
    p.inputs['IOR'].default_value=1.45
    p.inputs['Coat Weight'].default_value=.3
    if emission:
        p.inputs['Emission Color'].default_value=(*color,1); p.inputs['Emission Strength'].default_value=emission
    return m
def organic(m,c1,c2,scale,bump=.1):
    ns=m.node_tree.nodes; ls=m.node_tree.links; p=ns.get('Principled BSDF')
    tex=ns.new('ShaderNodeTexNoise'); tex.inputs['Scale'].default_value=scale; tex.inputs['Detail'].default_value=4
    ramp=ns.new('ShaderNodeValToRGB'); ramp.color_ramp.elements[0].position=.25; ramp.color_ramp.elements[1].position=.78
    ramp.color_ramp.elements[0].color=(*c1,1); ramp.color_ramp.elements[1].color=(*c2,1)
    ls.new(tex.outputs['Fac'],ramp.inputs[0]); ls.new(ramp.outputs[0],p.inputs['Base Color'])
    b=ns.new('ShaderNodeBump'); b.inputs['Strength'].default_value=bump; b.inputs['Distance'].default_value=.035
    ls.new(tex.outputs['Fac'],b.inputs['Height']); ls.new(b.outputs[0],p.inputs['Normal'])
ivory=material('Warm ivory • ceramic structural ribs',(.72,.75,.61),.35,.26)
trim=material('Champagne titanium • restrained warm trim',(.37,.43,.30),.7,.24)
dark=material('Deep teal • engraved recess',(.013,.083,.070),.45,.3)
cyan=material('Cyan bioluminescence',(.06,.72,.65),.25,.22,emission=2.5)
soft=material('Soft cyan indicators',(.12,.42,.34),.25,.3,emission=.65)
glass=material('Jade optical glass',(.55,.80,.72),.05,.14,.65)
core=material('Seed • translucent ivory living tissue',(.86,.84,.56),.07,.3,.07,.12)
organic(core,(.53,.59,.31),(.93,.91,.68),7,.12)
jade=material('Leaf • polished primordial jade',(.12,.43,.23),.14,.26,.08,.13)
organic(jade,(.025,.18,.11),(.39,.65,.22),5,.18)
vein=material('Leaf vein • pale living edge',(.67,.77,.32),.2,.3,.06,.1)
eye=material('Eyes • turquoise luminous seed',(.015,.8,.72),.3,.12,emission=1.4)
bark=material('Living roots • mossy bark',(.11,.17,.055),.05,.8)
organic(bark,(.027,.064,.022),(.23,.25,.08),4,.4)
foliage=[]
for i,col in enumerate([(.045,.16,.028),(.13,.25,.037),(.23,.32,.07),(.055,.24,.10),(.30,.39,.10)]):
    foliage.append(material('Canopy leaf %02d'%i,col,0,.6,0,.06))
floor=material('Pale green mineral floor',(.31,.41,.33),.18,.22)
organic(floor,(.15,.24,.20),(.53,.59,.43),1.9,.13)
def smooth(o):
    if o.type=='MESH':
        for p in o.data.polygons:p.use_smooth=True
def uv(name,loc,scale,mat,seg=40,rings=24):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=seg,ring_count=rings,location=loc)
    o=put(bpy.context.object,name,mat); o.scale=scale; smooth(o); return o
def cyl(name,loc,r,depth,mat,verts=128):
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=r,depth=depth,location=loc)
    o=put(bpy.context.object,name,mat)
    b=o.modifiers.new('Manufactured edge radius','BEVEL'); b.width=min(.045,depth/5); b.segments=3
    o.modifiers.new('Weighted corner normals','WEIGHTED_NORMAL'); return o
def curve(name,pts,r,mat,cyclic=False):
    cu=bpy.data.curves.new(name,'CURVE'); cu.dimensions='3D'; cu.resolution_u=12
    s=cu.splines.new('POLY'); s.points.add(len(pts)-1)
    for p,co in zip(s.points,pts):p.co=(*co,1)
    s.use_cyclic_u=cyclic; cu.bevel_depth=r; cu.bevel_resolution=3
    ob=bpy.data.objects.new(name,cu); current.objects.link(ob); cu.materials.append(mat); return ob
def ring(name,r,z,mat,width=.025,center=(0,0),n=160):
    return curve(name,[(center[0]+r*cos(t*2*pi/n),center[1]+r*sin(t*2*pi/n),z) for t in range(n)],width,mat,True)
def mesh(name,verts,faces,mat):
    me=bpy.data.meshes.new(name); me.from_pydata(verts,[],faces); me.update()
    ob=bpy.data.objects.new(name,me); current.objects.link(ob); me.materials.append(mat); smooth(ob);return ob
def bez(p,t):
    return Vector(p[0])*(1-t)**3+Vector(p[1])*3*t*(1-t)**2+Vector(p[2])*3*t*t*(1-t)+Vector(p[3])*t**3
def leaf(name,control,width,mat,veins=False):
    # A curved, cupped ribbon with a closed solid shell; independent editable mesh.
    n=36; m=10; vs=[]
    def sample(t,u):
        c=bez(control,t)
        tangent=bez(control,min(1,t+.005))-bez(control,max(0,t-.005))
        side=Vector((tangent.z,0,-tangent.x)).normalized()
        w=width*max(0,sin(pi*t))**.8
        return c+side*(u*w)+Vector((0,(u*u)*w*.36,0))
    for i in range(n+1):
        for j in range(m+1):vs.append(sample(i/n,j/m*2-1))
    fs=[]
    for i in range(n):
        for j in range(m):
            a=i*(m+1)+j;fs.append((a,a+1,a+m+2,a+m+1))
    o=mesh(name,vs,fs,mat)
    sol=o.modifiers.new('Living lamina thickness','SOLIDIFY');sol.thickness=.026
    sub=o.modifiers.new('Smooth organic surface','SUBSURF');sub.levels=1
    if veins:
        curve(name+' • central vascular strand',[sample(i/70,0)+Vector((0,-.025,0)) for i in range(71)],.014,cyan)
        for side in [-1,1]:
            curve(name+' • leaf margin',[sample(i/70,side)+Vector((0,-.005,0)) for i in range(71)],.010,vein)
            for k in range(2,9):
                t=k/11
                curve(name+' • lateral vein', [sample(t+.135*q/12,side*.94*q/12)+Vector((0,-.027,0)) for q in range(13)],.008,vein)
    return o
# Conservatory floor: concentric inlays and radial stone seams.
cyl('Continuous polished mineral foundation',(0,0,-.22),12.8,.4,floor)
for r in [3.7,3.77,6.1,6.17,9.1,9.17,11.5]:ring('Floor • concentric ecological conduit',r,.012,soft if r in [3.77,6.17,9.17] else trim,.012)
for i in range(32):
    a=2*pi*i/32
    curve('Floor • radial stone joint',[(r*cos(a),r*sin(a),.006) for r in [3.8,6.1,9.1,12.4]],.009,dark)
for r,z,d in [(11.5,.06,.12),(11.95,.18,.25),(12.4,.34,.4)]:
    ring('Peripheral terraced planter lip',r,z,ivory,d)
# Open spatial greenhouse, real curved ribs and transparent roof panels.
for i in range(20):
    a=2*pi*i/20; x,y=11.8*cos(a),11.8*sin(a)
    for off in [-.13,.13]:
        pts=[]
        for j in range(45):
            t=j/44
            r=11.8 if t<.4 else 11.8*cos((t-.4)/.6*pi/2)
            z=.5+16*t if t<.4 else 6.9+6.0*sin((t-.4)/.6*pi/2)
            pts.append((r*cos(a)+off*sin(a),r*sin(a)-off*cos(a),z))
        curve('Dome rib %02d'%i,pts,.085,ivory)
    cyl('Column foot',(x,y,.6),.32,.7,ivory,32)
    uv('Column status light',(x*.988,y*.988,2.7),(.075,.075,.15),cyan,16,12)
    # botanical lancets between columns, including upper tracery
    b=a+2*pi/20
    for h in [5.6,8.1]:
        pts=[]
        for j in range(41):
            t=j/40; ang=a+(b-a)*t
            pts.append((11.7*cos(ang),11.7*sin(ang),h+1.8*sin(pi*t)))
        curve('Biomorphic window arch',pts,.05,ivory)
for z,r in [(3.3,11.8),(6.6,11.8),(9.1,10.1),(11,7.6),(12.65,2.8)]:ring('Dome • circumferential brace',r,z,ivory,.065)
# Clear glazing with a transparent shader to keep readable daylight in Eevee.
glazing=bpy.data.materials.new('Conservatory glazing • clear');glazing.use_nodes=True
ns=glazing.node_tree.nodes;ns.clear();out=ns.new('ShaderNodeOutputMaterial');tr=ns.new('ShaderNodeBsdfTransparent');glazing.node_tree.links.new(tr.outputs[0],out.inputs[0])
for i in range(20):
    a=2*pi*i/20;b=2*pi*(i+1)/20
    vs=[]
    for j in range(17):
        t=j/16; r=11.8*cos(t*pi/2);z=6.9+6*sin(t*pi/2)
        vs.extend([(r*cos(a),r*sin(a),z),(r*cos(b),r*sin(b),z)])
    mesh('Roof • clear pane %02d'%i,vs,[(j*2,j*2+1,j*2+3,j*2+2) for j in range(16)],glazing)
# Rear nested oval / seed-shaped cultivation aperture.
current=PORTAL
for index,(rx,rz,yy,rad) in enumerate([(2.35,4.3,5.0,.18),(2.63,4.45,5.15,.105),(2.05,4.12,4.92,.065),(2.43,4.35,5.40,.07)]):
    pts=[]
    for j in range(200):
        a=2*pi*j/200
        pts.append((rx*sin(a)*(.9+.1*cos(a)),yy+.13*cos(2*a),4.63+rz*cos(a)))
    curve('Aperture • ivory oval rib %02d'%index,pts,rad,ivory,True)
for side in [-1,1]:
    for offset in [0,.20,.43]:
        pts=[(side*(2.1*sin(pi*j/80)+offset*sin(2*pi*j/80)),4.85+offset, .35+8.7*j/80) for j in range(81)]
        curve('Aperture • interwoven living rib',pts,.045,trim)
curve('Aperture • cyan inner channel',[(2.12*sin(2*pi*j/180),4.77,4.63+4.14*cos(2*pi*j/180)) for j in range(180)],.019,cyan,True)
for z in [.45,8.82]:
    uv('Aperture • jade node',(0,4.77,z),(.18,.15,.25),dark)
    uv('Aperture • luminous node',(0,4.61,z),(.082,.052,.135),eye)
cyl('Aperture • grounding plinth',(0,5,.16),2.7,.25,ivory)
# Thin engineered cultivation platform.
current=POD
PC=(0,-1.15)
cyl('Podium • ceramic support',(0,PC[1],.27),2.45,.36,ivory)
cyl('Podium • recessed dark gasket',(0,PC[1],.45),2.43,.055,dark)
cyl('Podium • thick jade glass plate',(0,PC[1],.525),2.46,.13,glass)
cyl('Podium • frosted optical surface',(0,PC[1],.596),2.35,.022,material('Podium • frosted pale jade',(.51,.72,.63),.18,.26,.2))
for r,z,ma,w in [(2.43,.61,glass,.034),(2.36,.613,cyan,.011),(2.1,.612,dark,.011),(2.08,.612,soft,.009),(1.05,.612,soft,.012),(1.16,.612,dark,.008),(2.44,.18,trim,.018)]:ring('Podium • machined annular detail',r,z,ma,w,PC)
for i in range(96):
    a=i*2*pi/96; r=1.98
    curve('Podium • calibrated tick',[(rr*cos(a),PC[1]+rr*sin(a),.617) for rr in [r,r+(.085 if i%8==0 else .025)]],.007,soft)
for i in range(12):
    a=i*2*pi/12
    curve('Podium • ceramic panel seam',[(2.455*cos(a),PC[1]+2.455*sin(a),z) for z in [.13,.22,.32,.43]],.013,dark)
    for off in [-.07,0,.07]:uv('Podium • side diagnostics',(2.456*cos(a+off),PC[1]+2.456*sin(a+off),.3),(.027,.027,.027),cyan,12,8)
    pts=[(2.457*cos(a+t),PC[1]+2.457*sin(a+t),.30) for t in [.12+j*.027 for j in range(12)]]
    curve('Podium • side light slot',pts,.017,soft)
# Primordial legume: ivory seed core, overlapping living jade laminae, tiny eyes.
current=HERO
Y=PC[1]; base=.64
body=uv('V-001 原豆母体 • ivory seed body',(0,Y,base+1.45),(.85,.64,1.26),core,64,48)
# deform body to egg silhouette, tighter crown and rounded lower seed
for v in body.data.vertices:
    v.co.x*=1-.18*v.co.z;v.co.y*=1-.10*v.co.z
for s in [-1,1]:
    uv('V-001 原豆母体 • leaf foot',(s*.43,Y-.11,base+.13),(.34,.46,.17),jade)
    curve('V-001 原豆母体 • foot midrib',[(s*.43,Y-.51,base+.12),(s*.43,Y-.3,base+.28),(s*.39,Y,base+.25)],.018,vein)
    uv('V-001 原豆母体 • root ankle',(s*.40,Y+.03,base+.30),(.15,.19,.24),jade)
    # Wrapped leaves hug the seed sides and leave a pale face-shaped window.
    leaf('V-001 原豆母体 • outer seed husk',[(s*.05,Y-.43,base+.32),(s*1.08,Y-.43,base+.6),(s*1.03,Y-.25,base+2.0),(s*.08,Y-.08,base+2.68)],.31,jade,True)
    leaf('V-001 原豆母体 • front embracing leaf',[(0,Y-.54,base+.22),(s*.16,Y-.72,base+1.25),(s*.94,Y-.65,base+1.38),(s*.64,Y-.35,base+2.05)],.16,jade,True)
    leaf('V-001 原豆母体 • rear sepal',[(s*.2,Y+.34,base+.28),(s*1.04,Y+.43,base+.8),(s*.65,Y+.47,base+2.1),(0,Y+.12,base+2.6)],.34,jade,False)
    # Tall organic ears arch out, curl down and end in fine points.
    controls=[(s*.34,Y+.04,base+2.44),(s*.74,Y+.10,base+4.16),(s*2.48,Y+.03,base+4.68),(s*1.93,Y-.15,base+2.62)]
    leaf('V-001 原豆母体 • large drooping leaf '+('L' if s<0 else 'R'),controls,.56,jade,True)
    curve('V-001 原豆母体 • ear stem',[bez(controls,j/100) for j in range(26)],.07,jade)
    x=s*.33; z=base+1.96
    uv('V-001 原豆母体 • ivory eye rim',(x,Y-.605,z),(.092,.050,.12),vein,32,20)
    uv('V-001 原豆母体 • small cyan eye',(x,Y-.648,z),(.063,.039,.082),eye,32,20)
    uv('V-001 原豆母体 • eye catchlight',(x-.018,Y-.679,z+.026),(.016,.012,.022),core,16,12)
leaf('V-001 原豆母体 • curled crown shoot',[(0,Y-.18,base+2.4),(-.44,Y-.22,base+3.08),(.44,Y-.01,base+3.05),(.02,Y+.03,base+3.47)],.24,jade,True)
# Real geometric leaves instanced along winding roots and hanging tendrils.
current=GREEN
leafmeshes=[]
for k,ma in enumerate(foliage):
    me=bpy.data.meshes.new('Canopy leaf geometry %d'%k)
    me.from_pydata([(0,0,0),(-.34,0,.34),(-.31,.055,.7),(0,.11,1),(.31,.055,.7),(.34,0,.34),(0,-.085,.5)],[],[(0,1,6),(1,2,6),(2,3,6),(3,4,6),(4,5,6),(5,0,6)])
    me.materials.append(ma);leafmeshes.append(me)
def smallleaf(pos,scale,idx=None):
    ob=bpy.data.objects.new('Canopy • individual living leaf',leafmeshes[random.randrange(5) if idx is None else idx]);GREEN.objects.link(ob)
    ob.location=pos;ob.scale=(scale,scale,scale);ob.rotation_euler=(random.uniform(-1.2,1.2),random.uniform(-1.1,1.1),random.uniform(0,2*pi)); return ob
for i in range(20):
    a=2*pi*i/20
    # preserve central opening and foreground sightline
    for k in range(3):
        pts=[]
        for j in range(65):
            t=j/64;ang=a+.025*sin(t*18+k*2);r=11.45+.14*sin(t*24+k)
            p=Vector((r*cos(ang),r*sin(ang),.45+t*9.3));pts.append(p)
            if j%2==0:
                for b in range(3):smallleaf(p+Vector((random.uniform(-.35,.35),random.uniform(-.35,.35),random.uniform(-.13,.13))),random.uniform(.22,.53))
        curve('Ivy • climbing braided root',pts,.035 if k else .075,bark)
    for k in range(4):
        ang=a+random.uniform(-.13,.13);r=random.uniform(8.5,11.5);top=random.uniform(8.3,11.5);length=random.uniform(1.8,4.3)
        pts=[Vector((r*cos(ang)+.12*sin(t*.2+k),r*sin(ang)+.12*cos(t*.26),top-length*t/35)) for t in range(36)]
        curve('Ivy • hanging tendril',pts,.015,bark)
        for j,p in enumerate(pts):
            if j%2==0:smallleaf(p,random.uniform(.15,.35))
    # dense domed crown clusters
    for j in range(95):
        ang=a+random.uniform(-.17,.17);r=random.uniform(9.3,12.1)
        smallleaf((r*cos(ang),r*sin(ang),random.uniform(7.9,10.5)),random.uniform(.35,.7))
    # terraced undergrowth and ferns
    for k in range(4):
        ang=a+random.uniform(-.1,.1);r=random.uniform(10.35,11.5)
        origin=Vector((r*cos(ang),r*sin(ang),.5))
        for q in range(9):
            b=q*2*pi/9;length=random.uniform(.5,1.2)
            pts=[origin+Vector((cos(b)*length*t/16,sin(b)*length*t/16,.65*sin(pi*.78*t/16))) for t in range(17)]
            curve('Fern • arching rachis',pts,.012,bark)
            for j in range(2,15,2):
                for sign in [-1,1]:
                    pos=pts[j]+Vector((cos(b+sign*pi/2)*.13,sin(b+sign*pi/2)*.13,0))
                    ob=smallleaf(pos,.23*(1-j/20),3);ob.rotation_euler=(.7,sign*.8,b)
# Distant vegetation silhouettes outside the glazing.
for i in range(65):
    a=random.uniform(0,pi);r=random.uniform(13,17)
    uv('Exterior • distant green crown',(r*cos(a),r*sin(a),random.uniform(3,7)),(random.uniform(1,2),1.5,random.uniform(2,3.5)),foliage[i%5],12,8)
# Camera and lighting.
current=LIGHT
def aim(o,p):o.rotation_euler=(Vector(p)-o.location).to_track_quat('-Z','Y').to_euler()
def camera(name,loc,target,lens):
    bpy.ops.object.camera_add(location=loc);o=put(bpy.context.object,name);aim(o,target);o.data.lens=lens;o.data.clip_end=200;return o
main=camera('CAM 01 • 原初生态研究所全景',(8.1,-18.8,7.3),(0,1,3.5),38)
detail=camera('CAM 02 • 原豆母体与培养台',(5.9,-12.5,5.25),(0,Y,2.55),53)
front=camera('CAM 03 • 原豆母体正面',(0,-11,3.1),(0,Y,2.7),55)
def area(name,loc,target,power,color,size):
    bpy.ops.object.light_add(type='AREA',location=loc);o=put(bpy.context.object,name);o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;aim(o,target)
area('Daylight • warm upper right',(3,-1,12),(0,0,0),2300,(1,.88,.65),8)
area('Daylight • soft front',(-2,-8,6),(0,-1,2),1500,(.75,.91,1),7)
area('Daylight • aperture',(0,7,10),(0,0,3),2100,(1,.90,.65),6)
area('Biological rim • cyan',(-4,1,4),(0,-1,2),550,(.14,.85,1),5)
bpy.ops.object.light_add(type='SUN',location=(3,1,12));sun=put(bpy.context.object,'Sun • conservatory morning');sun.data.energy=2.0;sun.data.angle=.16;sun.rotation_euler=(.4,-.35,-.5)
scene=bpy.context.scene;scene.world.use_nodes=True
bg=scene.world.node_tree.nodes.get('Background');bg.inputs['Color'].default_value=(.22,.31,.24,1);bg.inputs['Strength'].default_value=.4
scene.render.engine='CYCLES';scene.cycles.samples=32;scene.cycles.use_denoising=True
exec(compile(open(os.path.join(OUT,'configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
scene.render.resolution_x=1600;scene.render.resolution_y=1100;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.film_transparent=False
scene.view_settings.view_transform='AgX'
scene.render.image_settings.color_mode='RGB'
scene.camera=main
# Keep the render pipeline native: Blender 5.2 no longer exposes Scene.node_tree.
# Embedded source boards, kept out of all renders.
current=REF
for i,name in enumerate(['immersive-home-v3.png','home-podium-v4.png','home-legume-v3.png']):
    im=bpy.data.images.load(os.path.join(ROOT,'public','art',name));im.pack()
    o=bpy.data.objects.new('REFERENCE • '+name,None);REF.objects.link(o);o.empty_display_type='IMAGE';o.data=im;o.empty_display_size=5;o.location=(20+i*6,0,4);o.rotation_euler=(pi/2,0,0);o.hide_render=True
REF.hide_viewport=True;REF.hide_render=True
scene['Project']='VARIANT ZERO / 原初生态研究所'
scene.name='原初生态研究所 · V-001 原豆母体'
scene['Scene Name']='原初生态研究所';scene['Scene English']='PRIMORDIAL CONSERVATORY'
scene['Character ID']='V-001';scene['Character Name']='原豆母体';scene['Family']='legume / 豆科';scene['Growth Stage']='原初母体'
HERO['species_id']='V-001';HERO['species_name']='原豆母体';HERO['family']='legume';HERO['growth_stage']='原初母体'
scene['Contents']='One scene: geometric conservatory, glass cultivation podium, primordial legume. References are packed; no image billboards in the rendered scene.'
scene['Version']='v1 / concept reconstruction, not an exact scan; unrigged editable asset'
scene.unit_settings.system='METRIC'
readme=bpy.data.texts.new('READ ME • 场景说明')
readme.write('零号变种 · 原初生态研究所\n\n三张参考图重建于同一个三维场景。\n01 温室，02 椭圆装置，03 植被，04 培养台，05 原豆母体。\n相机 01 全景 / 02 角色与展台 / 03 正面。\n三张原图打包在隐藏的 07 集合。所有渲染主体均为实际几何；程序材质无外部贴图依赖。\n这是首版概念建模，角色未绑定，植物采用共享网格实例；不等于原图的逐像素复刻或已优化的游戏资产。\n生成脚本 build_sanctuary.py 与预览图位于本文件旁。\n')
for screen in bpy.data.screens:
    for ar in screen.areas:
        if ar.type=='VIEW_3D':
            ar.spaces.active.region_3d.view_perspective='CAMERA'
            ar.spaces.active.shading.type='MATERIAL'
exec(compile(open(os.path.join(OUT,'refine_scene.py'),encoding='utf-8').read(),'refine_scene.py','exec'))
scene.render.filepath=os.path.join(OUT,'原初生态研究所_全景_v1.png')
environment_asset=bpy.data.collections.new('原初生态研究所 • 环境资产')
scene.collection.children.link(environment_asset)
for asset_part in [ARCH,PORTAL,GREEN,POD]:
    environment_asset.children.link(asset_part)
    scene.collection.children.unlink(asset_part)
environment_asset['asset_type']='environment'
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,'原初生态研究所_V-001_原豆母体_v1.blend'))
stats={'objects':len(scene.objects),'mesh_objects':sum(o.type=='MESH' for o in scene.objects),'cameras':[o.name for o in scene.objects if o.type=='CAMERA'],'packed_images':[im.name for im in bpy.data.images if im.packed_file]}
with open(os.path.join(OUT,'scene-manifest.json'),'w',encoding='utf-8') as f:json.dump(stats,f,ensure_ascii=False,indent=2)
print('SCENE_SAVED',json.dumps(stats,ensure_ascii=False),flush=True)
bpy.ops.render.render(write_still=True)
scene.camera=detail;scene.render.resolution_x=1100;scene.render.resolution_y=1100
scene.render.filepath=os.path.join(OUT,'V-001_原豆母体_培养台近景_v1.png')
bpy.ops.render.render(write_still=True)
print('ALL_RENDERS_COMPLETE',flush=True)
