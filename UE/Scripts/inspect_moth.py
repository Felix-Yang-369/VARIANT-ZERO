import bpy,json,math
from pathlib import Path
from mathutils import Vector
R=Path(r'D:\Projects\VariantZeroUE\ArtSource\Moth');R.mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=str(R/'Original/base_basic_pbr.glb'))
report=[]
for o in bpy.context.scene.objects:
 if o.type=='MESH':
  o.data.calc_loop_triangles();report.append({'name':o.name,'vertices':len(o.data.vertices),'triangles':len(o.data.loop_triangles),'dimensions':list(o.dimensions),'bounds':[[min((o.matrix_world@v.co)[i] for v in o.data.vertices),max((o.matrix_world@v.co)[i] for v in o.data.vertices)] for i in range(3)],'materials':[m.name for m in o.data.materials]})
(R/'inspection.json').write_text(json.dumps(report,indent=2))
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=20;scene.render.resolution_x=1200;scene.render.resolution_y=900;scene.render.resolution_percentage=100
points=[o.matrix_world@v.co for o in scene.objects if o.type=='MESH' for v in o.data.vertices];center=sum(points,Vector())/len(points);extent=max((p-center).length for p in points)
scene.world.color=(.2,.2,.2)
for pos,power in [((2,-3,4),700),((-3,1,2),500)]:
 bpy.ops.object.light_add(type='AREA',location=center+Vector(pos)*extent);bpy.context.object.data.energy=power*extent*extent;bpy.context.object.data.shape='DISK';bpy.context.object.data.size=extent*3
 bpy.context.object.rotation_euler=(center-bpy.context.object.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=center+Vector((0,-2,3))*extent);cam=bpy.context.object;cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=extent*2.4;scene.camera=cam
scene.render.filepath=str(R/'inspection.png');bpy.ops.wm.save_as_mainfile(filepath=str(R/'Inspection.blend'));bpy.ops.render.render(write_still=True)
