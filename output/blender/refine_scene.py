"""Shared visual refinements, called by build_sanctuary.py before saving."""
import bpy, math, random
from mathutils import Vector, Euler
random.seed(810)
s=bpy.context.scene
def point_camera(name,location,target,lens):
    o=bpy.data.objects[name];o.location=location;o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler();o.data.lens=lens
point_camera('CAM 01 • 原初生态研究所全景',(2.7,-10.3,5.2),(0,2.0,3.45),23)
point_camera('CAM 02 • 原豆母体与培养台',(2.5,-9.5,4.3),(0,-1.15,2.77),46)
# Smooth leaves replace the early faceted leaf proxies, including all shared instances.
verts=[];faces=[]
for i in range(13):
    t=i/12;w=.34*math.sin(math.pi*t)**.85
    for j in range(5):
        u=j/2-1;verts.append((w*u,.11*t*t+.09*u*u*math.sin(math.pi*t),t))
for i in range(12):
    for j in range(4):
        a=i*5+j;faces.append((a,a+1,a+6,a+5))
for me in bpy.data.meshes:
    if me.name.startswith('Canopy leaf geometry'):
        me.clear_geometry();me.from_pydata(verts,[],faces);me.update()
        for p in me.polygons:p.use_smooth=True
# Replace solid distant crown volumes with clusters of modeled leaves.
for o in list(s.objects):
    if not o.name.startswith('Exterior • distant green crown'):continue
    vv=[];ff=[]
    for i in range(150):
        u=random.uniform(-1,1);a=random.uniform(0,2*math.pi);r=random.uniform(.25,1)**(1/3)
        pos=Vector((math.sqrt(1-u*u)*math.cos(a),math.sqrt(1-u*u)*math.sin(a),u))*r
        rot=Euler((random.uniform(-2,2),random.uniform(-2,2),a)).to_matrix()
        size=random.uniform(.25,.48);offset=len(vv)
        vv.extend([pos+rot@Vector(v)*size for v in verts]);ff.extend([tuple(offset+j for j in f) for f in faces])
    me=bpy.data.meshes.new('Exterior leafy crown');me.from_pydata(vv,[],ff);me.materials.append(o.data.materials[0]);me.update()
    for p in me.polygons:p.use_smooth=True
    o.data=me
for m in bpy.data.materials:
    if m.name.startswith('Canopy leaf'):
        p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Coat Weight'].default_value=.04;p.inputs['Roughness'].default_value=.68
j=bpy.data.materials['Leaf • polished primordial jade'].node_tree.nodes.get('Principled BSDF')
j.inputs['Transmission Weight'].default_value=0;j.inputs['Subsurface Weight'].default_value=.045;j.inputs['Coat Weight'].default_value=.22
for o in s.objects:
    if o.name.startswith('V-001 原豆母体 • ivory eye rim'):
        o.data.materials.clear();o.data.materials.append(bpy.data.materials['Deep teal • engraved recess'])
    if o.type=='LIGHT' and o.data.type=='AREA':
        o.data.energy*=.75
    if o.name=='Daylight • soft front':o.data.energy*=.65
bpy.data.objects['Sun • conservatory morning'].data.energy=1.5
s.world.node_tree.nodes.get('Background').inputs['Strength'].default_value=.24
s.cycles.samples=64
print('VISUAL_REFINEMENT_COMPLETE',flush=True)
