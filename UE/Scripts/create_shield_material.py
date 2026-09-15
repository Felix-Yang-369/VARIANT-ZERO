import unreal
from pathlib import Path
path='/Game/VariantZero/Ecology/M_EcologicalShield'
m=unreal.load_asset(path)
if not m:
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_EcologicalShield','/Game/VariantZero/Ecology',unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    lib=unreal.MaterialEditingLibrary
    edge=lib.create_material_expression(m,unreal.MaterialExpressionFresnel,-500,0)
    edge.set_editor_property('exponent',3.0);edge.set_editor_property('base_reflect_fraction',0.025)
    color=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector,-500,180)
    color.set_editor_property('constant',unreal.LinearColor(.12,.85,.54,1))
    glow=lib.create_material_expression(m,unreal.MaterialExpressionMultiply,-200,100)
    lib.connect_material_expressions(edge,'',glow,'A');lib.connect_material_expressions(color,'',glow,'B')
    opacity=lib.create_material_expression(m,unreal.MaterialExpressionMultiply,-200,-100)
    opacity.set_editor_property('const_b',.48);lib.connect_material_expressions(edge,'',opacity,'A')
    lib.connect_material_property(glow,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
    lib.recompile_material(m)
    assert unreal.EditorAssetLibrary.save_loaded_asset(m)
assert m.get_editor_property('blend_mode')==unreal.BlendMode.BLEND_TRANSLUCENT
Path(r'D:\Projects\VariantZeroUE\Docs\shield-material.txt').write_text('Original procedural Fresnel shield. No purchased assets. '+m.get_path_name(),encoding='utf-8')
print('VZ_SHIELD_MATERIAL: PASS')
