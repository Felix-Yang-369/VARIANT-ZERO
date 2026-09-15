#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "VZPrototype.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UVZStateSubsystem;
class UAnimSequence;
class UVZVitalComponent;
class AVZDisturbance;
class UVZPrologue;
class UVZGarden;
class UVZChuyaStory;

// Functional P0 proxy. This is deliberately not presented as a finished ecological companion asset.
UCLASS()
class VARIANTZEROUE_API AVZCompanionProxy : public ACharacter
{
    GENERATED_BODY()
public:
    AVZCompanionProxy();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TObjectPtr<ACharacter> Leader;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY() TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY() TObjectPtr<USkeletalMesh> MothMesh;
    UPROPERTY() TObjectPtr<UAnimSequence> MothHover;
    UPROPERTY() TObjectPtr<UAnimSequence> MothFly;
    UPROPERTY() TObjectPtr<UMaterialInterface> MothMaterial;
    UPROPERTY() FString SpeciesId=TEXT("V-001");
    UPROPERTY() FGuid IndividualId;
    void ConfigureSpecies(const FString& Id);
    UPROPERTY(VisibleAnywhere) TObjectPtr<UVZVitalComponent> Vital;
    float AttackCooldown=0;
    bool Walking = false;
    FVector FormationOffset = FVector(-180, 100, 0);
    float StuckSeconds = 0;
    float RepathSeconds = 0;
    bool HasGroundOrder=false;
    FVector GroundOrder=FVector::ZeroVector;
    float OrderSeconds=0;
    float RecallSeconds=0;
    int32 GroundSkillsCompleted=0;
    void CancelOrder();
    bool TickGroundOrder(float Dt);
};

UCLASS()
class VARIANTZEROUE_API AVZEcologist : public ACharacter
{
    GENERATED_BODY()
public:
    AVZEcologist();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UVZPrologue> Prologue;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UVZGarden> Garden;
    UPROPERTY() TObjectPtr<UVZChuyaStory> ChuyaStory;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Boom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UVZVitalComponent> Vital;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Staff;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ShieldVisual;
    UPROPERTY() TObjectPtr<AVZDisturbance> Disturbance;
    UPROPERTY() TObjectPtr<UMaterialInterface> RepairedFlowerMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DormantFlowerMaterial;
    UPROPERTY() TArray<TObjectPtr<AVZCompanionProxy>> Companions;
    int32 ManualSlot = 1;
    float DodgeCooldown = 0;
    float PulseCooldown=0;
    float ShieldCooldown=0;
    float CompanionSkillCooldown=0;
    float CompanionSkillCooldown2=0;
    float CompanionSkillCooldown3=0;
    void CompanionSkill2(); void CompanionSkill3(); void UseCompanionSkill(int32 Slot);
    bool PulseHeld=false;
    bool AimingOrder=false;
    bool OrderAimValid=false;
    FVector OrderAim=FVector::ZeroVector;
    void BeginOrderAim(); void ReleaseOrderAim(); void RecallParty();
    void TickOrderAim();
    bool IssueGroundOrder(FVector Point);
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Sprint(); void StopSprint(); void Dodge(); void Interact(); void Scan();
    void Save(); void Load(); void CycleSlot(); void Checkpoint(); void Respawn();
    void RebuildParty();
    void RefreshEcology();
    void StartPulse(); void StopPulse(); void FirePulse(); void Shield(); void CompanionSkill(); void StartEncounter(); void ToggleDifficulty();
    UVZStateSubsystem* State() const;
};

UCLASS()
class VARIANTZEROUE_API AVZPrototypeMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AVZPrototypeMode();
    virtual void StartPlay() override;
};

UCLASS()
class VARIANTZEROUE_API AVZPrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
