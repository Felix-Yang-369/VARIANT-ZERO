#include "VZGarden.h"
#include "VZPrototype.h"
#include "VZPrologue.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "UnrealClient.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

void StartVZGardenVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZGardenTest")))return;
    FTimerHandle H;World->GetTimerManager().SetTimer(H,[World]()
    {
        UWorld* const W=World;auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(W,0));bool Ok=P && P->Garden->Enabled();
        UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK enabled %d"),Ok);
        if(P && Ok)
        {
            auto* S=P->State();auto C=S->State;C.Repairs.Add(TEXT("prologue.pollinated"));C.QuestStages.Add(TEXT("main.prologue"),7);
            FVZCompanion M;M.IndividualId=FGuid::NewGuid();M.SpeciesId=TEXT("V-041");M.Name=TEXT("菌翼蛾");C.Companions.Add(M);C.Party.Add(M.IndividualId);Ok &= S->Commit(C);P->RebuildParty();
            P->SetActorLocation(FVector(0,1100,100));S->WritesBlocked=true;P->Interact();Ok &= FVZGardenRules::Stage(S->State)==0;S->WritesBlocked=false;P->Interact();P->Prologue->Dismiss();
            Ok &= FVZGardenRules::Stage(S->State)==1;UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK enter %d stage %d"),Ok,FVZGardenRules::Stage(S->State));
            auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(0,2200,90),FVector(0,6500,90));Ok &= Path && Path->IsValid() && !Path->IsPartial();UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK nav %d valid %d partial %d"),Ok,Path?Path->IsValid():false,Path?Path->IsPartial():true);
            const FVector Positions[]={FVector(-750,4250,90),FVector(750,4250,90),FVector(0,6500,90)};
            P->SetActorLocation(Positions[0]);P->Interact();Ok &= S->SaveSlot(1);P->Respawn();Ok &= S->LoadSlot(1);P->Respawn();
            for(int32 I=1;I<3;++I){P->SetActorLocation(Positions[I]);for(int32 N=0;N<(I==1?3:2);++N)P->Interact();}
            Ok &= FVZGardenRules::Aligned(S->State);UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK alignment %d"),Ok);
            auto* G=P->Garden->Guardian.Get();P->SetActorLocation(FVector(0,5100,90));P->Garden->StartEncounter();Ok &= !G->Hit(100,P) && G->Vital->Health==180;
            int32 ImportedLeaves=0;TInlineComponentArray<UStaticMeshComponent*> MeshParts;G->GetComponents(MeshParts);
            for(auto* Part:MeshParts)if(Part->GetStaticMesh() && Part->GetStaticMesh()->GetName()==TEXT("SM_GuardianLeaf"))++ImportedLeaves;
            Ok &= ImportedLeaves==6;
            for(auto Partner:P->Companions)Partner->SetActorLocation(FVector(0,5100,40));P->Interact();Ok &= G->ExposureSeconds==8 && G->Hit(60,P);
            UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK shield %d"),Ok);P->Respawn();Ok &= !G->Active && G->ExposureSeconds==0 && FVZGardenRules::Aligned(S->State);
            P->SetActorLocation(FVector(0,5100,90));for(auto Partner:P->Companions)Partner->SetActorLocation(FVector(0,5100,40));P->Interact();Ok &= G->Hit(180,P);
            Ok &= G->Phase==2 && G->Vital->Health==90 && !G->Cleared && G->ExposureSeconds==0 && !G->Hit(1,P);
            P->SetActorLocation(FVector(200,5100,90));P->Interact();G->ExposureSeconds=3;P->Interact();Ok &= G->ExposureSeconds==3;
            G->AttackClock=0;G->Tick(.01f);Ok &= G->WindingUp && G->WarningCenters.Num()==3;
            P->SetActorLocation(FVector(200,5550,90));G->Tick(1.41f);Ok &= P->Vital->Health==100 && !G->WindingUp;
            P->SetActorLocation(FVector(200,5100,90));G->Tick(3.1f);G->Tick(1.5f);Ok &= P->Vital->Health==82;
            P->Vital->Restore();S->State.StoryDifficulty=false;G->Tick(3.1f);G->Tick(1.5f);Ok &= P->Vital->Health==70;
            P->Vital->Restore();P->Vital->ShieldSeconds=3;G->Tick(2.3f);G->Tick(1.5f);Ok &= P->Vital->Health==94;
            S->State.StoryDifficulty=true;
            P->Vital->Restore();P->Interact();Ok &= G->ExposureSeconds==8;
            P->Scan();P->PulseCooldown=0;const float BeforePulse=G->Vital->Health;P->FirePulse();Ok &= G->Vital->Health==BeforePulse-12;
            Ok &= G->Hit(180,P) && G->Cleared;
            UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK phase2, no refresh, committed telegraph, dodge, damage and actual pulse %d"),Ok);
            S->WritesBlocked=true;P->Garden->TickComponent(.1f,LEVELTICK_All,nullptr);Ok &= FVZGardenRules::Stage(S->State)==1;UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK enter %d stage %d"),Ok,FVZGardenRules::Stage(S->State));
            S->WritesBlocked=false;P->Garden->TickComponent(.1f,LEVELTICK_All,nullptr);Ok &= FVZGardenRules::Stage(S->State)==2 && S->State.Materials.FindRef(TEXT("research.sample"))==4;
            UE_LOG(LogTemp,Display,TEXT("GARDEN_CHECK reward %d stage %d"),Ok,FVZGardenRules::Stage(S->State));Ok &= S->LoadSlot(0);P->Respawn();for(TActorIterator<AStaticMeshActor> It(W);It;++It)if(It->ActorHasTag(TEXT("VZGardenShortcut")))Ok &= It->IsHidden() && !It->GetActorEnableCollision();
            Ok &= G->Cleared && G->Vital->IsDown() && !G->Active;
            P->SetActorLocation(FVector(0,2500,90));P->Interact();P->Prologue->Dismiss();Ok &= FVZGardenRules::Stage(S->State)==3;
            P->SetActorLocation(FVector(370,5200,90));P->RebuildParty();
            auto* Cam=W->SpawnActor<ACameraActor>(FVector(900,4550,650),FRotator::ZeroRotator);Cam->SetActorRotation((FVector(0,5550,150)-Cam->GetActorLocation()).Rotation());Cast<APlayerController>(P->GetController())->SetViewTarget(Cam);
        }
        UE_LOG(LogTemp,Display,TEXT("VZ_GARDEN: %s real navigation, puzzle save/resume, shield gating, death retry, failed write protection, reward, shortcut and report"),Ok?TEXT("PASS"):TEXT("FAIL"));
        FTimerHandle Photo;W->GetTimerManager().SetTimer(Photo,[](){FScreenshotRequest::RequestScreenshot(TEXT("VZ_Guardian.png"),true,false);},1.f,false);
        FTimerHandle End;W->GetTimerManager().SetTimer(End,[Ok](){FPlatformMisc::RequestExitWithStatus(false,Ok?0:1);},2.f,false);
    },3.f,false);
#endif
}
