import unreal
mat=unreal.load_asset('/Game/VariantZero/Ecology/M_forest_ground_04')
lib=unreal.MaterialEditingLibrary
# Rebuild only this project's derived material; CC0 source textures remain intact.
lib.delete_all_material_expressions(mat)
base=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);base.texture=unreal.load_asset('/Game/VariantZero/Ecology/T_forest_ground_04_Diffuse');base.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
tint=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);tint.constant=unreal.LinearColor(.34,.48,.25,1)
mul=lib.create_material_expression(mat,unreal.MaterialExpressionMultiply);lib.connect_material_expressions(base,'RGB',mul,'A');lib.connect_material_expressions(tint,'',mul,'B');lib.connect_material_property(mul,'',unreal.MaterialProperty.MP_BASE_COLOR)
normal=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);normal.texture=unreal.load_asset('/Game/VariantZero/Ecology/T_forest_ground_04_nor_dx');normal.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL;lib.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.r=.94;lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
