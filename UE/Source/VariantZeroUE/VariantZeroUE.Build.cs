using UnrealBuildTool;
public class VariantZeroUE : ModuleRules
{
    public VariantZeroUE(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Json", "JsonUtilities", "AIModule", "NavigationSystem", "UMG", "Slate", "SlateCore" });
        if (Target.bBuildEditor) PrivateDependencyModuleNames.Add("UnrealEd");
    }
}
