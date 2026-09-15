#include "VZPrototype.h"
#include "VZPrologue.h"
#include "VZState.h"
#include "VZCombat.h"
#include "VZPlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

void StartVZStoryVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZStoryTest")))return;
    FTimerHandle Handle;
    World->GetTimerManager().SetTimer(Handle,[World]()
    {
        UWorld* const TestWorld=World; // Timer insertion can relocate the executing delegate; copy its capture before scheduling more timers.
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(TestWorld,0));
        bool Ok=P && P->Prologue->Enabled();
        if(P)
        {
            auto* S=P->State();auto* Story=P->Prologue.Get();auto* PC=Cast<AVZPlayerController>(P->GetController());
            P->StartEncounter();Ok &= !P->Disturbance->Active;
            // The real interaction path must leave state unchanged when persistence is protected.
            P->SetActorLocation(FVector(-650,-250,95));S->WritesBlocked=true;P->Interact();
            Ok &= FVZPrologueRules::Stage(S->State)==0 && !Story->DialogueOpen();
            S->WritesBlocked=false;P->Interact();Ok &= Story->DialogueOpen() && PC->IsMoveInputIgnored();
            PC->ToggleMenu();Ok &= !Story->DialogueOpen() && PC->IsMenuOpen();PC->CloseMenu();Ok &= !PC->IsMoveInputIgnored();
            P->SetActorLocation(FVector(650,0,95));P->Scan();Ok &= FVZPrologueRules::Stage(S->State)==2;Story->Dismiss();
            for(auto Pos:{FVector(980,620,95),FVector(-900,-720,95),FVector(0,900,95)}) {P->SetActorLocation(Pos);P->Interact();}
            Ok &= FVZPrologueRules::Stage(S->State)==3 && S->SaveSlot(1);
            const FGuid Parent=S->State.Companions[0].IndividualId;
            P->Respawn();Ok &= S->LoadSlot(1);P->Respawn();
            P->SetActorLocation(FVector(-650,350,95));P->Interact();
            Ok &= Story->DialogueOpen() && P->Companions.Num()==2 && S->State.Companions[0].IndividualId==Parent;
            Story->Dismiss();P->StartEncounter();Ok &= P->Disturbance->Active;
            P->Disturbance->Hit(1000,P);Story->TickComponent(.1f,LEVELTICK_All,nullptr);
            Ok &= FVZPrologueRules::Stage(S->State)==5;
            P->SetActorLocation(FVector(650,0,95));P->Interact();Story->Dismiss();
            Ok &= FVZPrologueRules::Stage(S->State)==6 && S->State.Materials.FindRef(TEXT("research.sample"))==2;
            Ok &= S->LoadSlot(0);P->Respawn();P->SetActorLocation(FVector(650,0,95));P->Interact();
            Ok &= S->State.Materials.FindRef(TEXT("research.sample"))==2;
            P->SetActorLocation(FVector(-650,-250,95));P->Interact();
            Ok &= FVZPrologueRules::Stage(S->State)==7 && Story->DialogueOpen();
            Story->Dismiss();P->SetActorLocation(FVector(-650,350,95));P->Interact();Story->Dismiss();
            Ok &= P->Companions.Num()==3 && S->State.Materials.FindRef(TEXT("research.sample"))==0;
            AVZCompanionProxy* Moth=nullptr;for(auto C:P->Companions)if(C->SpeciesId==TEXT("V-041"))Moth=C;
            Ok &= Moth && Moth->GetMesh()->GetSkeletalMeshAsset()->GetLODNum()==3 && Moth->GetMesh()->GetBoneIndex(TEXT("wing_fore_L"))!=INDEX_NONE && Moth->GetMesh()->GetMaterial(0)->GetName()==TEXT("M_Moth");
            if(Moth)
            {
                const auto MothId=Moth->IndividualId;
                P->SetActorLocation(FVector(450,0,95));Moth->SetActorLocation(FVector(650,0,40));P->Interact();
                P->SetActorLocation(FVector(-650,-250,95));Story->TickComponent(2.f,LEVELTICK_All,nullptr);
                Ok &= !S->State.Repairs.Contains(TEXT("prologue.pollinated"));
                P->SetActorLocation(FVector(450,0,95));P->Interact();Story->TickComponent(1.5f,LEVELTICK_All,nullptr);Story->Dismiss();
                Ok &= S->State.Repairs.Contains(TEXT("prologue.pollinated")) && S->LoadSlot(0);
                P->Respawn();Ok &= P->Companions.Num()==3 && S->State.Party.Contains(MothId);
                P->SetActorLocation(FVector(100,0,95));
                for(auto Ally:P->Companions)Ally->SetActorLocation(FVector(250,-110,40));
                P->Vital->Health=55;P->Companions[0]->Vital->Health=65;P->Companions[1]->Vital->Health=0;
                P->CompanionSkill3();
                Ok &= P->Vital->Health==75 && P->Companions[0]->Vital->Health==85 && P->Companions[1]->Vital->IsDown() && P->CompanionSkillCooldown3==10;
                P->CompanionSkill3();Ok &= P->Vital->Health==75;
                P->Vital->Restore();for(auto Ally:P->Companions)Ally->Vital->Restore();P->CompanionSkillCooldown3=0;
                P->CompanionSkill3();Ok &= P->CompanionSkillCooldown3==0;
                P->Disturbance->Start(P);
                for(auto Ally:P->Companions)Ally->SetActorLocation(P->Disturbance->GetActorLocation()+FVector(-160,0,0));
                P->CompanionSkill();P->CompanionSkill2();
                Ok &= P->CompanionSkillCooldown==6 && P->CompanionSkillCooldown2==6 && P->CompanionSkillCooldown3==0 && P->Disturbance->Vital->Health==116;
                P->Disturbance->ResetEncounter();
                UE_LOG(LogTemp,Display,TEXT("VZ_SQUAD: %s independent skills and cooldowns; bounded healing; no resurrection; no cost at full health"),Ok?TEXT("PASS"):TEXT("FAIL"));
                for(auto C:P->Companions)if(C->SpeciesId==TEXT("V-041"))
                {
                    C->SetActorLocation(FVector(250,-110,40));C->FormationOffset=FVector(150,-110,0);
                    FTimerHandle A;TestWorld->GetTimerManager().SetTimer(A,[TestWorld,C]()
                    {
                        const auto Before=C->GetMesh()->GetSocketTransform(TEXT("wing_fore_L"),RTS_Component);
                        FTimerHandle B;TestWorld->GetTimerManager().SetTimer(B,[C,Before]()
                        {
                            const auto After=C->GetMesh()->GetSocketTransform(TEXT("wing_fore_L"),RTS_Component);
                            const bool Animated=C->GetMesh()->GetSingleNodeInstance() && !Before.Equals(After,.0001f);
                            UE_LOG(LogTemp,Display,TEXT("VZ_MOTH_ANIMATION: %s evaluated wing bone motion"),Animated?TEXT("PASS"):TEXT("FAIL"));
                            if(!Animated)FPlatformMisc::RequestExitWithStatus(false,1);
                        },.3f,false);
                    },.1f,false);
                    auto* Cam=TestWorld->SpawnActor<ACameraActor>(FVector(410,-380,230),FRotator::ZeroRotator);
                    Cam->SetActorRotation((C->GetMesh()->GetComponentLocation()-Cam->GetActorLocation()).Rotation());Cam->GetCameraComponent()->SetFieldOfView(45);PC->SetViewTarget(Cam);
                }
            }
            UE_LOG(LogTemp,Display,TEXT("VZ_MOTH: %s skeletal mesh, material, three LODs, research cost, party, nearby pollination and reload"),Ok?TEXT("PASS"):TEXT("FAIL"));
            UE_LOG(LogTemp,Display,TEXT("VZ_STORY: %s persisted interactions, blocked write, dialogue and pause, cultivation, parent identity, death/reload, reward idempotency, journal"),Ok?TEXT("PASS"):TEXT("FAIL"));
        }
        FTimerHandle Photo;TestWorld->GetTimerManager().SetTimer(Photo,[](){FScreenshotRequest::RequestScreenshot(TEXT("VZ_Prologue_Journal.png"),true,false);},1.f,false);
        FTimerHandle End;TestWorld->GetTimerManager().SetTimer(End,[Ok](){FPlatformMisc::RequestExitWithStatus(false,Ok?0:1);},2.f,false);
    },3.f,false);
#endif
}

