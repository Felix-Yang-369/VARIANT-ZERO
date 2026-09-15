#include "VZPrototype.h"
#include "VZHeroAnimation.h"
#include "VZCombat.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

void StartVZHeroVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZHeroTest"))) return;
    auto Passed=MakeShared<bool>(true);
    auto Hand=MakeShared<FTransform>();
    auto Later=[World](float Delay,TFunction<void()> Action){FTimerHandle H;World->GetTimerManager().SetTimer(H,MoveTemp(Action),Delay,false);};
    auto Player=[World](){return Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));};
    Later(1.f,[Player,Passed,Hand]()
    {
        auto* P=Player();if(!P){*Passed=false;return;}
        auto* Anim=Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance());
        *Passed &= Anim && P->GetMesh()->GetSkeletalMeshAsset()->GetName()==TEXT("Belica") && P->GetMesh()->GetBoneIndex(TEXT("hand_r"))!=INDEX_NONE;
        *Passed &= P->Staff->GetStaticMesh() && P->Staff->GetStaticMesh()->GetName()==TEXT("SM_FieldScanner") && P->GetMesh()->GetMaterial(0) && P->GetMesh()->GetMaterial(0)->GetName()==TEXT("MI_EcologistUpper");
        if(Anim) for(const auto& Clip:Anim->Clips) *Passed &= Clip && Clip->GetSkeleton()==P->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
        if(Anim) for(const auto& Clip:Anim->ActionClips) *Passed &= Clip && Clip->GetSkeleton()==P->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
        *Passed &= P->GetMesh()->GetBoneIndex(TEXT("spine_01"))!=INDEX_NONE;
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO_ASSETS: %s"),*Passed?TEXT("PASS"):TEXT("FAIL"));
        *Hand=P->GetMesh()->GetSocketTransform(TEXT("hand_r"),RTS_Component);
        P->SetActorLocation(FVector(-700,-700,95));P->GetCharacterMovement()->Velocity=FVector(420,0,0);
    });
    Later(1.7f,[Player,Passed]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        *Passed &= A && A->LocomotionState==1;
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO_WALK: %s state=%d speed=%.1f"),*Passed?TEXT("PASS"):TEXT("FAIL"),A?A->LocomotionState:-1,A?A->GroundSpeed:0);
        if(P)P->Jump();
    });
    Later(1.95f,[Player,Passed,Hand]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        *Passed &= A && A->LocomotionState>=2 && !P->GetMesh()->GetSocketTransform(TEXT("hand_r"),RTS_Component).Equals(*Hand,.001f);
        const float GripError=P?FVector::Distance(P->Staff->GetComponentLocation(),P->GetMesh()->GetSocketLocation(TEXT("hand_r"))):MAX_flt;
        const bool GripOk=P && P->Staff->GetAttachSocketName()==TEXT("hand_r") && GripError<2.f && P->GetMesh()->IsBoneHiddenByName(TEXT("weapon"));
        *Passed &= GripOk;
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO_GRIP: %s airborne socket error=%.3f cm; source firearm hidden"),GripOk?TEXT("PASS"):TEXT("FAIL"),GripError);
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO_JUMP: %s animated bone pose and airborne state"),*Passed?TEXT("PASS"):TEXT("FAIL"));
        if(P)P->StopJumping();
    });
    Later(4.f,[Player,Passed]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        *Passed &= A && A->LocomotionState==0 && P->GetMesh()->GetSkeletalMeshAsset()->GetLODNum()>1;
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO_LANDING: %s"),*Passed?TEXT("PASS"):TEXT("FAIL"));
    });
    Later(4.1f,[Player,Passed,Hand]()
    {
        auto* P=Player();if(!P||!P->Disturbance){*Passed=false;return;}
        P->SetActorLocation(P->Disturbance->GetActorLocation()+FVector(-500,0,0));
        P->GetCharacterMovement()->StopMovementImmediately();
        *Hand=P->GetMesh()->GetSocketTransform(TEXT("hand_r"),RTS_Component);
        P->Disturbance->Start(P);P->Disturbance->Marked=true;P->PulseCooldown=0;P->FirePulse();
    });
    Later(4.27f,[Player,Passed,Hand]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        const bool Ok=A&&A->ActiveAction==0&&A->ActionWeight>.5f&&!P->GetMesh()->GetSocketTransform(TEXT("hand_r"),RTS_Component).Equals(*Hand,.001f);
        *Passed &= Ok;UE_LOG(LogTemp,Display,TEXT("VZ_HERO_PULSE: %s actual action and changed hand pose"),Ok?TEXT("PASS"):TEXT("FAIL"));
        if(P){P->ShieldCooldown=0;P->Shield();P->GetCharacterMovement()->Velocity=FVector(200,0,0);}
    });
    Later(4.45f,[Player,Passed]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        const bool Ok=A&&A->ActiveAction==1&&P->Vital->ShieldSeconds>2&&P->GetVelocity().Size2D()>0
            &&P->ShieldVisual->IsVisible()&&P->ShieldVisual->GetMaterial(0)
            &&P->ShieldVisual->GetMaterial(0)->GetName()==TEXT("M_EcologicalShield")
            &&P->ShieldVisual->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
        *Passed &= Ok;UE_LOG(LogTemp,Display,TEXT("VZ_HERO_SHIELD: %s casting retains movement"),Ok?TEXT("PASS"):TEXT("FAIL"));
        FScreenshotRequest::RequestScreenshot(TEXT("VariantZero_HeroActions.png"),true,false);
        if(P){P->Vital->InvulnerableSeconds=0;P->Vital->ReceiveDamage(5);}
    });
    Later(4.63f,[Player,Passed]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        bool Ok=A&&A->ActiveAction==2&&A->ActionWeight>.5f;
        if(A)
        {
            const int32 Serial=A->ActionSerial;A->RequestAction(0);
            P->Vital->InvulnerableSeconds=1;P->Vital->ReceiveDamage(5);
            Ok &= Serial==A->ActionSerial;
        }
        *Passed &= Ok;UE_LOG(LogTemp,Display,TEXT("VZ_HERO_HIT: %s hit priority and invulnerability"),Ok?TEXT("PASS"):TEXT("FAIL"));
        if(P){P->Disturbance->ResetEncounter();P->GetCharacterMovement()->StopMovementImmediately();}
    });
    Later(5.4f,[Player,Passed]()
    {
        auto* P=Player();auto* A=P?Cast<UVZHeroAnimation>(P->GetMesh()->GetAnimInstance()):nullptr;
        bool Ok=A&&A->ActiveAction==-1&&A->ActionWeight==0;
        if(A){A->RequestAction(1);P->Respawn();Ok &= A->RequestedAction==-1&&A->ActionWeight==0&&!P->ShieldVisual->IsVisible();}
        *Passed &= Ok;UE_LOG(LogTemp,Display,TEXT("VZ_HERO_ACTION_RESET: %s expiry and checkpoint reset"),Ok?TEXT("PASS"):TEXT("FAIL"));
    });
    Later(5.8f,[Passed]()
    {
        UE_LOG(LogTemp,Display,TEXT("VZ_HERO: %s locomotion, grip, pulse, shield, additive hit, movement, priority and reset"),*Passed?TEXT("PASS"):TEXT("FAIL"));
        FPlatformMisc::RequestExitWithStatus(false,*Passed?0:1);
    });
#endif
}
