import bpy,math,json
from pathlib import Path
from mathutils import Matrix,Vector
R=Path(r'D:\Projects\VariantZeroUE\ArtSource\Moth');O=R/'v1';O.mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=str(R/'Original/base_basic_pbr.glb'))
m=next(o for o in bpy.context.scene.objects if o.type=='MESH');m.name='SK_Moth'
s=.70/1.8952190876
transform=Matrix(((-s,0,0,0),(0,0,-s,0),(0,-s,0,0),(0,0,0,1)))
m.data.transform(transform@m.matrix_world);m.matrix_world=Matrix.Identity(4)
mat=m.data.materials[0];textures={}
for node in mat.node_tree.nodes:
 if node.type=='TEX_IMAGE' and node.image:
  im=node.image;name=im.name.lower();channel='Normal' if 'normal' in name else 'ORM' if 'metal' in name or 'rough' in name else 'BaseColor'
  im.filepath_raw=str(O/('T_Moth_'+channel+'.png'));im.file_format='PNG';im.save();textures[channel]={'file':Path(im.filepath_raw).name,'size':list(im.size)}
if 'BaseColor' not in textures:raise RuntimeError('Missing color texture')
bpy.ops.object.armature_add();arm=bpy.context.object;arm.name='Moth_Rig'
bpy.ops.object.mode_set(mode='EDIT');arm.data.edit_bones.remove(arm.data.edit_bones[0])
bones={'root':((0,0,-.08),(0,0,0)),'body':((0,.06,0),(0,-.08,0)),'head':((0,-.08,0),(0,-.15,0)),'abdomen':((0,.03,0),(0,.22,0))}
for side,sign in [('L',-1),('R',1)]:
 bones['wing_fore_'+side]=((sign*.035,0,0),(sign*.31,0,0))
 bones['wing_hind_'+side]=((sign*.035,0,0),(sign*.31,0,0))
for name,(a,b) in bones.items():
 bone=arm.data.edit_bones.new(name);bone.head=a;bone.tail=b
for bone in arm.data.edit_bones:
 if bone.name!='root':bone.parent=arm.data.edit_bones['root' if bone.name=='body' else 'body']
bpy.ops.object.mode_set(mode='OBJECT')
groups={n:m.vertex_groups.new(name=n) for n in bones}
for v in m.data.vertices:
 x,y,z=v.co;side='L' if x<0 else 'R';ax=abs(x)
 head=max(0,min(1,(-y-.10)/.025));w=max(0,min(1,(ax-.035)/.045))*(1-head);hind=max(0,min(1,(y+.01)/.14))
 weights={'head':head,'wing_fore_'+side:w*(1-hind),'wing_hind_'+side:w*hind,'abdomen':max(0,1-head-w)*max(0,min(1,(y-.035)/.04))}
 weights['body']=max(0,1-sum(weights.values()))
 for n,w in weights.items():
  if w>0:groups[n].add([v.index],w,'REPLACE')
m.parent=arm;mod=m.modifiers.new('Wing and body deformation','ARMATURE');mod.object=arm
scene=bpy.context.scene;scene.render.fps=30;actions=[]
for name,frames,amplitude in [('Hover',30,.30),('Fly',20,.48)]:
 arm.animation_data_create();action=bpy.data.actions.new('A_Moth_'+name);arm.animation_data.action=action;action.use_fake_user=True;actions.append((action,frames))
 for frame in range(1,frames+2):
  phase=(frame-1)/frames*math.tau
  for b in arm.pose.bones:b.rotation_mode='XYZ';b.rotation_euler=(0,0,0);b.location=(0,0,0)
  arm.pose.bones['body'].location.z=.005*math.sin(phase)
  for side,sign in [('L',-1),('R',1)]:
   for part,delay in [('fore',0),('hind',.08)]:arm.pose.bones['wing_'+part+'_'+side].rotation_euler.x=sign*amplitude*math.sin(phase*2-delay)
  arm.pose.bones['abdomen'].rotation_euler.x=.04*math.sin(phase)
  for b in arm.pose.bones:b.keyframe_insert('rotation_euler',frame=frame);b.keyframe_insert('location',frame=frame)
def export(name,action=None,frames=30):
 arm.animation_data.action=action;scene.frame_start=1;scene.frame_end=frames;scene.frame_set(1)
 for b in arm.pose.bones:b.rotation_euler=(0,0,0);b.location=(0,0,0)
 bpy.ops.object.select_all(action='DESELECT');m.select_set(True);arm.select_set(True);bpy.context.view_layer.objects.active=arm
 bpy.ops.export_scene.fbx(filepath=str(O/(name+'.fbx')),use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,bake_anim=action is not None,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
export('SK_Moth')
for action,frames in actions:export(action.name,action,frames)
arm.animation_data.action=actions[0][0];scene.frame_set(5)
scene.render.engine='CYCLES';scene.cycles.samples=24;scene.render.resolution_x=1280;scene.render.resolution_y=720;scene.render.resolution_percentage=100;scene.world.color=(.15,.15,.15)
for pos,energy in [((1,-1,2),180),((-1,0,1),100)]:
 bpy.ops.object.light_add(type='AREA',location=pos);l=bpy.context.object;l.data.energy=energy;l.data.size=2;l.rotation_euler=(-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(.5,-.8,1.2));cam=bpy.context.object;cam.rotation_euler=(-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=1.1;scene.camera=cam
for im in bpy.data.images:
 if im.size[0]>0:im.pack()
scene.render.filepath=str(O/'Moth_bound.png');bpy.ops.wm.save_as_mainfile(filepath=str(O/'Moth_bound.blend'));bpy.ops.render.render(write_still=True)
m.data.calc_loop_triangles();report={'triangles':len(m.data.loop_triangles),'vertices':len(m.data.vertices),'bones':len(arm.data.bones),'unweighted':sum(not v.groups for v in m.data.vertices),'textures':textures,'uv_layers':len(m.data.uv_layers),'span_m':.7,'animations':[a.name for a,n in actions],'status':'rigged candidate, engine validation pending'}
(O/'build-report.json').write_text(json.dumps(report,indent=2));print('VZ_MOTH_BUILD',json.dumps(report),flush=True)
