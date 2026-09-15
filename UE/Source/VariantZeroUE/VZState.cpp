#include "VZState.h"
#include "VZSaveLocation.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Crc.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    constexpr int32 MaxSaveBytes = 4 * 1024 * 1024;
    constexpr int32 FormatVersion = 1;
    FString Checksum(const FString& Payload)
    {
        FTCHARToUTF8 Bytes(*Payload);
        return FString::Printf(TEXT("%08x"), FCrc::MemCrc32(Bytes.Get(), Bytes.Length()));
    }
}

bool FVZSaveStore::IsDrillSpecies(const FString& Id)
{
    if (Id.Len() != 5 || !Id.StartsWith(TEXT("V-"))) return false;
    for (int32 I = 2; I < 5; ++I) if (!FChar::IsDigit(Id[I])) return false;
    const int32 Number = FCString::Atoi(*Id.Mid(2));
    return Number >= 1 && Number <= 45;
}

bool FVZSaveStore::IsWorldSpecies(const FString& Id)
{
    static const TSet<FString> Ids = {TEXT("V-001"),TEXT("V-002"),TEXT("V-003"),TEXT("V-004"),TEXT("V-005"),TEXT("V-006"),TEXT("V-011"),TEXT("V-016"),TEXT("V-020"),TEXT("V-039"),TEXT("V-041"),TEXT("V-045")};
    return Ids.Contains(Id);
}

FVZWorldState FVZSaveStore::NewGame()
{
    FVZWorldState S;
    FVZCompanion Bud;
    Bud.IndividualId = FGuid::NewGuid(); Bud.SpeciesId = TEXT("V-001"); Bud.Name = TEXT("初芽");
    Bud.Memories.Add(TEXT("origin.first_companion"));
    S.Companions.Add(Bud); S.Party.Add(Bud.IndividualId);
    S.QuestStages.Add(TEXT("p0.first_flower"), 0);
    for (int32 I = 1; I <= 5; ++I) S.DrillUnlocks.Add(FString::Printf(TEXT("V-%03d"), I));
    return S;
}

bool FVZSaveStore::Validate(const FVZWorldState& S)
{
    if (S.Version != FormatVersion || S.PlayerName.IsEmpty() || S.PlayerName.Len() > 64 || S.Checkpoint.ContainsNaN() || S.Checkpoint.GetAbsMax() > 2000000) return false;
    if (S.Companions.Num() == 0 || S.Companions.Num() > 512 || S.Party.Num() > 3 || S.Party.Num() == 0) return false;
    if (S.RewardTransactions.Num() > 10000 || S.QuestStages.Num() > 1000 || S.Repairs.Num() > 1000 || S.Materials.Num() > 256 || S.DrillRunJson.Len() > 1000000) return false;
    TSet<FGuid> Ids;
    for (const FVZCompanion& C : S.Companions)
    {
        if (!C.IndividualId.IsValid() || Ids.Contains(C.IndividualId) || !IsWorldSpecies(C.SpeciesId) || C.Level < 1 || C.Level > 20 || C.Name.Len() > 64 || C.Memories.Num() > 1000) return false;
        Ids.Add(C.IndividualId);
    }
    TSet<FGuid> Party;
    for (const FGuid& Id : S.Party) { if (!Ids.Contains(Id) || Party.Contains(Id)) return false; Party.Add(Id); }
    for (const auto& M : S.Materials) if (M.Key.IsEmpty() || M.Key.Len() > 128 || M.Value < 0 || M.Value > 1000000) return false;
    for (const auto& Q : S.QuestStages) if (Q.Key.IsEmpty() || Q.Key.Len() > 128 || Q.Value < 0 || Q.Value > 10000) return false;
    for (const auto& Id : S.RewardTransactions) if (Id.IsEmpty() || Id.Len() > 256) return false;
    for (const auto& Id : S.Repairs) if (Id.IsEmpty() || Id.Len() > 128) return false;
    for (const auto& Id : S.DrillUnlocks) if (!IsDrillSpecies(Id)) return false;
    return true;
}

