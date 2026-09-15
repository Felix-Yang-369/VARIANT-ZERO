#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VZState.generated.h"

USTRUCT(BlueprintType)
struct FVZCompanion
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid IndividualId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SpeciesId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> Memories;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AppearanceSeed = 1;
};

USTRUCT(BlueprintType)
struct FVZWorldState
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Version = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PlayerName = TEXT("生态师");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool StoryDifficulty = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Checkpoint = FVector(0, 0, 120);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVZCompanion> Companions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FGuid> Party;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, int32> QuestStages;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSet<FString> Repairs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, int32> Materials;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSet<FString> RewardTransactions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSet<FString> DrillUnlocks;
    // Reserved schema boundary. Native drill simulation is not implemented yet.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DrillRunJson;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool LegacyImported = false;
};

enum class EVZReadResult { Ok, Recovered, Missing, Corrupt, UnsupportedVersion, IOError };

// File codec is independent of actors, for deterministic and destructive fault tests in a temporary directory.
class VARIANTZEROUE_API FVZSaveStore
{
public:
    static FVZWorldState NewGame();
    static bool Validate(const FVZWorldState& State);
    static bool IsWorldSpecies(const FString& Id);
    static bool IsDrillSpecies(const FString& Id);
    static bool StageFlowerRepair(FVZWorldState& Candidate);
    static bool Write(const FString& Path, const FVZWorldState& State);
    static EVZReadResult Read(const FString& Path, FVZWorldState& Out);
    static EVZReadResult ReadOne(const FString& Path, FVZWorldState& Out);
};

UCLASS()
class VARIANTZEROUE_API UVZStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UPROPERTY(BlueprintReadOnly) FVZWorldState State;
    UPROPERTY(BlueprintReadOnly) FString LastMessage;
    UPROPERTY(BlueprintReadOnly) bool WritesBlocked = false;
    UFUNCTION(BlueprintCallable) bool SaveSlot(int32 Slot);
    UFUNCTION(BlueprintCallable) bool LoadSlot(int32 Slot);
    UFUNCTION(BlueprintCallable) bool RepairTestFlower();
    UFUNCTION(BlueprintCallable) bool SetCheckpoint(FVector Position);
    // Candidates are persisted before replacing live state: failed writes cannot grant in-memory rewards.
    bool Commit(const FVZWorldState& Candidate);
    FString SlotPath(int32 Slot) const;
};
