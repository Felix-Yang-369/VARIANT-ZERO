#include "../VZPrologue.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZPrologueTest,"VariantZero.Story.PrologueTransactions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZPrologueTest::RunTest(const FString& Parameters)
{
    auto S=FVZSaveStore::NewGame();const auto Parent=S.Companions[0].IndividualId;
    TestFalse(TEXT("cannot skip to reward"),FVZPrologueRules::Apply(S,TEXT("repair")));
    TestTrue(TEXT("accept briefing"),FVZPrologueRules::Apply(S,TEXT("briefing")));
    TestFalse(TEXT("cannot collect before scan"),FVZPrologueRules::Apply(S,TEXT("sample.0")));
    TestTrue(TEXT("scan"),FVZPrologueRules::Apply(S,TEXT("scan")));
    TestFalse(TEXT("reject unknown sample"),FVZPrologueRules::Apply(S,TEXT("sample.8")));
    for(const int32 I:{2,0,1})
    {
        auto E=FString::Printf(TEXT("sample.%d"),I);
        TestTrue(TEXT("collect in arbitrary order"),FVZPrologueRules::Apply(S,E));
        TestFalse(TEXT("collected sample cannot be farmed"),FVZPrologueRules::Apply(S,E));
    }
    const FString Path=FPaths::ProjectSavedDir()/TEXT("Automation/Prologue")/FGuid::NewGuid().ToString()/TEXT("state.vzsave");
    TestTrue(TEXT("save mid quest"),FVZSaveStore::Write(Path,S));
    FVZWorldState Loaded;TestTrue(TEXT("resume"),FVZSaveStore::Read(Path,Loaded)==EVZReadResult::Ok);
    auto Empty=Loaded;Empty.Materials.Add(TEXT("prologue.viable_sample"),0);
    TestFalse(TEXT("cannot cultivate without cost"),FVZPrologueRules::Apply(Empty,TEXT("cultivate")));
    TestEqual(TEXT("failed nurture leaves party untouched"),Empty.Companions.Num(),1);
    TestTrue(TEXT("deterministic cultivation"),FVZPrologueRules::Apply(Loaded,TEXT("cultivate")));
    TestEqual(TEXT("spends all three"),Loaded.Materials.FindRef(TEXT("prologue.viable_sample")),0);
    TestEqual(TEXT("retains parent"),Loaded.Companions[0].IndividualId,Parent);
    TestEqual(TEXT("new individual"),Loaded.Companions.Num(),2);
    TestTrue(TEXT("distinct identity"),Loaded.Companions[1].IndividualId!=Parent);
    TestFalse(TEXT("nurture not duplicated"),FVZPrologueRules::Apply(Loaded,TEXT("cultivate")));
    TestFalse(TEXT("guardian bypass blocked"),FVZPrologueRules::Apply(Loaded,TEXT("repair")));
    TestTrue(TEXT("practice"),FVZPrologueRules::Apply(Loaded,TEXT("practice")));
    TestTrue(TEXT("repair"),FVZPrologueRules::Apply(Loaded,TEXT("repair")));
    TestTrue(TEXT("persist reward"),FVZSaveStore::Write(Path,Loaded));
    TestTrue(TEXT("reload reward"),FVZSaveStore::Read(Path,S)==EVZReadResult::Ok);
    TestFalse(TEXT("no repeat payout after reload"),FVZPrologueRules::Apply(S,TEXT("repair")));
    TestEqual(TEXT("exact research reward"),S.Materials.FindRef(TEXT("research.sample")),2);
    TestTrue(TEXT("debrief"),FVZPrologueRules::Apply(S,TEXT("debrief")));
    TestEqual(TEXT("complete"),FVZPrologueRules::Stage(S),7);
    TestTrue(TEXT("journal restores observations"),FVZPrologueRules::Journal(S).Contains(TEXT("来自花庭")));
    TestFalse(TEXT("no pollination without species"),FVZPrologueRules::Apply(S,TEXT("pollinate")));
    TestTrue(TEXT("moth research"),FVZPrologueRules::Apply(S,TEXT("research_moth")));
    TestEqual(TEXT("three independent party members"),S.Party.Num(),3);
    TestEqual(TEXT("research cost"),S.Materials.FindRef(TEXT("research.sample")),0);
    TestFalse(TEXT("no duplicate moth"),FVZPrologueRules::Apply(S,TEXT("research_moth")));
    TestTrue(TEXT("pollinate with moth"),FVZPrologueRules::Apply(S,TEXT("pollinate")));
    TestTrue(TEXT("save pollination"),FVZSaveStore::Write(Path,S));
    TestTrue(TEXT("restore pollination"),FVZSaveStore::Read(Path,Loaded)==EVZReadResult::Ok);
    TestFalse(TEXT("no repeated pollination"),FVZPrologueRules::Apply(Loaded,TEXT("pollinate")));
    TestTrue(TEXT("individual memory retained"),Loaded.Companions.Last().Memories.Contains(TEXT("prologue.first_pollination")));
    return true;
}
#endif
