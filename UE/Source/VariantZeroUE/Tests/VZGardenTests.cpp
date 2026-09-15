#include "../VZGarden.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZGardenTest,"VariantZero.Story.GardenState",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZGardenTest::RunTest(const FString&)
{
    auto S=FVZSaveStore::NewGame();TestFalse(TEXT("garden locked before pollination"),FVZGardenRules::Apply(S,TEXT("enter")));
    S.Repairs.Add(TEXT("prologue.pollinated"));FVZCompanion M;M.IndividualId=FGuid::NewGuid();M.SpeciesId=TEXT("V-041");M.Name=TEXT("菌翼蛾");S.Companions.Add(M);S.Party.Add(M.IndividualId);
    TestTrue(TEXT("enter"),FVZGardenRules::Apply(S,TEXT("enter")));TestFalse(TEXT("no reward without puzzle"),FVZGardenRules::Apply(S,TEXT("repair")));
    TestFalse(TEXT("unknown mirror rejected"),FVZGardenRules::Apply(S,TEXT("mirror.4")));
    for(int32 I=0;I<3;++I)for(int32 N=0;N<(I==0?1:I==1?3:2);++N)TestTrue(TEXT("rotate"),FVZGardenRules::Apply(S,FString::Printf(TEXT("mirror.%d"),I)));
    TestTrue(TEXT("three independent alignments"),FVZGardenRules::Aligned(S));
    const auto Path=FPaths::ProjectSavedDir()/TEXT("Automation/Garden")/FGuid::NewGuid().ToString()/TEXT("test.vzsave");TestTrue(TEXT("save puzzle"),FVZSaveStore::Write(Path,S));
    FVZWorldState R;TestTrue(TEXT("restore puzzle"),FVZSaveStore::Read(Path,R)==EVZReadResult::Ok);TestTrue(TEXT("alignment restored"),FVZGardenRules::Aligned(R));
    TestTrue(TEXT("repair transaction"),FVZGardenRules::Apply(R,TEXT("repair")));TestFalse(TEXT("reward once"),FVZGardenRules::Apply(R,TEXT("repair")));
    TestEqual(TEXT("four samples"),R.Materials.FindRef(TEXT("research.sample")),4);TestTrue(TEXT("shortcut"),R.Repairs.Contains(TEXT("garden.shortcut")));
    TestFalse(TEXT("completed puzzle cannot be broken"),FVZGardenRules::Apply(R,TEXT("mirror.0")));TestTrue(TEXT("report"),FVZGardenRules::Apply(R,TEXT("report")));return true;
}
#endif
