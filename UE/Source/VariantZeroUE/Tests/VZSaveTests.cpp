#include "../VZState.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZSaveRecoveryTest, "VariantZero.Save.RecoveryAndProtection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVZSaveRecoveryTest::RunTest(const FString& Parameters)
{
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("Automation/VZ") / FGuid::NewGuid().ToString();
    const FString Path = Directory / TEXT("test.vzsave");
    auto Initial = FVZSaveStore::NewGame();
    TestTrue(TEXT("new state valid"), FVZSaveStore::Validate(Initial));
    TestTrue(TEXT("write first generation"), FVZSaveStore::Write(Path, Initial));
    auto Next = Initial; Next.Materials.Add(TEXT("research.sample"), 3); Next.RewardTransactions.Add(TEXT("test.reward"));
    TestTrue(TEXT("write second generation"), FVZSaveStore::Write(Path, Next));
    FVZWorldState Loaded;
    TestTrue(TEXT("read committed state"), FVZSaveStore::Read(Path, Loaded) == EVZReadResult::Ok);
    TestEqual(TEXT("reward amount persisted"), Loaded.Materials.FindRef(TEXT("research.sample")), 3);
    TestTrue(TEXT("individual identity stable"), Loaded.Companions[0].IndividualId == Initial.Companions[0].IndividualId);
    FFileHelper::SaveStringToFile(TEXT("{truncated"), *Path);
    TestTrue(TEXT("recover truncated primary"), FVZSaveStore::Read(Path, Loaded) == EVZReadResult::Recovered);
    TestEqual(TEXT("backup is previous generation"), Loaded.Materials.FindRef(TEXT("research.sample")), 0);
    TestTrue(TEXT("commit recovered state"), FVZSaveStore::Write(Path, Loaded));
    FString Future = TEXT("{\"magic\":\"VariantZero\",\"version\":999}");
    FFileHelper::SaveStringToFile(Future, *Path);
    TestTrue(TEXT("future version blocks fallback"), FVZSaveStore::Read(Path, Loaded) == EVZReadResult::UnsupportedVersion);
    TestFalse(TEXT("future version cannot be overwritten"), FVZSaveStore::Write(Path, Initial));
    FString Preserved; FFileHelper::LoadFileToString(Preserved, *Path);
    TestEqual(TEXT("future file retained exactly"), Preserved, Future);
    auto Invalid = Initial; const FGuid Duplicate = Invalid.Party[0]; Invalid.Party.Add(Duplicate);
    TestFalse(TEXT("duplicate party rejected"), FVZSaveStore::Validate(Invalid));
    Invalid = Initial; Invalid.Companions[0].Level = 21;
    TestFalse(TEXT("level cap enforced"), FVZSaveStore::Validate(Invalid));
    Invalid = Initial; Invalid.Companions[0].SpeciesId = TEXT("V-042");
    TestFalse(TEXT("drill only species rejected in world"), FVZSaveStore::Validate(Invalid));
    Invalid = Initial; Invalid.Materials.Add(TEXT("sample"), -1);
    TestFalse(TEXT("negative inventory rejected"), FVZSaveStore::Write(Directory / TEXT("invalid.vzsave"), Invalid));
    FFileHelper::SaveStringToFile(TEXT("broken"), *(Path + TEXT(".bak")));
    FFileHelper::SaveStringToFile(TEXT("broken"), *Path);
    TestTrue(TEXT("both corrupt surfaced"), FVZSaveStore::Read(Path, Loaded) == EVZReadResult::Corrupt);
    TestFalse(TEXT("both corrupt not overwritten"), FVZSaveStore::Write(Path, Initial));
    auto Reward = Initial;
    TestTrue(TEXT("first repair stages reward"), FVZSaveStore::StageFlowerRepair(Reward));
    TestFalse(TEXT("duplicate repair cannot reward"), FVZSaveStore::StageFlowerRepair(Reward));
    TestEqual(TEXT("reward remains exactly three"), Reward.Materials.FindRef(TEXT("research.sample")), 3);
    const FString RewardPath = Directory / TEXT("reward.vzsave");
    TestTrue(TEXT("reward and receipt persist together"), FVZSaveStore::Write(RewardPath, Reward));
    TestTrue(TEXT("reload receipt"), FVZSaveStore::Read(RewardPath, Loaded) == EVZReadResult::Ok);
    TestFalse(TEXT("reload cannot duplicate reward"), FVZSaveStore::StageFlowerRepair(Loaded));
    TestTrue(TEXT("repair state matches receipt"), Loaded.Repairs.Contains(TEXT("p0.flowerbed")));
    // Only this test's fresh GUID directory is removed; player saves are never used.
    IFileManager::Get().DeleteDirectory(*Directory, false, true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZIdentityTest, "VariantZero.Data.SpeciesBoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FVZIdentityTest::RunTest(const FString& Parameters)
{
    int32 Count = 0;
    for (int32 I = 1; I <= 45; ++I)
    {
        const FString Id = FString::Printf(TEXT("V-%03d"), I);
        TestTrue(TEXT("all 45 drill identities accepted"), FVZSaveStore::IsDrillSpecies(Id));
        if (FVZSaveStore::IsWorldSpecies(Id)) ++Count;
    }
    TestEqual(TEXT("exactly 12 world identities"), Count, 12);
    TestFalse(TEXT("malformed ID"), FVZSaveStore::IsDrillSpecies(TEXT("V-01x")));
    TestFalse(TEXT("zero out of range"), FVZSaveStore::IsDrillSpecies(TEXT("V-000")));
    TestFalse(TEXT("46 out of range"), FVZSaveStore::IsDrillSpecies(TEXT("V-046")));
    return true;
}
#endif
