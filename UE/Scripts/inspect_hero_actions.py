import unreal,json
from pathlib import Path
result=[]
for name in ['Primary_Fire_Fast','Cast','HitReact_Front','E_Ability_Start','Q_Ability_Start']:
    a=unreal.load_asset('/Game/ParagonLtBelica/Characters/Heroes/Belica/Animations/'+name)
    result.append(dict(name=name,length=a.get_play_length(),additive=str(a.get_editor_property('additive_anim_type')),skeleton=a.get_editor_property('skeleton').get_path_name()))
Path(r'D:\Projects\VariantZeroUE\Docs\hero-action-source.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print('VZ_ACTION_INSPECT: PASS '+json.dumps(result))
