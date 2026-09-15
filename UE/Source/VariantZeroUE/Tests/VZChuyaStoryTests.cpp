#include "../VZChuyaStory.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZChuyaStoryTest,"VariantZero.Story.ChuyaFirstRain",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZChuyaStoryTest::RunTest(const FString&)
{
    auto S=FVZSaveStore::NewGame();const auto Original=S.Companions[0].IndividualId;
    auto Legacy=S;Legacy.Companions[0].Memories.Reset();Legacy.QuestStages.Add(TEXT("main.garden"),3);
    TestTrue(TEXT("legacy original is pinned at first acceptance"),FVZChuyaStoryRules::Apply(Legacy,TEXT("begin")) && FVZChuyaStoryRules::Protagonist(Legacy)==Original);
    TestFalse(TEXT("locked before garden report"),FVZChuyaStoryRules::Apply(S,TEXT("begin")));
    S.QuestStages.Add(TEXT("main.garden"),3);S.Companions[0].Name=TEXT("改过名字的初芽");
    FVZCompanion Other;Other.IndividualId=FGuid::NewGuid();Other.SpeciesId=TEXT("V-001");Other.Name=TEXT("新芽");S.Companions.Add(Other);S.Party={Other.IndividualId};
    TestFalse(TEXT("same species cannot substitute protagonist"),FVZChuyaStoryRules::Apply(S,TEXT("begin")));S.Party.Add(Original);
    TestTrue(TEXT("begin with renamed original"),FVZChuyaStoryRules::Apply(S,TEXT("begin")));S.Companions.Swap(0,1);
    TestTrue(TEXT("identity survives reordering"),FVZChuyaStoryRules::Protagonist(S)==Original);
    TestFalse(TEXT("early choice rejected"),FVZChuyaStoryRules::Apply(S,TEXT("choose.dry")));
    TestFalse(TEXT("unknown observation rejected"),FVZChuyaStoryRules::Apply(S,TEXT("observe.9")));
    TestTrue(TEXT("observe"),FVZChuyaStoryRules::Apply(S,TEXT("observe.0")));
    TestFalse(TEXT("observation cannot be counted twice"),FVZChuyaStoryRules::Apply(S,TEXT("observe.0")));
    const auto Path=FPaths::ProjectSavedDir()/TEXT("Automation/ChuyaStory")/FGuid::NewGuid().ToString()/TEXT("test.vzsave");
    TestTrue(TEXT("save partial story"),FVZSaveStore::Write(Path,S));FVZWorldState R;
    TestTrue(TEXT("restore partial story"),FVZSaveStore::Read(Path,R)==EVZReadResult::Ok);
    TestTrue(TEXT("remaining observations"),FVZChuyaStoryRules::Apply(R,TEXT("observe.2")) && FVZChuyaStoryRules::Apply(R,TEXT("observe.1")));
    TestEqual(TEXT("choice available"),FVZChuyaStoryRules::Stage(R),2);
    auto Wet=R,Dry=R;TestTrue(TEXT("shallow choice"),FVZChuyaStoryRules::Apply(Wet,TEXT("choose.shallow")));TestTrue(TEXT("dry choice"),FVZChuyaStoryRules::Apply(Dry,TEXT("choose.dry")));
    TestFalse(TEXT("cannot switch completed choice"),FVZChuyaStoryRules::Apply(Wet,TEXT("choose.dry")));TestFalse(TEXT("cannot farm reward"),FVZChuyaStoryRules::Apply(Wet,TEXT("choose.shallow")));
    TestEqual(TEXT("equal reward"),Wet.Materials.FindRef(TEXT("research.sample")),Dry.Materials.FindRef(TEXT("research.sample")));TestEqual(TEXT("one sample"),Wet.Materials.FindRef(TEXT("research.sample")),1);
    TestTrue(TEXT("only original receives story memory"),Wet.Companions[1].Memories.Contains(TEXT("story.chuya.first_rain")) && !Wet.Companions[0].Memories.Contains(TEXT("story.chuya.first_rain")));
    TestTrue(TEXT("different persisted endings"),FVZChuyaStoryRules::Journal(Wet)!=FVZChuyaStoryRules::Journal(Dry));
    return true;
}
#endif
