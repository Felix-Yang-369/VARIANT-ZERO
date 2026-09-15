import unreal,math,json,random
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');PATH='/Game/VariantZero/Maps/P0_Conservatory_Ecology'
source=(ROOT/'Scripts/create_conservatory.py').read_text(encoding='utf8')
source=source.replace("PATH='/Game/VariantZero/Maps/P0_Conservatory'","PATH='/Game/VariantZero/Maps/P0_Conservatory_Ecology'").replace('conservatory-build.json','ecology-scene-base.json')
exec(compile(source,'ecology_base','exec'))
eco='/Game/VariantZero/Ecology';random.seed(104)
def place(name,asset,loc,scale=(1,1,1),yaw=0):
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc),unreal.Rotator(pitch=0,yaw=yaw,roll=0));a.set_actor_label(name)
    a.static_mesh_component.set_static_mesh(unreal.load_asset(asset));a.set_actor_scale3d(unreal.Vector(*scale))
    a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    a.static_mesh_component.set_editor_property('can_ever_affect_navigation',False)
    return a
# Replace only the prototype vegetation in this newly authored map.
for a in list(actors.get_all_level_actors()):
    label=a.get_actor_label()
    if label in ['Plant stem','Broad foliage']:actors.destroy_actor(a)
    if label=='Mineral foundation':a.static_mesh_component.set_material(0,unreal.load_asset(eco+'/M_rock_01'))
fern_count=0
for cx,cy in [(-950,-650),(-950,650),(950,-650),(950,650)]:
    for i in range(7):
        t=i*2.4;r=0 if i==0 else 105
        place('CC0 cultivation fern',eco+'/SM_fern_02',(cx+r*math.cos(t),cy+r*math.sin(t),86),(.65,.65,1.6),i*51);fern_count+=1
place('Institute terrain - visual boundary',eco+'/SM_InstituteTerrain',(0,0,0))
def ground(x,y):
    x/=100;y/=100;r=math.hypot(x,y);b=min(1,max(0,(r-22)/24))
    return ((5+4*math.sin(x*.08+y*.03)+3*math.cos(y*.11-x*.025)+1.3*math.sin(x*.23)*math.cos(y*.17))*b-.65)*100
rock_count=0
for i in range(44):
    t=i*math.tau/44;r=random.uniform(2900,7800);x=r*math.cos(t);y=r*math.sin(t)
    s=random.uniform(3,7)
    place('CC0 weathered boundary rock',eco+'/SM_boulder_01',(x,y,ground(x,y)-30),(s,s*.8,s*.75),random.uniform(0,360));rock_count+=1
for i in range(72):
    t=random.uniform(0,math.tau);r=random.uniform(1850,3300);x=r*math.cos(t);y=r*math.sin(t)
    place('CC0 exterior fern',eco+'/SM_fern_02',(x,y,ground(x,y)),(1.2,1.2,2.2),random.uniform(0,360));fern_count+=1
# A restrained pane material lets the real exterior silhouettes read through the greenhouse.
glass=tools.create_asset('M_ConservatoryGlass',eco,unreal.Material,unreal.MaterialFactoryNew())
glass.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);glass.set_editor_property('two_sided',True)
for value,prop in [(.12,unreal.MaterialProperty.MP_OPACITY),(.18,unreal.MaterialProperty.MP_ROUGHNESS)]:
    node=lib.create_material_expression(glass,unreal.MaterialExpressionConstant);node.r=value;lib.connect_material_property(node,'',prop)
node=lib.create_material_expression(glass,unreal.MaterialExpressionConstant3Vector);node.constant=unreal.LinearColor(.1,.24,.18,1);lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
lib.recompile_material(glass);unreal.EditorAssetLibrary.save_loaded_asset(glass)
for a in actors.get_all_level_actors():
    if a.get_actor_label()=='Jade wall panel':a.static_mesh_component.set_material(0,glass)
fog=actors.spawn_actor_from_class(unreal.ExponentialHeightFog,unreal.Vector(0,0,-150))
fc=fog.get_component_by_class(unreal.ExponentialHeightFogComponent);fc.set_editor_property('fog_density',.008);fc.set_editor_property('fog_height_falloff',.15)
assert sub.save_current_level()
report={'map':PATH,'fern_actors':fern_count,'rock_actors':rock_count,'terrain':'250m visual backdrop only; not open-world traversable area','nav':'Original obstruction and bounds preserved; backdrop does not affect navigation'}
(ROOT/'Docs/ecology-scene.json').write_text(json.dumps(report,indent=2),encoding='utf8')
unreal.log('VZ_ECOLOGY_SCENE PASS')
