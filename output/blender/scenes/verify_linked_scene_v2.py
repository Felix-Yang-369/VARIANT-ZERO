import bpy,os
assert bpy.data.filepath.endswith('_v2.blend')
assert len(bpy.data.libraries)==3
assert all(lib.filepath.endswith('_v2.blend') for lib in bpy.data.libraries)
path=os.path.join(os.path.dirname(os.path.abspath(__file__)),'verify_linked_scene.py')
with open(path,encoding='utf-8') as f:code=f.read()
exec(compile(code.replace('verification.json','verification-v2.json'),path,'exec'))
