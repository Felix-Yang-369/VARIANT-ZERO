"""Derive an animated engine candidate from the existing V-001 sculpt; never overwrites the source scene."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector, Matrix
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'ArtSource'/'Chuya'/'v1'; OUT.mkdir(parents=True,exist_ok=True)
WEB_ROOT=ROOT.parent if (ROOT.parent/'package.json').exists() else ROOT.parent/'零号变种'
SOURCE=WEB_ROOT/'output'/'blender'/'原初生态研究所_V-001_原豆母体_v2.blend'
bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
with bpy.data.libraries.load(str(SOURCE),link=False) as (a,b):
    b.collections=[n for n in a.collections if n.startswith('05')]
c=b.collections[0]; bpy.context.scene.collection.children.link(c)
body=next(o for o in c.all_objects if 'ivory seed body' in o.name)
origin=Vector((0,body.location.y,.60)); scale=.22
deps=bpy.context.evaluated_depsgraph_get()
pieces=[]
bone_points={'root':((0,0,0),(0,0,.18)), 'body':((0,0,.3),(0,0,2.4)),
 'foot_L':((-.4,0,.32),(-.4,-.4,.13)), 'foot_R':((.4,0,.32),(.4,-.4,.13)),
 'crown':((0,0,2.4),(0,0,3.4))}
for side,s in [('L',-1),('R',1)]:
    points=[(s*.34,.04,2.44),(s*.9,.08,3.45),(s*1.8,.04,3.7),(s*1.93,-.15,2.62)]
    for i in range(3):bone_points[f'leaf_{side}_{i+1}']=(points[i],points[i+1])
for source in list(c.all_objects):
    if source.type not in {'MESH','CURVE'}:continue
    mesh=bpy.data.meshes.new_from_object(source.evaluated_get(deps),depsgraph=deps)
    ob=bpy.data.objects.new(source.name,mesh); bpy.context.scene.collection.objects.link(ob)
    mesh.transform(Matrix.Scale(scale,4) @ Matrix.Translation(-origin) @ source.matrix_world)
    name=source.name.lower()
    center=sum((v.co for v in mesh.vertices),Vector())/max(1,len(mesh.vertices))
    side='L' if center.x<0 else 'R'
    for key in bone_points:ob.vertex_groups.new(name=key)
    for v in mesh.vertices:
        if 'foot' in name or 'ankle' in name:weights={f'foot_{side}':1}
        elif 'drooping leaf' in name or 'ear stem' in name:
            local=v.co/scale
            centers=[(Vector(bone_points[f'leaf_{side}_{i}'][0])+Vector(bone_points[f'leaf_{side}_{i}'][1]))*.5 for i in range(1,4)]
            distances=sorted(((local-p).length,i+1) for i,p in enumerate(centers))[:2]
            inv=[1/max(.05,d)**3 for d,i in distances]; total=sum(inv)
            weights={f'leaf_{side}_{i}':w/total for (_,i),w in zip(distances,inv)}
        elif 'crown' in name:weights={'crown':1}
        else:weights={'body':1}
        for group,weight in weights.items():ob.vertex_groups[group].add([v.index],weight,'REPLACE')
    pieces.append(ob)
for o in list(c.all_objects):bpy.data.objects.remove(o,do_unlink=True)
bpy.ops.object.select_all(action='DESELECT')
for o in pieces:o.select_set(True)
bpy.context.view_layer.objects.active=pieces[0]; bpy.ops.object.join(); high=bpy.context.object; high.name='Chuya_High_BakeSource'
high.data.calc_loop_triangles(); high_count=len(high.data.loop_triangles)
low=high.copy();low.data=high.data.copy();low.name='SK_Chuya';bpy.context.scene.collection.objects.link(low)
bpy.ops.object.select_all(action='DESELECT');low.select_set(True);bpy.context.view_layer.objects.active=low
dec=low.modifiers.new('Engine triangle budget','DECIMATE');dec.ratio=min(1,22000/max(1,high_count));bpy.ops.object.modifier_apply(modifier=dec.name)
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(angle_limit=math.radians(65),island_margin=.008);bpy.ops.object.mode_set(mode='OBJECT')
low.data.calc_loop_triangles();low_count=len(low.data.loop_triangles)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=16
prefs=bpy.context.preferences.addons['cycles'].preferences
try:
    prefs.compute_device_type='OPTIX';prefs.get_devices()
    for device in prefs.devices:device.use=device.type=='OPTIX'
    scene.cycles.device='GPU'
except Exception:scene.cycles.device='CPU'
scene.render.bake.use_selected_to_active=True;scene.render.bake.cage_extrusion=.006;scene.render.bake.max_ray_distance=.02;scene.render.bake.margin=12
atlas=bpy.data.materials.new('M_Chuya_Baked');atlas.use_nodes=True;low.data.materials.clear();low.data.materials.append(atlas)
for p in low.data.polygons:p.material_index=0
images={}
def bake(channel,kind):
    image=bpy.data.images.new('T_Chuya_'+channel,2048,2048,alpha=False)
    if channel!='BaseColor' and channel!='Emission':image.colorspace_settings.name='Non-Color'
    node=atlas.node_tree.nodes.new('ShaderNodeTexImage');node.image=image;atlas.node_tree.nodes.active=node
    backups=[]
    if kind=='EMIT':
        for mat in high.data.materials:
            if not mat or not mat.use_nodes:continue
            nodes=mat.node_tree.nodes;links=mat.node_tree.links;p=next((n for n in nodes if n.type=='BSDF_PRINCIPLED'),None);output=next(n for n in nodes if n.type=='OUTPUT_MATERIAL')
            if p is None:continue
            old=output.inputs['Surface'].links[0].from_socket if output.inputs['Surface'].links else None
            emit=nodes.new('ShaderNodeEmission')
            socket=p.inputs['Base Color' if channel=='BaseColor' else 'Roughness' if channel=='Roughness' else 'Emission Color']
            if socket.is_linked:links.new(socket.links[0].from_socket,emit.inputs['Color'])
            else:
                value=socket.default_value
                emit.inputs['Color'].default_value=tuple(value) if hasattr(value,'__len__') else (value,value,value,1)
            emit.inputs['Strength'].default_value=p.inputs['Emission Strength'].default_value if channel=='Emission' else 1
            links.new(emit.outputs[0],output.inputs['Surface']);backups.append((mat,emit,output,old))
    bpy.ops.object.select_all(action='DESELECT'); high.select_set(True);low.select_set(True);bpy.context.view_layer.objects.active=low
    bpy.ops.object.bake(type=kind)
    for mat,emit,output,old in backups:
        if old:mat.node_tree.links.new(old,output.inputs['Surface'])
        mat.node_tree.nodes.remove(emit)
    image.filepath_raw=str(OUT/f'T_Chuya_{channel}.png');image.file_format='PNG';image.save();images[channel]=image
    print('VZ_BAKED',channel,flush=True)
for channel,kind in [('BaseColor','EMIT'),('Normal','NORMAL'),('Roughness','EMIT'),('Emission','EMIT')]:bake(channel,kind)
ns=atlas.node_tree.nodes;ls=atlas.node_tree.links;p=ns.get('Principled BSDF')
for channel,socket in [('BaseColor','Base Color'),('Roughness','Roughness'),('Emission','Emission Color')]:
    n=next(n for n in ns if n.type=='TEX_IMAGE' and n.image==images[channel]);ls.new(n.outputs['Color'],p.inputs[socket])
p.inputs['Emission Strength'].default_value=1;p.inputs['Coat Weight'].default_value=.3;p.inputs['Subsurface Weight'].default_value=.06
n=next(n for n in ns if n.type=='TEX_IMAGE' and n.image==images['Normal']);normal=ns.new('ShaderNodeNormalMap');ls.new(n.outputs['Color'],normal.inputs['Color']);ls.new(normal.outputs['Normal'],p.inputs['Normal'])
high.hide_render=True;high.hide_set(True)
bpy.ops.object.select_all(action='DESELECT');bpy.ops.object.armature_add();arm=bpy.context.object;arm.name='Chuya_Rig'
bpy.ops.object.mode_set(mode='EDIT');arm.data.edit_bones.remove(arm.data.edit_bones[0])
for name,(head,tail) in bone_points.items():
    b=arm.data.edit_bones.new(name);b.head=Vector(head)*scale;b.tail=Vector(tail)*scale
for b in arm.data.edit_bones:
    if b.name=='root':continue
    parent='root' if b.name=='body' else 'body'
    if b.name.startswith('leaf_') and not b.name.endswith('_1'):parent=b.name[:-1]+str(int(b.name[-1])-1)
    b.parent=arm.data.edit_bones[parent]
bpy.ops.object.mode_set(mode='OBJECT');low.parent=arm
mod=low.modifiers.new('Living skeleton','ARMATURE');mod.object=arm
actions=[]
scene.render.fps=30
for label,frames in [('Idle',60),('Walk',30)]:
    arm.animation_data_create();action=bpy.data.actions.new('A_Chuya_'+label);arm.animation_data.action=action
    for frame in range(1,frames+2,3):
        phase=(frame-1)/frames*2*math.pi
        for b in arm.pose.bones:b.rotation_mode='XYZ';b.rotation_euler=(0,0,0);b.location=(0,0,0)
        arm.pose.bones['body'].location.y=(.003 if label=='Idle' else .008)*math.sin(phase*2)
        for side,s in [('L',-1),('R',1)]:
            for i in range(1,4):arm.pose.bones[f'leaf_{side}_{i}'].rotation_euler.x=(.04 if label=='Idle' else .09)*math.sin(phase+s*.7-i*.5)
            if label=='Walk':arm.pose.bones[f'foot_{side}'].rotation_euler.x=.4*math.sin(phase+s*math.pi/2)
        arm.pose.bones['crown'].rotation_euler.x=.08*math.sin(phase)
        for b in arm.pose.bones:
            b.keyframe_insert('rotation_euler',frame=frame);b.keyframe_insert('location',frame=frame)
    action.use_fake_user=True;actions.append(action)
def export(name,action=None):
    arm.animation_data.action=action
    scene.frame_start=1;scene.frame_end=60 if not action or 'Idle' in action.name else 30;scene.frame_set(1)
    bpy.ops.object.select_all(action='DESELECT');arm.select_set(True);low.select_set(True);bpy.context.view_layer.objects.active=arm
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,bake_anim=action is not None,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
export('SK_Chuya')
for action in actions:export(action.name,action)
arm.animation_data.action=actions[0];scene.frame_set(16)
for img in images.values():img.pack()
scene.world.color=(.15,.15,.15)
def area(name,location,energy,color,size):
    data=bpy.data.lights.new(name,'AREA');data.energy=energy;data.color=color;data.shape='DISK';data.size=size
    o=bpy.data.objects.new(name,data);scene.collection.objects.link(o);o.location=location;o.rotation_euler=(Vector((0,0,.45))-o.location).to_track_quat('-Z','Y').to_euler()
area('Key',(2,-3,3),400,(1,.88,.7),3);area('Fill',(-2,-2,1.5),250,(.5,.9,1),2);area('Rim',(0,2,2),500,(.4,1,.8),2)
bpy.ops.object.camera_add(location=(1.3,-2.6,1.25));camera=bpy.context.object;camera.rotation_euler=(Vector((0,0,.5))-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.type='ORTHO';camera.data.ortho_scale=1.35;scene.camera=camera
scene.render.resolution_x=960;scene.render.resolution_y=960;scene.render.resolution_percentage=100;scene.render.film_transparent=True
scene.render.filepath=str(OUT/'Chuya_bound_preview.png')
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Chuya_bound_v1.blend'))
bpy.ops.render.render(write_still=True)
report={'source':str(SOURCE),'status':'rigged engine candidate; visual approval and engine verification required','high_triangles':high_count,'lod0_triangles':low_count,'bones':len(arm.data.bones),'vertices':len(low.data.vertices),'unweighted_vertices':sum(not v.groups for v in low.data.vertices),'uv_layers':len(low.data.uv_layers),'textures':[p.name for p in OUT.glob('*.png')],'animations':[a.name for a in actions],'height_m':low.dimensions.z}
(OUT/'build-report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print('VZ_BUILD_REPORT',json.dumps(report),flush=True)