EVZReadResult FVZSaveStore::ReadOne(const FString& Path, FVZWorldState& Out)
{
    const int64 Size = IFileManager::Get().FileSize(*Path);
    if (Size < 0) return EVZReadResult::Missing;
    if (Size == 0 || Size > MaxSaveBytes) return EVZReadResult::Corrupt;
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *Path)) return EVZReadResult::IOError;
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid()) return EVZReadResult::Corrupt;
    FString Magic, Payload, Crc; double Version = 0;
    if (!Root->TryGetStringField(TEXT("magic"), Magic) || Magic != TEXT("VariantZero") || !Root->TryGetNumberField(TEXT("version"), Version)) return EVZReadResult::Corrupt;
    // Never fall back and overwrite a future format with an older backup.
    if (Version != FormatVersion) return EVZReadResult::UnsupportedVersion;
    if (!Root->TryGetStringField(TEXT("payload"), Payload) || !Root->TryGetStringField(TEXT("crc32"), Crc) || Checksum(Payload) != Crc) return EVZReadResult::Corrupt;
    FVZWorldState Candidate;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(Payload, &Candidate, 0, 0, true) || !Validate(Candidate)) return EVZReadResult::Corrupt;
    Out = MoveTemp(Candidate);
    return EVZReadResult::Ok;
}

EVZReadResult FVZSaveStore::Read(const FString& Path, FVZWorldState& Out)
{
    const auto Result = ReadOne(Path, Out);
    if (Result == EVZReadResult::Ok || Result == EVZReadResult::UnsupportedVersion || Result == EVZReadResult::IOError) return Result;
    const auto Backup = ReadOne(Path + TEXT(".bak"), Out);
    if (Backup == EVZReadResult::Ok) return EVZReadResult::Recovered;
    if (Backup == EVZReadResult::UnsupportedVersion || Backup == EVZReadResult::IOError) return Backup;
    return Result == EVZReadResult::Missing && Backup == EVZReadResult::Missing ? EVZReadResult::Missing : EVZReadResult::Corrupt;
}

bool FVZSaveStore::Write(const FString& Path, const FVZWorldState& State)
{
    if (!Validate(State)) return false;
    FVZWorldState Existing;
    const auto Current = ReadOne(Path, Existing);
    if (Current == EVZReadResult::UnsupportedVersion || Current == EVZReadResult::IOError) return false;
    const auto Backup = ReadOne(Path + TEXT(".bak"), Existing);
    if (Backup == EVZReadResult::UnsupportedVersion || Backup == EVZReadResult::IOError) return false;
    // Corrupt history must remain recoverable for inspection, never silently replace both copies.
    if (Current != EVZReadResult::Ok && Backup != EVZReadResult::Ok && (Current != EVZReadResult::Missing || Backup != EVZReadResult::Missing)) return false;
    FString Payload;
    if (!FJsonObjectConverter::UStructToJsonObjectString(State, Payload)) return false;
    auto Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("magic"), TEXT("VariantZero")); Root->SetNumberField(TEXT("version"), FormatVersion);
    Root->SetStringField(TEXT("payload"), Payload); Root->SetStringField(TEXT("crc32"), Checksum(Payload));
    FString Text;
    if (!FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Text))) return false;
    if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true)) return false;
    const FString Temp = Path + TEXT(".tmp");
    if (!FFileHelper::SaveStringToFile(Text, *Temp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)) return false;
    FVZWorldState Verified;
    if (ReadOne(Temp, Verified) != EVZReadResult::Ok) return false;
    if (Current == EVZReadResult::Ok)
    {
        const FString BackupTemp = Path + TEXT(".bak.tmp");
        if (IFileManager::Get().Copy(*BackupTemp, *Path, true, false) != COPY_OK || ReadOne(BackupTemp, Verified) != EVZReadResult::Ok) return false;
        if (!IFileManager::Get().Move(*(Path + TEXT(".bak")), *BackupTemp, true, false, false, true)) return false;
    }
    return IFileManager::Get().Move(*Path, *Temp, true, false, false, true);
}

void UVZStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    State = FVZSaveStore::NewGame();
    const FString Shared=FVZSaveLocation::StoryDirectory(FVZSaveLocation::UserRoot());
    if(FPaths::IsSamePath(FPaths::GetPath(SlotPath(0)),Shared))
    {
        FString Legacy=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("SaveGames/ProloguePrototype"));
        const bool ExplicitLegacy=FParse::Value(FCommandLine::Get(),TEXT("VZLegacySaveDir="),Legacy);
        const auto Migration=FVZSaveLocation::ImportLegacy(Legacy,Shared);
        if(Migration==EVZMigration::Failed||(ExplicitLegacy&&Migration==EVZMigration::NoSource))
        {WritesBlocked=true;LastMessage=TEXT("旧存档迁移失败，已保护原文件并暂停保存。请检查旧档路径、版本或备份。");return;}
    }
    LoadSlot(0);
}

