#include "VZPrototype.h"
#include "VZPlayerController.h"
#include "VZState.h"
#include "VZPrologue.h"
#include "VZGarden.h"
#include "VZChuyaStory.h"
#include "VZCombat.h"
#include "VZAudioDirector.h"
#include "VZCombatVerification.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"
#include "Animation/AnimSequence.h"
#include "VZHeroAnimation.h"
#include "VZRoster.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "NavigationPath.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

static int32 VZCompanionLevel(const AVZCompanionProxy* Companion)
{
    const auto* P=Cast<AVZEcologist>(Companion->Leader);
    return P?FVZRosterRules::Level(P->State()->State,Companion->IndividualId):1;
}

AVZCompanionProxy::AVZCompanionProxy()
{
    Vital=CreateDefaultSubobject<UVZVitalComponent>(TEXT("Vital"));
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(24, 35);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody")); Body->SetupAttachment(GetRootComponent());
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere"));
    Body->SetStaticMesh(Sphere.Object); Body->SetRelativeScale3D(FVector(.45, .45, .65)); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (!FParse::Param(FCommandLine::Get(),TEXT("VZAssetImport")))
    {
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Chuya(TEXT("/Game/VariantZero/Characters/Chuya/SK_Chuya"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(TEXT("/Game/VariantZero/Characters/Chuya/A_Chuya_Idle"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Walk(TEXT("/Game/VariantZero/Characters/Chuya/A_Chuya_Walk"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/VariantZero/Characters/Chuya/M_Chuya"));
    if (Chuya.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(Chuya.Object); GetMesh()->SetRelativeLocation(FVector(0,0,-35));
        GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);
        if (Material.Succeeded()) GetMesh()->SetMaterial(0,Material.Object);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); Body->SetVisibility(false);
    }
    IdleAnimation = Idle.Object; WalkAnimation = Walk.Object;
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Moth(TEXT("/Game/VariantZero/Characters/Moth/SK_Moth"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Hover(TEXT("/Game/VariantZero/Characters/Moth/A_Moth_Hover"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Fly(TEXT("/Game/VariantZero/Characters/Moth/A_Moth_Fly"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MothMat(TEXT("/Game/VariantZero/Characters/Moth/M_Moth"));
    MothMesh=Moth.Object;MothHover=Hover.Object;MothFly=Fly.Object;MothMaterial=MothMat.Object;
    }
    GetCharacterMovement()->MaxWalkSpeed = 480;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    AIControllerClass = AAIController::StaticClass(); AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void StartVZOrderVerification(UWorld* World);

void AVZCompanionProxy::ConfigureSpecies(const FString& Id)
{
    SpeciesId=Id;
    if(Id==TEXT("V-041") && MothMesh && MothHover && MothFly)
    {
        GetMesh()->SetSkeletalMesh(MothMesh);GetMesh()->SetMaterial(0,MothMaterial);
        GetMesh()->SetRelativeLocation(FVector(0,0,85));
        IdleAnimation=MothHover;WalkAnimation=MothFly;GetMesh()->PlayAnimation(IdleAnimation,true);Walking=false;
    }
}

void AVZCompanionProxy::BeginPlay()
{
    Super::BeginPlay();
    if (IdleAnimation) GetMesh()->PlayAnimation(IdleAnimation,true);
}

void AVZCompanionProxy::Tick(float Dt)
{
    Super::Tick(Dt);
    AttackCooldown=FMath::Max(0.f,AttackCooldown-Dt);
    if(TickGroundOrder(Dt))return;
    if (Vital->IsDown())
    {
        GetCharacterMovement()->StopMovementImmediately();
        if (auto* AI=Cast<AAIController>(GetController())) AI->StopMovement();
        if (Walking && IdleAnimation) GetMesh()->PlayAnimation(IdleAnimation,true);
        Walking=false;return;
    }
    if (auto* Ecologist=Cast<AVZEcologist>(Leader))
    {
        auto* Target=Ecologist->Disturbance.Get();
        if (IsValid(Target) && Target->Active && RecallSeconds<=0 && AttackCooldown<=0 && FVector::Dist(GetActorLocation(),Target->GetActorLocation())<850 && AVZDisturbance::HasClearShot(this,Target,GetActorLocation()+FVector(0,0,35)))
        {
            AVZDisturbance::DrawPulse(GetWorld(),GetActorLocation()+FVector(0,0,35),Target->GetActorLocation());Target->Hit(FVZRosterRules::BasicDamage(VZCompanionLevel(this)),this);AttackCooldown=1.3f;
        }
    }
    const bool Moving = GetVelocity().Size2D() > 12;
    if (Moving != Walking)
    {
        Walking = Moving;
        if (IdleAnimation && WalkAnimation) GetMesh()->PlayAnimation(Walking ? WalkAnimation : IdleAnimation,true);
    }
    if (!IsValid(Leader)) return;
    const FVector Goal = Leader->GetActorLocation() + Leader->GetActorRotation().RotateVector(FormationOffset);
    const FVector Delta = Goal - GetActorLocation();
    if (Delta.Size2D() > 100)
    {
        RepathSeconds -= Dt;
        // Prefer navigation once a production area supplies nav data. Direct movement is only the empty P0 floor fallback.
        auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (Nav && Nav->GetDefaultNavDataInstance())
        {
            if (RepathSeconds <= 0)
            {
                if (auto* AI = Cast<AAIController>(GetController())) AI->MoveToLocation(Goal, 65);
                RepathSeconds = .5f;
            }
        }
        else AddMovementInput(Delta.GetSafeNormal2D());
        StuckSeconds = GetVelocity().Size2D() < 10 ? StuckSeconds + Dt : 0;
    }
    else StuckSeconds = 0;
    if (StuckSeconds > 4 || Delta.Size2D() > 2000)
    {
        auto* PC = Cast<APlayerController>(Leader->GetController());
        if (!PC) return;
        FVector Eye; FRotator View; PC->GetPlayerViewPoint(Eye, View);
        FVector Destination = Eye - View.Vector().GetSafeNormal2D() * 220;
        Destination.Z = Leader->GetActorLocation().Z;
        FHitResult Ground;
        FCollisionQueryParams Params; Params.AddIgnoredActor(this); Params.AddIgnoredActor(Leader);
        if (GetWorld()->LineTraceSingleByChannel(Ground, Destination + FVector(0,0,200), Destination - FVector(0,0,500), ECC_Visibility, Params))
        {
            Destination = Ground.ImpactPoint + FVector(0,0,40);
            // Both departure and arrival must be behind the camera; FindTeleportSpot checks capsule clearance.
            if (FVector::DotProduct(View.Vector(), (GetActorLocation()-Eye).GetSafeNormal()) < 0 && FVector::DotProduct(View.Vector(), (Destination-Eye).GetSafeNormal()) < 0 && GetWorld()->FindTeleportSpot(this, Destination, GetActorRotation()))
            { TeleportTo(Destination, GetActorRotation()); StuckSeconds = 0; }
        }
    }
}

AVZEcologist::AVZEcologist()
{
    Prologue=CreateDefaultSubobject<UVZPrologue>(TEXT("Prologue"));
    Garden=CreateDefaultSubobject<UVZGarden>(TEXT("Garden"));
    ChuyaStory=CreateDefaultSubobject<UVZChuyaStory>(TEXT("ChuyaStory"));
    Vital=CreateDefaultSubobject<UVZVitalComponent>(TEXT("Vital"));
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(35, 90);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 420;
    GetCharacterMovement()->JumpZVelocity = 550;
    GetCharacterMovement()->AirControl = .3f;
    Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); Boom->SetupAttachment(GetRootComponent()); Boom->TargetArmLength = 420; Boom->bUsePawnControlRotation = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); Camera->SetupAttachment(Boom);
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Hero(TEXT("/Game/ParagonLtBelica/Characters/Heroes/Belica/Meshes/Belica"));
    GetMesh()->SetSkeletalMesh(Hero.Object); GetMesh()->SetRelativeLocation(FVector(0,0,-90)); GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
    GetMesh()->SetAnimInstanceClass(UVZHeroAnimation::StaticClass());
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Upper(TEXT("/Game/VariantZero/Ecology/MI_EcologistUpper"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Lower(TEXT("/Game/VariantZero/Ecology/MI_EcologistLower"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Pad(TEXT("/Game/VariantZero/Ecology/MI_EcologistPad"));
    GetMesh()->SetMaterial(0,Upper.Object);GetMesh()->SetMaterial(1,Lower.Object);GetMesh()->SetMaterial(2,Pad.Object);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Petal(TEXT("/Game/VariantZero/Environment/Materials/M_Petal"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Dormant(TEXT("/Game/VariantZero/Environment/Materials/M_DormantLeaf"));
    RepairedFlowerMaterial = Petal.Object; DormantFlowerMaterial = Dormant.Object;
    Staff=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CultivationStaff"));Staff->SetupAttachment(GetMesh(),TEXT("hand_r"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Rod(TEXT("/Game/VariantZero/Ecology/SM_FieldScanner"));
    Staff->SetStaticMesh(Rod.Object);Staff->SetRelativeLocation(FVector(0,0,0));Staff->SetRelativeRotation(FRotator(0,0,90));Staff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldVisual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EcologicalShield"));ShieldVisual->SetupAttachment(GetRootComponent());
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShieldSphere(TEXT("/Engine/BasicShapes/Sphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShieldMaterial(TEXT("/Game/VariantZero/Ecology/M_EcologicalShield"));
    ShieldVisual->SetStaticMesh(ShieldSphere.Object);ShieldVisual->SetMaterial(0,ShieldMaterial.Object);
    ShieldVisual->SetRelativeScale3D(FVector(2.1f));ShieldVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldVisual->SetCanEverAffectNavigation(false);ShieldVisual->SetCastShadow(false);ShieldVisual->SetVisibility(false);
}

UVZStateSubsystem* AVZEcologist::State() const { return GetGameInstance()->GetSubsystem<UVZStateSubsystem>(); }
void AVZEcologist::BeginPlay()
{
    Super::BeginPlay(); SetActorLocation(State()->State.Checkpoint); RebuildParty(); RefreshEcology();
    // Hide the supplied firearm while retaining the original source asset.
    GetMesh()->HideBoneByName(TEXT("weapon"),EPhysBodyOp::PBO_None);
    Disturbance=GetWorld()->SpawnActor<AVZDisturbance>(FVector(550,-450,130),FRotator::ZeroRotator);
    Garden->Refresh();
    if(!State()->WritesBlocked)
    {
        State()->LastMessage=TEXT("欢迎回来。T 开始处理温室异常，Q 扫描；靠近花床按 E 照料。");
        if(Prologue->Enabled())State()->LastMessage=Prologue->Objective();
    }
}
void AVZEcologist::Tick(float Dt)
{
    Super::Tick(Dt); DodgeCooldown = FMath::Max(0.f, DodgeCooldown-Dt);
    PulseCooldown=FMath::Max(0.f,PulseCooldown-Dt);ShieldCooldown=FMath::Max(0.f,ShieldCooldown-Dt);CompanionSkillCooldown=FMath::Max(0.f,CompanionSkillCooldown-Dt);
    CompanionSkillCooldown2=FMath::Max(0.f,CompanionSkillCooldown2-Dt);CompanionSkillCooldown3=FMath::Max(0.f,CompanionSkillCooldown3-Dt);
    if (Vital->IsDown()) { Respawn();State()->LastMessage=TEXT("已回到检查点，初芽和任务物品都还在。"); }
    if (PulseHeld) FirePulse();
    ShieldVisual->SetVisibility(Vital->ShieldSeconds>0);
    TickOrderAim();
    if (GetActorLocation().Z < -1000) Respawn();
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(), TEXT("VZHeroTest")) && GetWorld()->GetTimeSeconds()>1.f && GetWorld()->GetTimeSeconds()<2.f) MoveForward(1);
    if (FParse::Param(FCommandLine::Get(), TEXT("VZHeroTest")) && GetWorld()->GetTimeSeconds()>4.25f && GetWorld()->GetTimeSeconds()<4.6f) MoveForward(1);
    if (FParse::Param(FCommandLine::Get(), TEXT("VZSmokeTest")) && GetWorld()->GetTimeSeconds() < 1.5f) MoveRight(1);
#endif
}
void AVZEcologist::RebuildParty()
{
    for (AVZCompanionProxy* Companion : Companions) if (IsValid(Companion)) Companion->Destroy();
    Companions.Reset();
    for (int32 I = 0; I < State()->State.Party.Num(); ++I)
    {
        FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        auto* C = GetWorld()->SpawnActor<AVZCompanionProxy>(GetActorLocation()+FVector(-180,100+I*90,0), FRotator::ZeroRotator, Params);
        if (C)
        {
            C->Leader=this;C->FormationOffset=FVector(-180,100+I*90,0);C->IndividualId=State()->State.Party[I];
            for(const auto& Individual:State()->State.Companions)if(Individual.IndividualId==C->IndividualId){C->ConfigureSpecies(Individual.SpeciesId);C->Vital->MaxHealth=FVZRosterRules::MaxHealth(Individual.Level);C->Vital->Restore();break;}
            Companions.Add(C);
        }
    }
}
void AVZEcologist::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAction("VZOrder",IE_Pressed,this,&AVZEcologist::BeginOrderAim);Input->BindAction("VZOrder",IE_Released,this,&AVZEcologist::ReleaseOrderAim);
    Input->BindAction("VZRecall",IE_Pressed,this,&AVZEcologist::RecallParty);
    Input->BindAxis("VZForward", this, &AVZEcologist::MoveForward); Input->BindAxis("VZRight", this, &AVZEcologist::MoveRight);
    Input->BindAxis("VZLookX", this, &APawn::AddControllerYawInput); Input->BindAxis("VZLookY", this, &APawn::AddControllerPitchInput);
    Input->BindAction("VZJump", IE_Pressed, this, &ACharacter::Jump); Input->BindAction("VZJump", IE_Released, this, &ACharacter::StopJumping);
    Input->BindAction("VZSprint", IE_Pressed, this, &AVZEcologist::Sprint); Input->BindAction("VZSprint", IE_Released, this, &AVZEcologist::StopSprint);
    Input->BindAction("VZDodge", IE_Pressed, this, &AVZEcologist::Dodge); Input->BindAction("VZInteract", IE_Pressed, this, &AVZEcologist::Interact);
    Input->BindAction("VZScan", IE_Pressed, this, &AVZEcologist::Scan);
    Input->BindAction("VZPulse",IE_Pressed,this,&AVZEcologist::StartPulse);Input->BindAction("VZPulse",IE_Released,this,&AVZEcologist::StopPulse);
    Input->BindAction("VZShield",IE_Pressed,this,&AVZEcologist::Shield);Input->BindAction("VZCompanionSkill",IE_Pressed,this,&AVZEcologist::CompanionSkill);
    Input->BindAction("VZCompanionSkill2",IE_Pressed,this,&AVZEcologist::CompanionSkill2);Input->BindAction("VZCompanionSkill3",IE_Pressed,this,&AVZEcologist::CompanionSkill3);
    Input->BindAction("VZEncounter",IE_Pressed,this,&AVZEcologist::StartEncounter);Input->BindAction("VZDifficulty",IE_Pressed,this,&AVZEcologist::ToggleDifficulty);
    Input->BindAction("VZSave", IE_Pressed, this, &AVZEcologist::Save); Input->BindAction("VZLoad", IE_Pressed, this, &AVZEcologist::Load);
    Input->BindAction("VZSlot", IE_Pressed, this, &AVZEcologist::CycleSlot); Input->BindAction("VZCheckpoint", IE_Pressed, this, &AVZEcologist::Checkpoint);
    Input->BindAction("VZRespawn", IE_Pressed, this, &AVZEcologist::Respawn);
}
void AVZEcologist::MoveForward(float V) { if (Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X), V); }
void AVZEcologist::MoveRight(float V) { if (Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y), V); }
void AVZEcologist::Sprint() { GetCharacterMovement()->MaxWalkSpeed = 650; }
void AVZEcologist::StopSprint() { GetCharacterMovement()->MaxWalkSpeed = 420; }
void AVZEcologist::Dodge() { if (DodgeCooldown <= 0 && !GetCharacterMovement()->IsFalling()) { LaunchCharacter(GetActorForwardVector()*700+FVector(0,0,100), true, true); DodgeCooldown = 1.2f;Vital->InvulnerableSeconds=.35f; } }
void AVZEcologist::Interact()
{
    if(ChuyaStory->Interact())return;
    if(Garden->Interact())return;
    if(Prologue->Interact())return;
    if (FVector::Dist2D(GetActorLocation(), FVector(650,0,0)) > 240) { State()->LastMessage = TEXT("靠近中央花床，使用照料动作修复"); return; }
    if (State()->RepairTestFlower()) { RefreshEcology();AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair")); }
}
void AVZEcologist::RefreshEcology()
{
    const bool Repaired = Prologue->Enabled()?State()->State.Repairs.Contains(TEXT("prologue.flowerbed")):State()->State.Repairs.Contains(TEXT("p0.flowerbed"));
    UMaterialInterface* Material = Repaired ? RepairedFlowerMaterial : DormantFlowerMaterial;
    if (!Material) return;
    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
        if (It->ActorHasTag(TEXT("VZRepairFlower"))) It->GetStaticMeshComponent()->SetMaterial(0,Material);
}
void AVZEcologist::Scan()
{
    if(ChuyaStory->Scan())return;
    if(Garden->Scan())return;
    if(Prologue->Scan())return;
    if (IsValid(Disturbance) && Disturbance->Active && FVector::Dist(GetActorLocation(),Disturbance->GetActorLocation())<1200)
    { Disturbance->Marked=true;State()->LastMessage=TEXT("已标记失衡节点。离开橙色预警圈，指挥初芽协同压制。");return; }
    DrawDebugSphere(GetWorld(),FVector(650,0,110),170,24,FColor::Cyan,false,4);State()->LastMessage=TEXT("花床需要照料。靠近后执行照料，让第一株花重新开放。");
}
void AVZEcologist::StartPulse() { PulseHeld=true;FirePulse(); }
void AVZEcologist::StopPulse() { PulseHeld=false; }
void AVZEcologist::FirePulse()
{
    if (PulseCooldown>0 || !IsValid(Disturbance) || !Disturbance->Active || FVector::Dist(GetActorLocation(),Disturbance->GetActorLocation())>1200) return;
    if (!Disturbance->Marked && FVector::DotProduct(GetControlRotation().Vector(),(Disturbance->GetActorLocation()-GetActorLocation()).GetSafeNormal())<.75f) return;
    const FVector Origin=GetActorLocation()+FVector(0,0,25);
    if (!AVZDisturbance::HasClearShot(this,Disturbance,Origin)) return;
    AVZAudioDirector::PlayCue(GetWorld(),TEXT("Pulse"));PulseCooldown=.55f;AVZDisturbance::DrawPulse(GetWorld(),Origin,Disturbance->GetActorLocation());Disturbance->Hit(12,this);
    if(auto* Anim=Cast<UVZHeroAnimation>(GetMesh()->GetAnimInstance()))Anim->RequestAction(0);
}
void AVZEcologist::Shield()
{
    if (ShieldCooldown>0) return;
    Vital->ShieldSeconds=3;ShieldCooldown=8;AVZAudioDirector::PlayCue(GetWorld(),TEXT("Shield"));
    if(auto* Anim=Cast<UVZHeroAnimation>(GetMesh()->GetAnimInstance()))Anim->RequestAction(1);
}
void AVZEcologist::CompanionSkill()
{
    UseCompanionSkill(0);
}
void AVZEcologist::CompanionSkill2(){UseCompanionSkill(1);}
void AVZEcologist::CompanionSkill3(){UseCompanionSkill(2);}
void AVZEcologist::UseCompanionSkill(int32 Slot)
{
    if(Slot<0 || Slot>2 || !Companions.IsValidIndex(Slot) || Prologue->DialogueOpen())return;
    float& Cooldown=Slot==0?CompanionSkillCooldown:Slot==1?CompanionSkillCooldown2:CompanionSkillCooldown3;
    auto* C=Companions[Slot].Get();
    if(!IsValid(C) || C->Vital->IsDown() || C->HasGroundOrder || Cooldown>0)return;
    if(C->SpeciesId==TEXT("V-041"))
    {
        if(FVector::Dist2D(C->GetActorLocation(),GetActorLocation())>600)return;
        Prologue->CancelPollination();
        bool Affected=false;
        auto Heal=[&](UVZVitalComponent* V,FVector Position)
        {
            if(!V->IsDown() && V->Health<V->MaxHealth && FVector::Dist2D(Position,C->GetActorLocation())<=600)
            {V->Health=FMath::Min(V->MaxHealth,V->Health+FVZRosterRules::Healing(VZCompanionLevel(C)));Affected=true;AVZDisturbance::DrawPulse(GetWorld(),C->GetMesh()->GetComponentLocation(),Position,true);}
        };
        Heal(Vital,GetActorLocation());for(auto Ally:Companions)if(IsValid(Ally))Heal(Ally->Vital,Ally->GetActorLocation());
        if(Affected){Cooldown=10;AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair"));State()->LastMessage=FString::Printf(TEXT("菌翼蛾：花粉抚慰，附近存活队员恢复 %.0f 点生命。"),FVZRosterRules::Healing(VZCompanionLevel(C)));}
        else State()->LastMessage=TEXT("附近队员无需恢复；技能未进入冷却。");
        return;
    }
    if(!IsValid(Disturbance) || !Disturbance->Active)return;
    if (!IsValid(C) || C->Vital->IsDown() || FVector::Dist(C->GetActorLocation(),Disturbance->GetActorLocation())>1100 || !AVZDisturbance::HasClearShot(C,Disturbance,C->GetActorLocation()+FVector(0,0,35))) return;
    Cooldown=6;AVZDisturbance::DrawPulse(GetWorld(),C->GetActorLocation()+FVector(0,0,35),Disturbance->GetActorLocation(),true);Disturbance->Hit(FVZRosterRules::SkillDamage(VZCompanionLevel(C)),C);
}
void AVZEcologist::StartEncounter()
{
    if(Garden->Enabled() && GetActorLocation().Y>1800){Garden->StartEncounter();return;}
    if(Prologue->Enabled() && (Prologue->DialogueOpen() || FVZPrologueRules::Stage(State()->State)<4)) {State()->LastMessage=Prologue->Objective();return;}
    if (IsValid(Disturbance) && !Disturbance->Active)
    { for (AVZCompanionProxy* C:Companions) if (IsValid(C)) C->Vital->Restore();Disturbance->Start(this); }
}
void AVZEcologist::ToggleDifficulty()
{
    auto Candidate=State()->State;Candidate.StoryDifficulty=!Candidate.StoryDifficulty;
    if (State()->Commit(Candidate)) State()->LastMessage=Candidate.StoryDifficulty?TEXT("故事难度：受到的伤害降低，奖励相同。"):TEXT("标准难度：注意预警和护盾时机，奖励相同。");
}
void AVZEcologist::Save() { State()->SaveSlot(ManualSlot); }
void AVZEcologist::Load() { if (State()->LoadSlot(ManualSlot)) { Respawn(); RefreshEcology(); } }
void AVZEcologist::CycleSlot() { ManualSlot = ManualSlot % 3 + 1; }
void AVZEcologist::Checkpoint() { if (!GetCharacterMovement()->IsMovingOnGround()) return; State()->SetCheckpoint(GetActorLocation()); }
void AVZEcologist::Respawn()
{
    Prologue->CancelPollination();
    Prologue->Dismiss();
    if(Garden->Guardian){Garden->Guardian->ResetEncounter();Garden->Guardian->ExposureSeconds=0;}
    if (IsValid(Disturbance)) Disturbance->ResetEncounter();
    AimingOrder=false;OrderAimValid=false;Vital->Restore();PulseHeld=false;PulseCooldown=0;ShieldCooldown=0;CompanionSkillCooldown=0;DodgeCooldown=0;
    if(auto* Anim=Cast<UVZHeroAnimation>(GetMesh()->GetAnimInstance()))Anim->ResetActions();
    ShieldVisual->SetVisibility(false);
    CompanionSkillCooldown2=0;CompanionSkillCooldown3=0;
    GetCharacterMovement()->StopMovementImmediately();SetActorLocation(State()->State.Checkpoint);RebuildParty();
    Prologue->Refresh();
    Garden->Refresh();
}

AVZPrototypeMode::AVZPrototypeMode() { PlayerControllerClass = AVZPlayerController::StaticClass(); DefaultPawnClass = AVZEcologist::StaticClass(); HUDClass = AVZPrototypeHUD::StaticClass(); }
void AVZPrototypeMode::StartPlay()
{
    // Preserve the previous graybox as a fallback; the new conservatory map owns its scene actors.
    if (!GetWorld()->GetMapName().Contains(TEXT("P0_Conservatory")))
    {
    auto* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto Block = [&](FVector Position, FVector Scale)
    {
        auto* A = GetWorld()->SpawnActor<AStaticMeshActor>(Position, FRotator::ZeroRotator);
        A->GetStaticMeshComponent()->SetStaticMesh(Cube); A->SetActorScale3D(Scale);
        A->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    };
    Block(FVector(0,0,-50), FVector(30,24,1));
    for (float X : {-1400.f,1400.f}) for (float Y : {-1100.f,0.f,1100.f}) Block(FVector(X,Y,300), FVector(.3,.3,6));
    for (float Y : {-1100.f,1100.f}) Block(FVector(0,Y,600), FVector(28,.3,.3));
    Block(FVector(650,0,40), FVector(2.5,2.5,.8));
    auto* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,800), FRotator(-50,-35,0));
    Sun->GetLightComponent()->SetIntensity(5);
    auto* Sky = GetWorld()->SpawnActor<ASkyLight>(); Sky->GetLightComponent()->SetIntensity(.7f);
    }
    Super::StartPlay();
    GetWorld()->SpawnActor<AVZAudioDirector>();
    UE_LOG(LogTemp, Display, TEXT("VZ_P0: native prototype world ready; production art pending"));
    StartVZCombatVerification(GetWorld());
    StartVZOrderVerification(GetWorld());
    extern void StartVZHeroVerification(UWorld* World);
    StartVZHeroVerification(GetWorld());
    extern void StartVZStoryVerification(UWorld* World);
    StartVZStoryVerification(GetWorld());
    extern void StartVZGardenVerification(UWorld* World);
    StartVZGardenVerification(GetWorld());
    extern void StartVZTravelVerification(UWorld* World);
    StartVZTravelVerification(GetWorld());
    extern void StartVZChuyaStoryVerification(UWorld* World);
    StartVZChuyaStoryVerification(GetWorld());
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(), TEXT("VZSmokeTest")))
    {
        FTimerHandle Handle;
        GetWorldTimerManager().SetTimer(Handle, [this]()
        {
            auto* P = Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
            bool Ok = P && P->GetActorLocation().Y > 100 && P->GetActorLocation().Z > 0 && P->Companions.Num() == 1;
            if (P)
            {
                auto* S = P->State();
                Ok &= P->Companions.Num() == 1 && FVector::Dist2D(P->GetActorLocation(),P->Companions[0]->GetActorLocation()) < 600;
                auto* CompanionMesh = P->Companions[0]->GetMesh();
                const bool RigOk = CompanionMesh->GetSkeletalMeshAsset() && CompanionMesh->GetBoneIndex(TEXT("leaf_L_1")) != INDEX_NONE && CompanionMesh->GetSingleNodeInstance() && CompanionMesh->GetSingleNodeInstance()->GetCurrentTime() > 0 && CompanionMesh->GetSkeletalMeshAsset()->GetLODNum() == 3;
                Ok &= RigOk;
                const bool MaterialsOk = CompanionMesh->GetMaterial(0) && CompanionMesh->GetMaterial(0)->GetName() == TEXT("M_Chuya") && P->RepairedFlowerMaterial && P->DormantFlowerMaterial;
                Ok &= MaterialsOk;
                UE_LOG(LogTemp,Display,TEXT("VZ_MATERIAL: %s companion and ecology materials cooked"),MaterialsOk?TEXT("PASS"):TEXT("FAIL"));
                if (GetWorld()->GetMapName().Contains(TEXT("P0_Conservatory")))
                {
                    auto* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(),FVector(-350,640,90),FVector(350,640,90));
                    const bool NavOk = NavPath && NavPath->IsValid() && !NavPath->IsPartial() && NavPath->PathPoints.Num() > 2;
                    Ok &= NavOk;
                    UE_LOG(LogTemp,Display,TEXT("VZ_NAV: %s obstacle detour points=%d"),NavOk?TEXT("PASS"):TEXT("FAIL"),NavPath?NavPath->PathPoints.Num():0);
                }
                UE_LOG(LogTemp,Display,TEXT("VZ_RIG: %s skeleton, animated playback, three LODs"),RigOk?TEXT("PASS"):TEXT("FAIL"));
                Ok &= S->SetCheckpoint(P->GetActorLocation());
                Ok &= S->RepairTestFlower();
                Ok &= !S->RepairTestFlower();
                for (int32 Slot = 1; Slot <= 3; ++Slot) Ok &= S->SaveSlot(Slot);
                Ok &= S->LoadSlot(1);
                Ok &= !S->RepairTestFlower();
                Ok &= S->State.Materials.FindRef(TEXT("research.sample")) == 3;
                P->RefreshEcology();
                P->SetActorLocation(FVector(0,0,500)); P->Respawn();
                Ok &= P->GetActorLocation().Equals(S->State.Checkpoint, 1);
                UE_LOG(LogTemp, Display, TEXT("VZ_SMOKE: %s movement, party follow, checkpoint, reward idempotency, three slots, reload, respawn"),Ok?TEXT("PASS"):TEXT("FAIL"));
            }
            FTimerHandle Photo;
            GetWorldTimerManager().SetTimer(Photo,[]() { FScreenshotRequest::RequestScreenshot(TEXT("VZ_P0_smoke.png"), true, false); },1.f,false);
            FTimerHandle Exit;
            GetWorldTimerManager().SetTimer(Exit, [Ok]() { FPlatformMisc::RequestExitWithStatus(false, Ok ? 0 : 1); }, 2.f, false);
        }, 3.f, false);
    }
#endif
}

void AVZPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* Player = Cast<AVZEcologist>(GetOwningPawn()); if (!Player || !Canvas) return;
    const auto* S = Player->State();
    const auto* VZPC=Cast<AVZPlayerController>(GetOwningPlayerController());
    const FVZInputSettings DefaultBindings;
    const auto& Keys=VZPC?VZPC->Bindings:DefaultBindings;
    const float W=Canvas->SizeX,H=Canvas->SizeY;
    const FLinearColor Ink(.02f,.06f,.045f,.87f),Jade(.52f,1.f,.78f),Paper(.94f,.94f,.83f);
    DrawRect(Ink,24,24,420,128);DrawRect(Jade,24,24,3,128);
    DrawText(TEXT("零号变种  ·  未完成的春天"),Paper,42,38,nullptr,1.6f);
    DrawText(Player->Garden->Enabled() && Player->GetActorLocation().Y>1800?TEXT("无访花庭  /  锁住的阳光"):TEXT("原初研究所  /  温室异常"),Jade,42,74,nullptr,1.05f);
    for(int32 I=0;I<Player->Companions.Num();++I)
    {
        const auto* C=Player->Companions[I].Get();if(!IsValid(C))continue;
        FString Name=C->SpeciesId;
        for(const auto& Individual:S->State.Companions)if(Individual.IndividualId==C->IndividualId){Name=Individual.Name;break;}
        const float CD=I==0?Player->CompanionSkillCooldown:I==1?Player->CompanionSkillCooldown2:Player->CompanionSkillCooldown3;
        const TCHAR* Action=I==0?TEXT("VZCompanionSkill"):I==1?TEXT("VZCompanionSkill2"):TEXT("VZCompanionSkill3");
        DrawRect(Ink,W-320,70+I*74,295,65);
        DrawText(FString::Printf(TEXT("%s  Lv.%d  %d / %d"),*Name,VZCompanionLevel(C),FMath::CeilToInt(C->Vital->Health),FMath::CeilToInt(C->Vital->MaxHealth)),Paper,W-305,80+I*74);
        DrawText(FString::Printf(TEXT("%s · %s · %.1fs"),*Keys.KeyName(Action),C->SpeciesId==TEXT("V-041")?TEXT("花粉抚慰"):TEXT("种子脉冲"),CD),Jade,W-305,105+I*74);
    }
    const bool Active=IsValid(Player->Disturbance) && Player->Disturbance->Active;
    DrawText(Player->Prologue->Enabled()?Player->Prologue->Objective():Active?TEXT("目标：协助初芽稳定失衡节点"):IsValid(Player->Disturbance)&&Player->Disturbance->Cleared?TEXT("异常已平息 · 回到花床照料生命"):TEXT("探索温室 · 激活节点开始协作练习"),Paper,42,108,nullptr,.95f);
    DrawText(Player->Prologue->Enabled()?TEXT("开场流程原型"):TEXT("P0 玩法试作"),Paper,W-175,30,nullptr,1.1f);
    if(Player->Prologue->Enabled() && !Player->Prologue->DialogueOpen())
    {
        if(VZPC && VZPC->HasMapMarker && !VZPC->IsMenuOpen())
        {
            FVector2D Point;
            const FVector Delta=VZPC->MapMarker-Player->GetActorLocation();
            if(!GetOwningPlayerController()->ProjectWorldLocationToScreen(VZPC->MapMarker+FVector(0,0,140),Point)||Point.X<90||Point.X>W-250||Point.Y<200||Point.Y>H-180)
            {
                const FVector Local=FRotationMatrix(FRotator(0,Player->GetControlRotation().Yaw,0)).GetTransposed().TransformVector(Delta);
                Point=FVector2D(W*.5f+FMath::Clamp(Local.Y*.25f,-W*.32f,W*.32f),Local.X>=0?205:H-195);
            }
            DrawText(FString::Printf(TEXT("◇ 导航：%s  %.0f 米"),*VZPC->MapMarkerLabel,Delta.Size2D()/100),FLinearColor(1,.76,.3),Point.X-60,Point.Y,nullptr,1.05f);
        }
        const auto Targets=Player->ChuyaStory->Enabled()?FVZChuyaStoryRules::Targets(S->State):Player->Garden->Enabled() && S->State.Repairs.Contains(TEXT("prologue.pollinated"))?FVZGardenRules::Targets(S->State):FVZPrologueRules::Targets(S->State);
        for(const auto& Target:Targets)
        {
            FVector2D Point;
            const FVector Delta=Target.Key-Player->GetActorLocation();
            if(!GetOwningPlayerController()->ProjectWorldLocationToScreen(Target.Key+FVector(0,0,80),Point) || Point.X<70 || Point.X>W-180 || Point.Y<175 || Point.Y>H-165)
            {
                const FVector Local=FRotationMatrix(FRotator(0,Player->GetControlRotation().Yaw,0)).GetTransposed().TransformVector(Delta);
                Point=FVector2D(W*.5f+FMath::Clamp(Local.Y*.25f,-W*.35f,W*.35f),Local.X>=0?175:H-170);
            }
            DrawText(FString::Printf(TEXT("◆ %s  %.0f 米"),*Target.Value,Delta.Size2D()/100),Jade,Point.X-45,Point.Y,nullptr,1.1f);
        }
    }
    DrawRect(Ink,24,H-137,W-48,113);
    DrawText(FString::Printf(TEXT("生态师  %d / 100   ·   %s"),FMath::CeilToInt(Player->Vital->Health),S->State.StoryDifficulty?TEXT("故事难度"):TEXT("标准难度")),Paper,42,H-123);
    DrawRect(FLinearColor(.08f,.12f,.09f,1),42,H-97,250,7);DrawRect(Jade,42,H-97,250*Player->Vital->Health/100,7);
    const float PartnerHealth=Player->Companions.Num()?Player->Companions[0]->Vital->Health:0;
    FString LeadName=TEXT("伙伴");
    if(Player->Companions.Num())for(const auto& C:S->State.Companions)if(C.IndividualId==Player->Companions[0]->IndividualId){LeadName=C.Name;break;}
    DrawText(FString::Printf(TEXT("%s  %d / %d%s"),*LeadName,FMath::CeilToInt(PartnerHealth),Player->Companions.Num()?FMath::CeilToInt(Player->Companions[0]->Vital->MaxHealth):0,PartnerHealth<=0?TEXT("  ·  战后恢复"):TEXT("")),Jade,42,H-80);
    DrawText(FString::Printf(TEXT("%s 脉冲 · %s 护盾 %.1fs · %s 技能 %.1fs · %s 落点 · %s 召回"),*Keys.KeyName(TEXT("VZPulse")),*Keys.KeyName(TEXT("VZShield")),Player->ShieldCooldown,*Keys.KeyName(TEXT("VZCompanionSkill")),Player->CompanionSkillCooldown,*Keys.KeyName(TEXT("VZOrder")),*Keys.KeyName(TEXT("VZRecall"))),Paper,330,H-122,nullptr,.95f);
    DrawText(S->LastMessage,Jade,330,H-92);
    DrawText(FString::Printf(TEXT("Esc 暂停 / 存档 / 按键设置 · %s 跳跃 · %s 闪避 · %s 标记 · %s 照料    |    槽位 %d"),*Keys.KeyName(TEXT("VZJump")),*Keys.KeyName(TEXT("VZDodge")),*Keys.KeyName(TEXT("VZScan")),*Keys.KeyName(TEXT("VZInteract")),Player->ManualSlot),Paper,42,H-51);
    if(Player->AimingOrder)
    {
        const FLinearColor AimColor=Player->OrderAimValid?Jade:FLinearColor(1,.3f,.2f);
        DrawLine(W*.5f-8,H*.5f,W*.5f+8,H*.5f,AimColor,2);
        DrawLine(W*.5f,H*.5f-8,W*.5f,H*.5f+8,AimColor,2);
        DrawText(TEXT("松开指令键提交落点 · 召回可取消"),Paper,W*.5f-115,H*.5f+25);
    }
    if (Active)
    {
        FVector2D Screen;
        if (GetOwningPlayerController()->ProjectWorldLocationToScreen(Player->Disturbance->GetActorLocation()+FVector(0,0,110),Screen))
        {
            FString NodeLabel=Player->Disturbance->Marked?TEXT("失衡节点 · 已标记"):TEXT("失衡节点");
            if(auto* Guardian=Cast<AVZGardenGuardian>(Player->Disturbance))
                NodeLabel=Guardian->ExposureSeconds>0?FString::Printf(TEXT("花庭守护者 · 阶段 %d · 破防 %.1f 秒"),Guardian->Phase,Guardian->ExposureSeconds):FString::Printf(TEXT("花庭守护者 · 阶段 %d · 防护中"),Guardian->Phase);
            DrawText(NodeLabel,Paper,Screen.X-75,Screen.Y-30);
            DrawRect(Ink,Screen.X-75,Screen.Y-7,150,6);
            DrawRect(FLinearColor(1.f,.58f,.25f),Screen.X-75,Screen.Y-7,150*Player->Disturbance->Vital->Health/180,6);
        }
    }
}
