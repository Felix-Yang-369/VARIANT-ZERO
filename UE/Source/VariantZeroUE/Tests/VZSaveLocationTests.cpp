#include "../VZSaveLocation.h"
#include "../VZState.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZSaveLocationTest,"VariantZero.Save.SharedLocation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZSaveLocationTest::RunTest(const FString&)
{
    const FString Root=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Automation/SharedSave")/FGuid::NewGuid().ToString());
    const FString From=Root/TEXT("old/Saved/SaveGames/ProloguePrototype"),To=FVZSaveLocation::StoryDirectory(Root/TEXT("user"));
    TestFalse(TEXT("user root absolute"),FPaths::IsRelative(FVZSaveLocation::UserRoot()));
    TestTrue(TEXT("missing source creates no destination"),FVZSaveLocation::ImportLegacy(From,To)==EVZMigration::NoSource&&!IFileManager::Get().DirectoryExists(*To));
    auto S=FVZSaveStore::NewGame();S.PlayerName=TEXT("跨版本生态师");S.Companions[0].Level=3;S.QuestStages.Add(TEXT("main.garden"),3);S.QuestStages.Add(TEXT("growth.spent"),3);
    TestTrue(TEXT("source save"),FVZSaveStore::Write(From/TEXT("slot_0.vzsave"),S));
    TestTrue(TEXT("source manual save"),FVZSaveStore::Write(From/TEXT("slot_2.vzsave"),S));
    S.PlayerName=TEXT("最近版本");FVZSaveStore::Write(From/TEXT("slot_0.vzsave"),S);
    TestTrue(TEXT("import whole group"),FVZSaveLocation::ImportLegacy(From,To)==EVZMigration::Imported);
    FVZWorldState Read;
    TestTrue(TEXT("read migrated progression"),FVZSaveStore::Read(To/TEXT("slot_0.vzsave"),Read)==EVZReadResult::Ok&&Read.PlayerName==S.PlayerName&&Read.Companions[0].Level==3&&Read.QuestStages.FindRef(TEXT("growth.spent"))==3);
    TestTrue(TEXT("manual and backup retained"),IFileManager::Get().FileExists(*(To/TEXT("slot_2.vzsave")))&&IFileManager::Get().FileExists(*(To/TEXT("slot_0.vzsave.bak"))));
    TArray<uint8> A,B;FFileHelper::LoadFileToArray(A,*(From/TEXT("slot_0.vzsave")));FFileHelper::LoadFileToArray(B,*(To/TEXT("slot_0.vzsave")));
    TestTrue(TEXT("source retained and byte-identical"),A==B&&!A.IsEmpty());
    S.PlayerName=TEXT("旧版另一次游戏");FVZSaveStore::Write(From/TEXT("slot_0.vzsave"),S);
    TestTrue(TEXT("existing target never replaced"),FVZSaveLocation::ImportLegacy(From,To)==EVZMigration::AlreadyPresent);
    FVZSaveStore::Read(To/TEXT("slot_0.vzsave"),Read);TestTrue(TEXT("newer target intact"),Read.PlayerName!=S.PlayerName);
    FFileHelper::SaveStringToFile(TEXT("broken"),*(From/TEXT("slot_2.vzsave")));
    const FString Bad=Root/TEXT("failed");
    TestTrue(TEXT("one bad slot prevents partial migration"),FVZSaveLocation::ImportLegacy(From,Bad)==EVZMigration::Failed&&!IFileManager::Get().DirectoryExists(*Bad));
    FVZSaveStore::Write(From/TEXT("slot_2.vzsave.bak"),S);
    const FString Recovered=Root/TEXT("recovered");
    TestTrue(TEXT("valid backup recovers corrupt slot"),FVZSaveLocation::ImportLegacy(From,Recovered)==EVZMigration::Imported);
    FFileHelper::SaveStringToFile(TEXT("{\"magic\":\"VariantZero\",\"version\":99}"),*(From/TEXT("slot_2.vzsave")));
    TestTrue(TEXT("future version does not fall back"),FVZSaveLocation::ImportLegacy(From,Root/TEXT("future"))==EVZMigration::Failed);
    return true;
}
#endif
