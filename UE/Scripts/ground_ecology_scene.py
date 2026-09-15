import unreal,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE')
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);assert levels.load_level('/Game/VariantZero/Maps/P0_Conservatory_Ecology')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
terrain=next(a for a in actors.get_all_level_actors() if a.get_actor_label()=='Institute terrain - visual boundary')
comp=terrain.static_mesh_component;mesh=comp.static_mesh
body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
comp.set_collision_profile_name('BlockAll');comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
report=[]
for a in actors.get_all_level_actors():
    label=a.get_actor_label()
    if label not in ['CC0 weathered boundary rock','CC0 exterior fern','CC0 cultivation fern']:continue
    a.set_actor_rotation(unreal.Rotator(pitch=0,yaw=len(report)*137.5,roll=0),False)
    loc=a.get_actor_location();origin,extent=a.get_actor_bounds(False)
    if label=='CC0 cultivation fern':z=86
    else:
        heights=[]
        offsets=[(0,0)] if 'fern' in label else [(x,y) for x in [-.8,0,.8] for y in [-.8,0,.8]]
        for dx,dy in offsets:
            x=origin.x+extent.x*dx;y=origin.y+extent.y*dy
            hit=unreal.SystemLibrary.line_trace_single(world,unreal.Vector(x,y,10000),unreal.Vector(x,y,-3000),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[],unreal.DrawDebugTrace.NONE,True)
            info=(hit[0] if isinstance(hit,tuple) else hit).to_tuple()
            if not info[0]:raise RuntimeError('Terrain trace missed '+label)
            heights.append(info[5].z)
        z=min(heights)
    # Use actual transformed bounds, including imported pivot offsets and slope footprint.
    loc.z+=z-(origin.z-extent.z)-(extent.z*.2 if 'rock' in label else 2)
    a.set_actor_location(loc,False,False);report.append({'actor':label,'z':loc.z})
comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
assert levels.save_current_level()
(ROOT/'Docs/ecology-grounding.json').write_text(json.dumps(report,indent=2),encoding='utf8')
unreal.log('VZ_GROUNDING PASS '+str(len(report)))
