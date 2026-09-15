#include "../VZRoster.h"
#include "../VZChuyaStory.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZRosterTest,"VariantZero.Companions.Roster",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZRosterTest::RunTest(const FString&)
{
    auto S=FVZSaveStore::NewGame();const auto Id=S.Companions[0].IndividualId;
    auto Growth=S;
    TestFalse(TEXT("no training before milestone"),FVZRosterRules::Train(Growth,Id,1));
    Growth.QuestStages.Add(TEXT("main.prologue"),6);Growth.QuestStages.Add(TEXT("main.garden"),3);
    TestEqual(TEXT("chapter growth entitlement"),FVZRosterRules::GrowthPoints(Growth),8);
    const auto Materials=Growth.Materials;
    TestTrue(TEXT("first growth"),FVZRosterRules::Train(Growth,Id,1));
    TestEqual(TEXT("growth level"),Growth.Companions[0].Level,2);
    TestEqual(TEXT("cost charged once"),FVZRosterRules::GrowthPoints(Growth),7);
    TestFalse(TEXT("replayed stale training rejected"),FVZRosterRules::Train(Growth,Id,1));
    TestTrue(TEXT("story materials and identity preserved"),Growth.Materials.OrderIndependentCompareEqual(Materials)&&Growth.Companions[0].IndividualId==Id&&Growth.Companions[0].Memories==S.Companions[0].Memories);
    TestTrue(TEXT("second growth"),FVZRosterRules::Train(Growth,Id,2));
    TestTrue(TEXT("third growth"),FVZRosterRules::Train(Growth,Id,3));
    TestFalse(TEXT("insufficient points no partial upgrade"),FVZRosterRules::Train(Growth,Id,4));
    TestEqual(TEXT("remaining points"),FVZRosterRules::GrowthPoints(Growth),2);
    TestEqual(TEXT("level two health"),FVZRosterRules::MaxHealth(2),105.f);
    TestEqual(TEXT("level two skill"),FVZRosterRules::SkillDamage(2),34.f);
    TestEqual(TEXT("level two healing"),FVZRosterRules::Healing(2),21.f);
    Growth.Companions[0].Level=20;TestFalse(TEXT("level cap"),FVZRosterRules::Train(Growth,Id,20));
    auto Other=S.Companions[0];Other.IndividualId=FGuid::NewGuid();Other.Memories.Reset();S.Companions.Add(Other);S.Party.Add(Other.IndividualId);
    TestFalse(TEXT("empty rejected"),FVZRosterRules::Rename(S,Id,TEXT("  ")));
    TestFalse(TEXT("newline rejected"),FVZRosterRules::Rename(S,Id,TEXT("小\n芽")));
    TestFalse(TEXT("long rejected"),FVZRosterRules::Rename(S,Id,FString::ChrN(17,'a')));
    TestFalse(TEXT("unknown identity rejected"),FVZRosterRules::Rename(S,FGuid::NewGuid(),TEXT("小芽")));
    TestTrue(TEXT("rename selected individual"),FVZRosterRules::Rename(S,Id,TEXT("  雨芽  ")));
    TestEqual(TEXT("trimmed name"),S.Companions[0].Name,FString(TEXT("雨芽")));
    TestEqual(TEXT("other individual untouched"),S.Companions[1].Name,Other.Name);
    TestFalse(TEXT("cannot leave first boundary"),FVZRosterRules::Move(S,Id,-1));
    TestTrue(TEXT("move to second"),FVZRosterRules::Move(S,Id,1));
    TestTrue(TEXT("story identity unchanged"),FVZChuyaStoryRules::Protagonist(S)==Id);
    TestFalse(TEXT("invalid direction"),FVZRosterRules::Move(S,Id,3));
    const auto Memories=S.Companions[0].Memories;
    TestTrue(TEXT("return original to institute"),FVZRosterRules::SetDeployed(S,Id,false));
    TestFalse(TEXT("keep at least one companion"),FVZRosterRules::SetDeployed(S,Other.IndividualId,false));
    TestFalse(TEXT("unknown cannot join"),FVZRosterRules::SetDeployed(S,FGuid::NewGuid(),true));
    TestTrue(TEXT("rejoin original"),FVZRosterRules::SetDeployed(S,Id,true));
    TestFalse(TEXT("duplicate join rejected"),FVZRosterRules::SetDeployed(S,Id,true));
    auto Moth=Other;Moth.IndividualId=FGuid::NewGuid();Moth.SpeciesId=TEXT("V-041");S.Companions.Add(Moth);
    TestTrue(TEXT("third slot available"),FVZRosterRules::SetDeployed(S,Moth.IndividualId,true));
    auto Fourth=Other;Fourth.IndividualId=FGuid::NewGuid();S.Companions.Add(Fourth);
    TestFalse(TEXT("fourth slot rejected"),FVZRosterRules::SetDeployed(S,Fourth.IndividualId,true));
    TestTrue(TEXT("moth may rest"),FVZRosterRules::SetDeployed(S,Moth.IndividualId,false));
    TestTrue(TEXT("quest companion recoverable"),FVZRosterRules::SetDeployed(S,Moth.IndividualId,true));
    TestTrue(TEXT("story identity and memories survive roster changes"),FVZChuyaStoryRules::Protagonist(S)==Id&&S.Companions[0].Memories==Memories);
    const auto Path=FPaths::ProjectSavedDir()/TEXT("Automation/Roster")/FGuid::NewGuid().ToString()/TEXT("slot.vzsave");
    TestTrue(TEXT("save edited roster"),FVZSaveStore::Write(Path,S));FVZWorldState R;
    TestTrue(TEXT("read identity and order"),FVZSaveStore::Read(Path,R)==EVZReadResult::Ok&&R.Party[1]==Id&&R.Companions[0].Name==TEXT("雨芽")&&R.Companions[0].Memories==S.Companions[0].Memories);
    TestTrue(TEXT("reserve individual persists outside active party"),R.Companions.Num()==4&&R.Party.Num()==3&&!R.Party.Contains(Fourth.IndividualId));
    return true;
}
#endif
