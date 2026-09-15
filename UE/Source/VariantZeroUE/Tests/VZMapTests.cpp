#include "../VZMap.h"
#include "../VZPrologue.h"
#include "../VZTravel.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZMapTest,"VariantZero.World.ChapterMap",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZMapTest::RunTest(const FString&)
{
    auto S=FVZSaveStore::NewGame();auto Points=FVZMapRules::Points(S);FVector Position;
    TestTrue(TEXT("new game has institute and current objective"),Points.Num()>=2);
    TestFalse(TEXT("undiscovered garden travel point hidden"),Points.ContainsByPredicate([](const FVZMapPoint& P){return P.TravelId==TEXT("garden");}));
    TestFalse(TEXT("no unexplored region objective"),Points.ContainsByPredicate([](const FVZMapPoint& P){return P.Position.Y>1800;}));
    const auto Before=Points.Last().Position;
    TestTrue(TEXT("briefing advances objective"),FVZPrologueRules::Apply(S,TEXT("briefing")));
    Points=FVZMapRules::Points(S);TestFalse(TEXT("map follows actual task progression"),Points.Last().Position.Equals(Before));
    S.QuestStages.Add(TEXT("main.prologue"),7);S.QuestStages.Add(TEXT("main.garden"),1);S.Repairs.Add(TEXT("prologue.pollinated"));
    Points=FVZMapRules::Points(S);
    TestTrue(TEXT("rest station remains discoverable after prologue"),Points.ContainsByPredicate([](const FVZMapPoint& P){return P.Label==TEXT("培养台 · 伙伴整备")&&P.TravelId.IsEmpty();}));
    TestTrue(TEXT("discovered garden travel available"),Points.ContainsByPredicate([](const FVZMapPoint& P){return P.TravelId==TEXT("garden");}));
    for(const auto& P:Points)
    {
        if(!P.TravelId.IsEmpty()){TestTrue(TEXT("map travel matches transaction rules"),FVZTravelRules::Destination(S,P.TravelId,Position));TestTrue(TEXT("map coordinate matches checkpoint"),P.Position.Equals(Position));}
        const auto XY=FVZMapRules::Project(P.Position);TestTrue(TEXT("known points inside drawing area"),XY.X>40&&XY.X<690&&XY.Y>115&&XY.Y<735);
    }
    TestEqual(TEXT("north draws upward"),FVZMapRules::Project(FVector(0,1000,0)).Y,FVZMapRules::Project(FVector::ZeroVector).Y-70);
    TestEqual(TEXT("uniform map scale"),FVZMapRules::Project(FVector(1000,0,0)).X,FVZMapRules::Project(FVector::ZeroVector).X+70);
    const auto Receipts=S.RewardTransactions;FVZMapRules::Points(S);TestTrue(TEXT("map does not grant rewards"),S.RewardTransactions.Num()==Receipts.Num());
    S.Companions.Empty();TestTrue(TEXT("invalid state yields no teleport targets"),FVZMapRules::Points(S).IsEmpty());
    return true;
}
#endif
