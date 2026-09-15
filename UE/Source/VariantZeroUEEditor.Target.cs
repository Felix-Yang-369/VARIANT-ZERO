using UnrealBuildTool;
public class VariantZeroUEEditorTarget : TargetRules
{
    public VariantZeroUEEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("VariantZeroUE");
    }
}
