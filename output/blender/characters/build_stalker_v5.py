"""V5: preserve V4, refine organic surfaces and fuse hand/foot junctions."""
import os
base=os.path.dirname(os.path.abspath(__file__))
source=open(os.path.join(base,'build_stalker_v4.py'),encoding='utf-8').read()
def change(old,new):
 global source
 assert old in source,old
 source=source.replace(old,new)
change('v4','v5');change('V4','V5')
change("curves('眼窝 • 内嵌边缘',[path],.012,DARK)","curves('眼窝 • 内嵌边缘',[path],.006,DARK)")
change("p.inputs['Specular IOR Level'].default_value=.15","p.inputs['Specular IOR Level'].default_value=.03")
change("pbs(PURPLE).inputs['Roughness'].default_value=.40","""pbs(PURPLE).inputs['Roughness'].default_value=.50
for node in PURPLE.node_tree.nodes:
 if node.type=='VALTORGB':
  for el in node.color_ramp.elements:el.color=(el.color[0]*.65,el.color[1]*.65,el.color[2]*.78,1)
""")
change('x/spread+.045*sin(z*5)+.10*z','x/spread+.13*sin(z*4+x*2)+.06*sin(z*13+x*5)+.17*z')
change("(.21*u*u+.018*sin(t*30+u*5))","(.23*u*u+.063*sin(t*19+u*3.2)*abs(u)**.7+.023*sin(t*42+u*5)*abs(u)**1.5)")
change("(-.19,.30,1.75)","(-.19,.48,1.48)")
change("(1.14,.54,1.63)","(1.24,.72,1.42)")
change("(1.52,1.57,1.56)","(1.62,1.77,1.36)")
# New growth knots perturb the surface instead of relying on parallel color streaks.
change("if not CLAY:relief(o,.009)","""if not CLAY:
  relief(o,.018)
  for v in o.data.vertices:
   v.co += v.normal*.005*noise.noise(v.co*18)
""")
change("# Asset interface and studio.","""
# Join the actual palm/toe meshes into continuous volumes. Fine longitudinal
# curves remain editable while the obvious intersection seam is removed.
if not CLAY:
 C=BODY
 fusedmat=WOOD.copy();fusedmat.name='树皮 • 融合根结'
 ns=fusedmat.node_tree.nodes;ls=fusedmat.node_tree.links
 tc=next(n for n in ns if n.type=='TEX_COORD')
 mp=next(n for n in ns if n.type=='VECT_MATH' and n.operation=='MULTIPLY')
 ls.new(tc.outputs['Generated'],mp.inputs[0]);mp.inputs[1].default_value=(2.6,2.0,.7)
 for label,palm_prefix,finger_prefix in [('手','掌部 • 根瘤','手爪'),('足','足掌 • 根结','足爪')]:
  palms=sorted([o for o in BODY.objects if o.name.startswith(palm_prefix)],key=lambda o:o.name)
  for side,palm in enumerate(palms):
   parts=[palm]+[o for o in BODY.objects if o.type=='MESH' and o.name.startswith(finger_prefix+' '+str(side)+'.')]
   assert len(parts)==4,(label,side,[o.name for o in parts])
   bpy.ops.object.select_all(action='DESELECT')
   for o in parts:o.select_set(True)
   bpy.context.view_layer.objects.active=palm
   bpy.ops.object.convert(target='MESH');bpy.ops.object.join()
   o=bpy.context.object;o.name=label+'部 • 连续融合根结 '+str(side)
   bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
   o.data.remesh_voxel_size=.012;bpy.ops.object.voxel_remesh()
   sm=o.modifiers.new('根部过渡平滑','SMOOTH');sm.factor=.65;sm.iterations=4
   bpy.ops.object.modifier_apply(modifier=sm.name)
   o.data.materials.clear();o.data.materials.append(fusedmat)
   for poly in o.data.polygons:poly.use_smooth=True
   relief(o,.009)
   for curve in list(BODY.objects):
    if curve.type=='CURVE' and curve.name.startswith(finger_prefix+' '+str(side)+'.'):
     shrink=curve.modifiers.new('贴合融合根面','SHRINKWRAP');shrink.target=o;shrink.wrap_method='NEAREST_SURFACEPOINT';shrink.offset=.002
 C=FACE
 # Branching life veins lie on the convex crystal surface, not a flat pupil.
 crystal=mat('紫晶 • 生命脉络',(.55,.02,.95),.3,emit=3.5)
 for sg in [-1,1]:
  paths=[];ang=-sg*.39
  localpaths=[[(.003*sin(k/30*8),-.18+k/30*.36) for k in range(31)]]
  for side in [-1,1]:
   for z0 in [-.11,-.02,.07]:localpaths.append([(side*.046*(k/15),z0+.072*(k/15)**.65) for k in range(16)])
  for localpath in localpaths:
   path=[]
   for xx,zz in localpath:
    x=sg*.335+xx*cos(ang)-zz*sin(ang);z=-.09+xx*sin(ang)+zz*cos(ang)
    radial=min(1,((xx/.095)**2+(zz/.238)**2)**.5)
    y=my(x,z)-.019-.045*(1-radial*radial)
    path.append(hp((x,y,z)))
   paths.append(path)
  curves('紫晶 • 分叉生命细脉',paths,.0009,crystal)

# Asset interface and studio.
""")
exec(compile(source,os.path.join(base,'build_stalker_v4.py'),'exec'))
