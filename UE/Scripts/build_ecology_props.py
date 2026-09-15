"""Original field scanner and terrain mesh; no external generation or source scene edits."""
import bpy, math, json
from mathutils import Vector
from pathlib import Path
OUT=Path(r'D:\Projects\VariantZeroUE\ArtSource\Ecology');OUT.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
mats=[]
for name,col in [('Ceramic',(.68,.76,.66,1)),('Grip',(.026,.053,.044,1)),('LivingCore',(.04,.7,.36,1)),('Brass',(.38,.24,.08,1))]:
    m=bpy.data.materials.new(name);m.diffuse_color=col;mats.append(m)
parts=[]
def cylinder(name,r,depth,z,mat):
    bpy.ops.mesh.primitive_cylinder_add(vertices=24,radius=r,depth=depth,location=(0,0,z));o=bpy.context.object;o.name=name;o.data.materials.append(mats[mat]);parts.append(o)
    bevel=o.modifiers.new('Machined edge','BEVEL');bevel.width=.0015;bevel.segments=2
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    return o
cylinder('Grip',.017,.14,0,1)
cylinder('Lower cap',.021,.025,-.085,3)
cylinder('Ceramic shaft',.013,.32,.225,0)
for z in [-.06,-.035,-.01,.015,.04,.065]:cylinder('Grip ring',.018,.004,z,3)
cylinder('Sensor base',.028,.025,.39,3)
bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=12,radius=1,location=(0,0,.455))
o=bpy.context.object;o.name='Living seed';o.scale=(.023,.023,.062);o.data.materials.append(mats[2]);parts.append(o)
for i in range(4):
    a=i*math.tau/4
    points=[Vector((math.cos(a)*r,math.sin(a)*r,z)) for r,z in [(.018,.39),(.04,.43),(.04,.48),(.012,.52)]]
    for x,y in zip(points,points[1:]):
        o=cylinder('Seed guard',.004,(y-x).length,0,0);o.location=(x+y)/2;o.rotation_euler=(y-x).to_track_quat('Z','Y').to_euler()
bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();staff=bpy.context.object;staff.name='SM_FieldScanner'
bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR');bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_FieldScanner.fbx'),use_selection=True,apply_unit_scale=True,axis_forward='-Y',axis_up='Z',add_leaf_bones=False)
staff.hide_set(True);staff.select_set(False)
# The courtyard stays level. Irregular ridges begin beyond the greenhouse, not in traversable paths.
n=100;verts=[];faces=[]
for j in range(n+1):
    for i in range(n+1):
        x=-125+250*i/n;y=-125+250*j/n;r=math.hypot(x,y)
        blend=min(1,max(0,(r-22)/24))
        h=(5+4*math.sin(x*.08+y*.03)+3*math.cos(y*.11-x*.025)+1.3*math.sin(x*.23)*math.cos(y*.17))*blend-.65
        verts.append((x,y,h))
for j in range(n):
    for i in range(n):
        a=j*(n+1)+i;faces.append((a,a+1,a+n+2,a+n+1))
mesh=bpy.data.meshes.new('Terrain');mesh.from_pydata(verts,[],faces);mesh.update()
terrain=bpy.data.objects.new('SM_InstituteTerrain',mesh);bpy.context.collection.objects.link(terrain)
terrain.select_set(True);bpy.context.view_layer.objects.active=terrain
uv=mesh.uv_layers.new(name='UVMap')
for p in mesh.polygons:
    p.use_smooth=True
    for li in p.loop_indices:
        co=mesh.vertices[mesh.loops[li].vertex_index].co;uv.data[li].uv=(co.x/5,co.y/5)
bpy.ops.export_scene.fbx(filepath=str(OUT/'SM_InstituteTerrain.fbx'),use_selection=True,apply_unit_scale=True,axis_forward='-Y',axis_up='Z')
staff.hide_set(False);bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'EcologyProps.blend'))
report={o.name:{'vertices':len(o.data.vertices),'triangles':sum(len(p.vertices)-2 for p in o.data.polygons)} for o in [staff,terrain]}
(OUT/'geometry.json').write_text(json.dumps(report,indent=2),encoding='utf8')
