"""Author a separate P0 art-development map. Existing template and P0 map remain unchanged."""
import unreal,math,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');BASE='/Game/VariantZero/Environment';PATH='/Game/VariantZero/Maps/P0_Conservatory'
if unreal.EditorAssetLibrary.does_asset_exist(PATH) and (ROOT/'Docs/conservatory-build.json').exists():raise RuntimeError('Completed map already exists; inspect instead of overwriting authored work')
tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
def material(name,color,rough=.5,metal=0,emit=0):
    p=BASE+'/Materials/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(p):return unreal.load_asset(p)
    m=tools.create_asset(name,BASE+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    n=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);n.constant=unreal.LinearColor(*color,1);lib.connect_material_property(n,'',unreal.MaterialProperty.MP_BASE_COLOR)
    for value,prop in [(rough,unreal.MaterialProperty.MP_ROUGHNESS),(metal,unreal.MaterialProperty.MP_METALLIC)]:
        node=lib.create_material_expression(m,unreal.MaterialExpressionConstant);node.r=value;lib.connect_material_property(node,'',prop)
    if emit:
        node=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);node.constant=unreal.LinearColor(*(v*emit for v in color),1);lib.connect_material_property(node,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);return m
mats={'ivory':material('M_Ivory',(.64,.69,.56),.3,.2),'floor':material('M_Mineral',(.18,.28,.23),.5,.05),'trim':material('M_Brass',(.36,.27,.12),.3,.65),'dark':material('M_Soil',(.028,.06,.033),.94),'leaf':material('M_LivingLeaf',(.06,.28,.14),.42),'dead':material('M_DormantLeaf',(.28,.2,.065),.8),'cyan':material('M_Biolight',(.025,.58,.39),.3,.1,2),'wall':material('M_GreenGlassOpaque',(.18,.34,.31),.22,.25),'petal':material('M_Petal',(.82,.65,.30),.45)}
sub=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    if not sub.load_level(PATH):raise RuntimeError('Could not reopen unfinished generated map')
else:
    if not sub.new_level(PATH):raise RuntimeError('Could not create map')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world.get_world_settings().set_editor_property('default_game_mode',unreal.load_class(None,'/Script/VariantZeroUE.VZPrototypeMode'))
meshes={n:unreal.load_asset('/Engine/BasicShapes/'+n) for n in ['Cube','Sphere','Cylinder']}
count=0
def mesh(name,loc,scale,mat='ivory',shape='Cube',rot=None,collision=True,tag=None):
    global count
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc),rot or unreal.Rotator())
    a.set_actor_label(name);c=a.static_mesh_component;c.set_static_mesh(meshes[shape]);c.set_material(0,mats[mat]);a.set_actor_scale3d(unreal.Vector(*scale))
    c.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS if collision else unreal.CollisionEnabled.NO_COLLISION)
    if tag:a.tags=[tag]
    count+=1;return a
def beam(name,a,b,r=.12,mat='ivory'):
    delta=unreal.Vector(*(b[i]-a[i] for i in range(3)))
    mid=[(a[i]+b[i])/2 for i in range(3)]
    return mesh(name,mid,(r,r,delta.length()/100),mat,'Cylinder',unreal.MathLibrary.make_rot_from_z(delta))
mesh('Mineral foundation',(0,0,-30),(30,24,.6),'floor')
for x in range(-1300,1400,200):mesh('Ivory walkway',(x,0,1),(1.96,4,.035),'ivory',collision=False)
for y in [-215,215]:mesh('Embedded living light',(0,y,3),(28,.028,.025),'cyan',collision=False)
for x in [-1450,1450]:mesh('Perimeter plinth',(x,0,40),(.5,24,.8))
for y in [-1150,1150]:mesh('Perimeter plinth',(0,y,40),(29,.5,.8))
# Structural ribs: elliptical greenhouse vault, not individual detailed concept-art reconstruction.
for y in [-1050,-525,0,525,1050]:
    for side in [-1,1]:beam('Structural column',(side*1370,y,60),(side*1370,y,460),.22)
    pts=[(1370*math.cos(math.pi*i/20),y,460+430*math.sin(math.pi*i/20)) for i in range(21)]
    for a,b in zip(pts,pts[1:]):beam('Vault rib',a,b,.19)
