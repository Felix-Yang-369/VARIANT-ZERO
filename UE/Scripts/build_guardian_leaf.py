"""Original rigid guardian leaf; native actor drives its opening rotation."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector
OUT=Path(r'D:\Projects\VariantZeroUE\ArtSource\GuardianLeaf');OUT.mkdir(parents=True,exist_ok=True)
if (OUT/'GuardianLeaf.blend').exists():raise RuntimeError('Preserve the existing authored leaf')
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
m=bpy.data.materials.new('JadeLeaf');m.diffuse_color=(.04,.3,.15,1)
v=bpy.data.materials.new('LivingVeins');v.diffuse_color=(.2,.75,.4,1)
verts=[];faces=[];rows=24;cols=10
for side in [1,-1]:
    for i in range(rows+1):
        t=i/rows;width=max(.006,.5*math.sin(math.pi*t)**.8)*(1+.055*math.sin(t*math.pi*12))
        for j in range(cols+1):
            u=-1+2*j/cols
            verts.append((u*width,.075*math.sin(t*math.pi)*(1-u*u)+side*.009,t-.5))
stride=(rows+1)*(cols+1)
for side in range(2):
    for i in range(rows):
        for j in range(cols):
            a=side*stride+i*(cols+1)+j
            f=(a,a+1,a+cols+2,a+cols+1)
            faces.append(f if side else tuple(reversed(f)))
for i in range(rows):
    for j in [0,cols]:
        a=i*(cols+1)+j;b=a+cols+1;faces.append((a,b,b+stride,a+stride))
for i in [0,rows]:
    for j in range(cols):
        a=i*(cols+1)+j;faces.append((a,a+stride,a+stride+1,a+1))
mesh=bpy.data.meshes.new('LeafSurface');mesh.from_pydata(verts,[],faces);mesh.update()
o=bpy.data.objects.new('SM_GuardianLeaf',mesh);bpy.context.collection.objects.link(o);mesh.materials.append(m)
for p in mesh.polygons:p.use_smooth=True
parts=[o]
def vein(points,radius):
    curve=bpy.data.curves.new('Vein','CURVE');curve.dimensions='3D';curve.bevel_depth=radius;curve.bevel_resolution=1
    path=curve.splines.new('POLY');path.points.add(len(points)-1)
    for p,co in zip(path.points,points):p.co=(*co,1)
    ob=bpy.data.objects.new('Vein',curve);bpy.context.collection.objects.link(ob);curve.materials.append(v)
    bpy.ops.object.select_all(action='DESELECT');ob.select_set(True);bpy.context.view_layer.objects.active=ob;bpy.ops.object.convert(target='MESH');parts.append(bpy.context.object)
vein([(0,.075*math.sin(i/24*math.pi)+.014,i/24-.5) for i in range(25)],.006)
for t in [.15,.27,.39,.51,.63,.75]:
    for direction in [-1,1]:
        points=[]
        for k in range(7):
            u=k/6*.85;z=t+.10*u;w=.5*math.sin(math.pi*z)**.8
            points.append((direction*u*w,.075*math.sin(z*math.pi)*(1-u*u)+.014,z-.5))
        vein(points,.0025)
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:ob.select_set(True)
bpy.context.view_layer.objects.active=o;bpy.ops.object.join();o.name='SM_GuardianLeaf'
bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
o.data.calc_loop_triangles();triangles=len(o.data.loop_triangles)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'GuardianLeaf.blend'))
bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_GuardianLeaf.fbx'),use_selection=True,apply_unit_scale=True,axis_forward='-Y',axis_up='Z',add_leaf_bones=False)
(OUT/'build.json').write_text(json.dumps({'triangles':triangles,'dimensions_m':list(o.dimensions),'materials':2,'origin':'original local procedural mesh','deformation':'rigid native leaf rotation; not a skeletal creature'},indent=2))
assert triangles<2500
unreal_export=OUT/'SM_GuardianLeaf.fbx';assert unreal_export.stat().st_size>1000
print('VZ_GUARDIAN_LEAF_BUILD_PASS',triangles)
