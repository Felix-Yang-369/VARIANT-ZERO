import bpy,os
from mathutils import Vector
out=os.path.dirname(os.path.abspath(__file__))
exec(compile(open(os.path.join(out,'configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
s=bpy.context.scene;o=bpy.data.objects['CAM 01 • 原初生态研究所全景']
o.rotation_euler=(Vector((0,2,3.45))-o.location).to_track_quat('-Z','Y').to_euler()
s.camera=o;s.render.resolution_x=1600;s.render.resolution_y=1100
s.render.filepath=os.path.join(out,'原初生态研究所_全景_v1.png')
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out,'原初生态研究所_V-001_原豆母体_v1.blend'))
bpy.ops.render.render(write_still=True)
exec(compile(open(os.path.join(out,'verify_scene.py'),encoding='utf-8').read(),'verify_scene.py','exec'))