FString UVZStateSubsystem::SlotPath(int32 Slot) const
{
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(), TEXT("VZChuyaStoryTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZTravelTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZGardenTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZStoryTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZHeroTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZSmokeTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZCombatTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZPreview")) || FParse::Param(FCommandLine::Get(), TEXT("VZMenuTest")) || FParse::Param(FCommandLine::Get(), TEXT("VZOrderTest")))
    {
        static const FString TestRun = FGuid::NewGuid().ToString();
        return FPaths::ProjectSavedDir() / TEXT("Automation/Runtime") / TestRun / FString::Printf(TEXT("slot_%d.vzsave"), Slot);
    }
#endif
    if(FParse::Param(FCommandLine::Get(),TEXT("VZStory")))
        return FVZSaveLocation::StoryDirectory(FVZSaveLocation::UserRoot()) / FString::Printf(TEXT("slot_%d.vzsave"),Slot);
    return FPaths::ProjectSavedDir() / TEXT("SaveGames/VariantZero") / FString::Printf(TEXT("slot_%d.vzsave"), Slot);
}

bool UVZStateSubsystem::SaveSlot(int32 Slot)
{
    if (Slot < 0 || Slot > 3 || WritesBlocked) { LastMessage = TEXT("存档受保护，未覆盖。"); return false; }
    const bool Ok = FVZSaveStore::Write(SlotPath(Slot), State);
    LastMessage = Ok ? TEXT("保存完成") : TEXT("保存失败；已有存档保留");
    return Ok;
}

bool UVZStateSubsystem::LoadSlot(int32 Slot)
{
    if (Slot < 0 || Slot > 3) return false;
    FVZWorldState Candidate;
    const auto Result = FVZSaveStore::Read(SlotPath(Slot), Candidate);
    if (Result == EVZReadResult::Ok || Result == EVZReadResult::Recovered)
    {
        State = MoveTemp(Candidate); WritesBlocked = false;
        LastMessage = Result == EVZReadResult::Recovered ? TEXT("已从上一版备份恢复") : TEXT("读取完成");
        return true;
    }
    // A failed manual load must not lift an existing startup save protection.
    if (Slot == 0) WritesBlocked = Result != EVZReadResult::Missing;
    LastMessage = Result == EVZReadResult::Missing ? TEXT("此槽位没有存档") : TEXT("存档损坏或版本不支持；未覆盖");
    return false;
}

bool UVZStateSubsystem::Commit(const FVZWorldState& Candidate)
{
    if (WritesBlocked || !FVZSaveStore::Write(SlotPath(0), Candidate)) { LastMessage = TEXT("自动保存失败；本次变更未提交"); return false; }
    State = Candidate; LastMessage = TEXT("进度已自动保存"); return true;
}

bool FVZSaveStore::StageFlowerRepair(FVZWorldState& Candidate)
{
    const FString Transaction = TEXT("p0.first_flower.reward.v1");
    if (!Validate(Candidate) || Candidate.RewardTransactions.Contains(Transaction)) return false;
    Candidate.Repairs.Add(TEXT("p0.flowerbed")); Candidate.QuestStages.Add(TEXT("p0.first_flower"), 1);
    Candidate.Materials.FindOrAdd(TEXT("research.sample")) += 3;
    Candidate.RewardTransactions.Add(Transaction);
    Candidate.Companions[0].Memories.AddUnique(TEXT("p0.first_flower"));
    return Validate(Candidate);
}

bool UVZStateSubsystem::RepairTestFlower()
{
    auto Candidate = State;
    if (!FVZSaveStore::StageFlowerRepair(Candidate)) { LastMessage = TEXT("花房已修复或状态无效；未重复发奖"); return false; }
    return Commit(Candidate);
}

bool UVZStateSubsystem::SetCheckpoint(FVector Position)
{
    auto Candidate = State; Candidate.Checkpoint = Position; return Commit(Candidate);
}

