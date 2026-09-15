#include "../VZTravel.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZTravelTest,"VariantZero.World.TravelAndRecords",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZTravelTest::RunTest(const FString&)
{
    auto S=FVZSaveStore::NewGame();const auto Id=S.Companions[0].IndividualId;FVector Point;
    TestFalse(TEXT("unknown destination"),FVZTravelRules::Destination(S,TEXT("core"),Point));
    TestFalse(TEXT("undiscovered garden locked"),FVZTravelRules::Apply(S,TEXT("garden")));
    TestTrue(TEXT("institute available"),FVZTravelRules::Apply(S,TEXT("institute")));
    TestFalse(TEXT("no future garden outcome in journal"),FVZTravelRules::Record(S,1).Contains(TEXT("供水已经恢复")));
    S.QuestStages.Add(TEXT("main.garden"),1);S.Materials.Add(TEXT("research.sample"),4);
    TestTrue(TEXT("discovered garden available"),FVZTravelRules::Apply(S,TEXT("garden")));
    const auto Path=FPaths::ProjectSavedDir()/TEXT("Automation/Travel")/FGuid::NewGuid().ToString()/TEXT("travel.vzsave");
    TestTrue(TEXT("save destination"),FVZSaveStore::Write(Path,S));FVZWorldState R;
    TestTrue(TEXT("read destination"),FVZSaveStore::Read(Path,R)==EVZReadResult::Ok);
    TestTrue(TEXT("checkpoint persisted"),R.Checkpoint.Equals(FVector(0,2200,120)));
    for(int32 I=0;I<10;++I){TestTrue(TEXT("return"),FVZTravelRules::Apply(R,TEXT("institute")));TestTrue(TEXT("revisit"),FVZTravelRules::Apply(R,TEXT("garden")));}
    TestEqual(TEXT("materials unchanged by travel"),R.Materials.FindRef(TEXT("research.sample")),4);
    TestEqual(TEXT("identity preserved"),R.Companions[0].IndividualId,Id);
    TestEqual(TEXT("quest preserved"),R.QuestStages.FindRef(TEXT("main.garden")),1);
    TestTrue(TEXT("travel has no reward receipt"),R.RewardTransactions.IsEmpty());
    R.Companions[0].Memories.Add(TEXT("garden.first_rain"));TestTrue(TEXT("earned memory shown"),FVZTravelRules::Record(R,2).Contains(TEXT("共同见证")));
    return true;
}
#endif
