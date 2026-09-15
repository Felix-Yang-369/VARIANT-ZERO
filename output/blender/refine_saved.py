import bpy,os
out=os.path.dirname(os.path.abspath(__file__))
exec(compile(open(os.path.join(out,'configure_gpu.py'),encoding='utf-8').read(),'configure_gpu.py','exec'))
exec(compile(open(os.path.join(out,'refine_scene.py'),encoding='utf-8').read(),'refine_scene.py','exec'))
s=bpy.context.scene
s.camera=bpy.data.objects['CAM 01 • 原初生态研究所全景']
s.render.filepath=os.path.join(out,'原初生态研究所_全景_v1.png')
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out,'原初生态研究所_V-001_原豆母体_v1.blend'))
bpy.ops.render.render(write_still=True)
s.camera=bpy.data.objects['CAM 02 • 原豆母体与培养台'];s.render.resolution_x=1100;s.render.resolution_y=1100
s.render.filepath=os.path.join(out,'V-001_原豆母体_培养台近景_v1.png');bpy.ops.render.render(write_still=True)
print('REFINED_RENDERS_COMPLETE',flush=True)
