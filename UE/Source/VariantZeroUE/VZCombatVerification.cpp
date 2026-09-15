#include "VZCombatVerification.h"
#include "VZPrototype.h"
#include "VZCombat.h"
#include "VZState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Engine/StaticMeshActor.h"

void StartVZCombatVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(),TEXT("VZCombatTest")))
    {
        auto Passed=MakeShared<bool>(true);
        auto AfterInput=MakeShared<float>(180.f);
        FTimerHandle Begin;
        World->GetTimerManager().SetTimer(Begin,[World,Passed,AfterInput]()
        {
            auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));
            if (!P || !IsValid(P->Disturbance)) { *Passed=false;return; }
            auto* Occluded=World->SpawnActor<AStaticMeshActor>(FVector(350,640,90),FRotator::ZeroRotator);
            const bool WallBlocks=!AVZDisturbance::HasClearShot(P,Occluded,FVector(-350,640,90));
            *Passed &= WallBlocks;Occluded->Destroy();
            if (P->Companions.Num()) P->Companions[0]->SetActorLocation(FVector(100,-100,40));
            P->StartEncounter();P->Scan();const float Before=P->Disturbance->Vital->Health;
            P->FirePulse();const float First=P->Disturbance->Vital->Health;
            *Passed &= FMath::IsNearlyEqual(Before-First,12.f);P->FirePulse();*Passed &= P->Disturbance->Vital->Health==First;
            P->CompanionSkill();const float Skill=P->Disturbance->Vital->Health;
            *Passed &= FMath::IsNearlyEqual(First-Skill,32.f);P->CompanionSkill();*Passed &= P->Disturbance->Vital->Health==Skill;
            *AfterInput=Skill;
            P->Shield();const float Health=P->Vital->Health;P->Vital->ReceiveDamage(50);
            *Passed &= FMath::IsNearlyEqual(Health-P->Vital->Health,10.f);
            P->Vital->InvulnerableSeconds=.3f;const float Protected=P->Vital->Health;P->Vital->ReceiveDamage(100);
            *Passed &= P->Vital->Health==Protected;
            P->Vital->Restore();P->StartPulse();
            UE_LOG(LogTemp,Display,TEXT("VZ_COMBAT_INPUT: %s pulse, cooldown, companion skill, shield, invulnerability"),*Passed?TEXT("PASS"):TEXT("FAIL"));
            UE_LOG(LogTemp,Display,TEXT("VZ_OCCLUSION: %s wall blocks attacks"),WallBlocks?TEXT("PASS"):TEXT("FAIL"));
        },.8f,false);
        FTimerHandle Auto;
        World->GetTimerManager().SetTimer(Auto,[World,Passed,AfterInput]()
        {
            auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));
            const bool Attacked=P && P->Disturbance->Vital->Health<*AfterInput;
            *Passed &= Attacked;
            if (P && P->Companions.Num()) P->Companions[0]->Vital->Health=0;
            UE_LOG(LogTemp,Display,TEXT("VZ_AUTO_ATTACK: %s companion attacks without command"),Attacked?TEXT("PASS"):TEXT("FAIL"));
        },1.2f,false);
        FTimerHandle Finish;
        World->GetTimerManager().SetTimer(Finish,[World,Passed]()
        {
            auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));
            if (!P) *Passed=false;
            else
            {
                P->StopPulse();*Passed &= P->Disturbance->Cleared && !P->Disturbance->Active;
                *Passed &= P->Vital->Health>0 && P->Vital->Health<100;
                *Passed &= P->Companions.Num() && !P->Companions[0]->Vital->IsDown();
                *Passed &= P->State()->State.Materials.FindRef(TEXT("research.sample"))==0;
                const FGuid Identity=P->State()->State.Party[0];
                P->Vital->Health=1;P->Vital->ReceiveDamage(500);P->Tick(.01f);
                *Passed &= P->Vital->Health==P->Vital->MaxHealth && P->State()->State.Party[0]==Identity && !P->Disturbance->Active;
            }
            UE_LOG(LogTemp,Display,TEXT("VZ_COMBAT: %s encounter completion, no farm rewards, death recovery, companion identity"),*Passed?TEXT("PASS"):TEXT("FAIL"));
            FPlatformMisc::RequestExitWithStatus(false,*Passed?0:1);
        },10.f,false);
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("VZPreview")))
    {
        FTimerHandle Setup;
        World->GetTimerManager().SetTimer(Setup,[World]()
        {
            auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));if (!P) return;
            P->SetActorLocation(FVector(-200,-130,95));P->SetActorRotation(FRotator(0,-23,0));
            if (P->Companions.Num()) P->Companions[0]->SetActorLocation(FVector(70,-100,40));
            auto* Camera=World->SpawnActor<ACameraActor>(FVector(-850,-1200,650),FRotator::ZeroRotator);
            Camera->SetActorRotation((FVector(100,-150,200)-Camera->GetActorLocation()).Rotation());Camera->GetCameraComponent()->SetFieldOfView(65);
            if (FParse::Param(FCommandLine::Get(),TEXT("VZEcologyPreview")))
            {
                P->SetActorRotation(FRotator(0,-130,0));
                Camera->SetActorLocation(FVector(-740,-850,330));
                Camera->SetActorRotation((FVector(-40,-30,150)-Camera->GetActorLocation()).Rotation());
                Camera->GetCameraComponent()->SetFieldOfView(55);
            }
            if (auto* PC=Cast<APlayerController>(P->GetController())) PC->SetViewTarget(Camera);
            P->StartEncounter();P->Scan();
        },1.f,false);
        FTimerHandle Picture;
        World->GetTimerManager().SetTimer(Picture,[World]()
        {
            auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));if (!P) return;
            P->Shield();P->FirePulse();P->CompanionSkill();
            FScreenshotRequest::RequestScreenshot(TEXT("VariantZero_CombatPreview.png"),true,false);
        },3.7f,false);
        FTimerHandle Exit;
        World->GetTimerManager().SetTimer(Exit,[](){FPlatformMisc::RequestExitWithStatus(false,0);},5.f,false);
    }
#endif
}