for x in [-1370,-900,0,900,1370]:
    z=460+430*math.sqrt(max(0,1-(x/1370)**2));beam('Longitudinal support',(x,-1050,z),(x,1050,z),.12,'trim')
for x in [-1400,1400]:
    for y in [-800,-275,275,800]:mesh('Jade wall panel',(x,y,245),(.08,5,3.5),'wall')
for y in [-1100,1100]:
    for x in [-1000,-500,0,500,1000]:mesh('Low garden enclosure',(x,y,100),(4.8,.08,1.4),'wall')
for x,y in [(-950,-650),(-950,650),(950,-650),(950,650)]:
    mesh('Cultivation island',(x,y,40),(4,4,.8),'ivory','Cylinder')
    mesh('Cultivation soil',(x,y,83),(3.65,3.65,.08),'dark','Cylinder',collision=False)
    for j in range(7):
        angle=j*2.4;px=x+110*math.cos(angle);py=y+110*math.sin(angle)
        beam('Plant stem',(px,py,88),(px,py,180+j*9),.026,'leaf')
        for side in [-1,1]:mesh('Broad foliage',(px+side*28,py,165+j*9),(.7,.25,.09),'leaf','Sphere',unreal.Rotator(side*25,j*35,0),False)
# Actual obstructing planter used by the runtime navigation check.
mesh('Navigation test planter',(0,640,70),(.8,7,1.4),'ivory')
mesh('Navigation planter soil',(0,640,142),(.7,6.8,.05),'dark',collision=False)
mesh('First flower cultivation bed',(650,0,40),(2.5,2.5,.8),'ivory','Cylinder')
mesh('First flower soil',(650,0,83),(2.25,2.25,.06),'dark','Cylinder',collision=False)
for i in range(5):
    x=650+65*math.cos(i*2.4);y=65*math.sin(i*2.4)
    beam('First flower stem',(x,y,84),(x,y,140),.022,'leaf')
    for j in range(5):
        angle=j*math.tau/5
        mesh('Awaiting spring',(x+16*math.cos(angle),y+16*math.sin(angle),145),(.36,.2,.07),'dead','Sphere',unreal.Rotator(0,j*72,0),False,'VZRepairFlower')
sun=actors.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,1200),unreal.Rotator(-45,-35,0));sun.light_component.set_editor_property('intensity',5);sun.light_component.set_editor_property('atmosphere_sun_light',True)
actors.spawn_actor_from_class(unreal.SkyAtmosphere,unreal.Vector())
sky=actors.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,600));sky.light_component.set_editor_property('real_time_capture',True);sky.light_component.set_editor_property('intensity',1)
post=actors.spawn_actor_from_class(unreal.PostProcessVolume,unreal.Vector());post.set_editor_property('unbound',True)
settings=post.get_editor_property('settings');settings.set_editor_property('override_auto_exposure_min_brightness',True);settings.set_editor_property('override_auto_exposure_max_brightness',True);settings.set_editor_property('auto_exposure_min_brightness',0);settings.set_editor_property('auto_exposure_max_brightness',0);post.set_editor_property('settings',settings)
actors.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(0,0,120))
if not unreal.VZEditorTools.add_navigation_bounds(world):raise RuntimeError('Navigation volume creation failed')
if not sub.save_current_level():raise RuntimeError('Map save failed')
report={'map':PATH,'static_mesh_actors':count,'status':'authored P0 environment candidate, modular primitives with original materials; production visual approval pending'}
(ROOT/'Docs/conservatory-build.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');unreal.log('VZ_CONSERVATORY_CREATED '+str(report))
