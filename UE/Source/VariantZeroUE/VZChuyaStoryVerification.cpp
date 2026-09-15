#include "VZChuyaStory.h"
#include "VZPrototype.h"
#include "VZGarden.h"
#include "VZPrologue.h"
#include "VZTravel.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "UnrealClient.h"

void StartVZChuyaStoryVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZChuyaStoryTest")))return;
    FTimerHandle H;World->GetTimerManager().SetTimer(H,[World]()
    {
        UWorld* const W=World;bool Ok=true;auto Check=[&Ok](const TCHAR* Label,bool Value){Ok &= Value;if(!Value)UE_LOG(LogTemp,Error,TEXT("VZ_CHUYA_CHECK: %s"),Label);};
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(W,0));Check(TEXT("player"),P!=nullptr);
        if(P)
        {
            auto* S=P->State();auto C=S->State;C.QuestStages.Add(TEXT("main.prologue"),7);C.QuestStages.Add(TEXT("main.garden"),3);C.Repairs.Add(TEXT("prologue.pollinated"));C.Repairs.Add(TEXT("garden.waterway"));C.Repairs.Add(TEXT("garden.shortcut"));
            C.Companions[0].Name=TEXT("改名后的初芽");Check(TEXT("setup"),S->Commit(C));P->Respawn();
            auto Stand=[P](FVector Pos){P->SetActorLocation(Pos);for(auto Partner:P->Companions)Partner->SetActorLocation(Pos+FVector(80,0,-50));};
            Stand(FVector(0,2500,90));P->Companions[0]->Vital->Health=0;P->Interact();Check(TEXT("down partner cannot begin"),FVZChuyaStoryRules::Stage(S->State)==0);
            P->Companions[0]->Vital->Restore();P->Interact();P->Prologue->Dismiss();Check(TEXT("begin"),FVZChuyaStoryRules::Stage(S->State)==1);
            const FVector Sites[]={FVector(-1000,4800,90),FVector(-1100,6100,90),FVector(1050,5900,90)};
            for(int32 I=0;I<3;++I)
            {
                auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(0,2200,90),Sites[I]);Check(TEXT("observation reachable"),Path && Path->IsValid() && !Path->IsPartial());
                Stand(Sites[I]);
                if(I==0)
                {
                    P->Companions[0]->SetActorLocation(Sites[I]+FVector(900,0,0));P->Scan();
                    Check(TEXT("distant partner cannot observe"),!S->State.Repairs.Contains(TEXT("story.chuya.observation.0")));Stand(Sites[I]);
                    P->Disturbance->Start(P);P->Scan();Check(TEXT("combat cannot advance observation"),!S->State.Repairs.Contains(TEXT("story.chuya.observation.0")));P->Disturbance->ResetEncounter();
                }
                P->Scan();P->Prologue->Dismiss();
                if(I==0){Check(TEXT("partial save"),S->SaveSlot(1));P->Respawn();Check(TEXT("partial load"),S->LoadSlot(1));P->Respawn();}
            }
            Check(TEXT("all observations"),FVZChuyaStoryRules::Stage(S->State)==2);Check(TEXT("choice checkpoint"),S->SaveSlot(2));
            Stand(FVector(-450,3000,90));S->WritesBlocked=true;P->Interact();Check(TEXT("failed commit retains choice"),FVZChuyaStoryRules::Stage(S->State)==2 && S->State.Materials.FindRef(TEXT("research.sample"))==0);S->WritesBlocked=false;
            P->Interact();P->Prologue->Dismiss();Check(TEXT("shallow result"),S->State.Repairs.Contains(TEXT("story.chuya.shallow")) && S->State.Materials.FindRef(TEXT("research.sample"))==1);
            Check(TEXT("reward restores"),S->LoadSlot(0));P->Respawn();int32 Wet=0;for(TActorIterator<AStaticMeshActor> It(W);It;++It)if(It->ActorHasTag(TEXT("VZChuyaShallow")))++Wet;Check(TEXT("wet visual restores"),Wet==6);
            Stand(FVector(-450,3000,90));P->Interact();P->Prologue->Dismiss();Check(TEXT("no duplicate reward"),S->State.Materials.FindRef(TEXT("research.sample"))==1);
            Check(TEXT("restore pre-choice slot"),S->LoadSlot(2));P->Respawn();Stand(FVector(450,3000,90));P->Interact();P->Prologue->Dismiss();
            Check(TEXT("dry exclusive result"),S->State.Repairs.Contains(TEXT("story.chuya.dry")) && !S->State.Repairs.Contains(TEXT("story.chuya.shallow")) && S->State.Materials.FindRef(TEXT("research.sample"))==1);
            int32 Dry=0;Wet=0;for(TActorIterator<AStaticMeshActor> It(W);It;++It){if(It->ActorHasTag(TEXT("VZChuyaDry")))++Dry;if(It->ActorHasTag(TEXT("VZChuyaShallow")))++Wet;}Check(TEXT("choice replaces visuals"),Dry==6 && Wet==0);
            Check(TEXT("journal contains choice"),FVZTravelRules::Record(S->State,2).Contains(TEXT("保留了干燥岛")));
            auto* Cam=W->SpawnActor<ACameraActor>(FVector(1000,2300,650),FRotator::ZeroRotator);Cam->SetActorRotation((FVector(0,3100,70)-Cam->GetActorLocation()).Rotation());Cast<APlayerController>(P->GetController())->SetViewTarget(Cam);
        }
        UE_LOG(LogTemp,Display,TEXT("VZ_CHUYA_STORY: %s partner readiness, navigation, partial save, both choices, failed commit, reward and world visuals"),Ok?TEXT("PASS"):TEXT("FAIL"));
        FTimerHandle Photo;W->GetTimerManager().SetTimer(Photo,[](){FScreenshotRequest::RequestScreenshot(TEXT("VZ_ChuyaStory.png"),true,false);},1.f,false);
        FTimerHandle End;W->GetTimerManager().SetTimer(End,[Ok](){FPlatformMisc::RequestExitWithStatus(false,Ok?0:1);},2.f,false);
    },3.f,false);
#endif
}
