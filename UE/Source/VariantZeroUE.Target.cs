using UnrealBuildTool;
public class VariantZeroUETarget : TargetRules
{
    public VariantZeroUETarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("VariantZeroUE");
    }
}
