#include "VZTravel.h"
#include "VZGarden.h"
#include "VZPrototype.h"
#include "VZPrologue.h"
#include "VZPlayerController.h"
#include "VZMap.h"
#include "Widgets/SWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "Containers/Ticker.h"
#include "UnrealClient.h"

void StartVZTravelVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZTravelTest")))return;
    FTimerHandle H;World->GetTimerManager().SetTimer(H,[World]()
    {
        UWorld* const W=World;bool Ok=true;
        auto Check=[&Ok](const TCHAR* Label,bool Result){Ok &= Result;if(!Result)UE_LOG(LogTemp,Error,TEXT("VZ_TRAVEL_CHECK: %s"),Label);};
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(W,0));auto* PC=P?Cast<AVZPlayerController>(P->GetController()):nullptr;
        Check(TEXT("player ready"),P && PC && P->Garden->Enabled());
        if(P && PC && P->Garden->Enabled())
        {
            auto* S=P->State();PC->ToggleMenu();PC->ActivateMenuRow(9);
            Check(TEXT("archive opens paused"),PC->ArchivePage && UGameplayStatics::IsGamePaused(W));
            PC->ActivateMenuRow(8);
            Check(TEXT("UMG map opens while paused"),PC->MapWidget&&PC->MapWidget->IsInViewport()&&UGameplayStatics::IsGamePaused(W));
            if(PC->MapWidget)
            {
                auto* Map=PC->MapWidget.Get();
                Check(TEXT("new map hides locked destination"),!Map->Points.ContainsByPredicate([](const FVZMapPoint& T){return T.TravelId==TEXT("garden");}));
                Map->TakeWidget()->OnKeyDown(FGeometry(),FKeyEvent(EKeys::Gamepad_DPad_Right,FModifierKeysState(),0,false,0,0));
                Check(TEXT("controller selects next map target"),Map->SelectedPoint==1);
                Map->TakeWidget()->OnKeyDown(FGeometry(),FKeyEvent(EKeys::Gamepad_FaceButton_Bottom,FModifierKeysState(),0,false,0,0));
                Check(TEXT("controller sets navigation marker"),PC->HasMapMarker&&PC->MapMarker.Equals(Map->Points[1].Position));
                Map->Activate(4);Check(TEXT("objective cannot teleport"),PC->MapWidget&&PC->ArchiveText.Contains(TEXT("步行")));
                Map->Activate(3);Check(TEXT("clear marker"),!PC->HasMapMarker);
                Map->Activate(2);PC->MenuBack();
                Check(TEXT("map back restores archive and keeps marker"),!PC->MapWidget&&PC->ArchivePage&&PC->IsMenuOpen()&&PC->HasMapMarker&&UGameplayStatics::IsGamePaused(W));
            }
            Check(TEXT("locked destination blocked"),!PC->TravelTo(TEXT("garden")));
            Check(TEXT("records remain accessible"),PC->IsMenuOpen());
            auto C=S->State;C.QuestStages.Add(TEXT("main.prologue"),7);C.QuestStages.Add(TEXT("main.garden"),1);C.Repairs.Add(TEXT("prologue.pollinated"));C.Materials.Add(TEXT("research.sample"),4);
            FVZCompanion M;M.IndividualId=FGuid::NewGuid();M.SpeciesId=TEXT("V-041");M.Name=TEXT("菌翼蛾");C.Companions.Add(M);C.Party.Add(M.IndividualId);
            Check(TEXT("setup"),S->Commit(C));P->RebuildParty();const auto Ids=S->State.Party;const FVector Before=P->GetActorLocation();
            PC->OpenMap();
            Check(TEXT("map refresh sees newly unlocked garden"),PC->MapWidget&&PC->MapWidget->Points.Num()>1&&PC->MapWidget->Points[1].TravelId==TEXT("garden"));
            if(PC->MapWidget)
            {
                PC->MapWidget->SelectedPoint=1;S->WritesBlocked=true;PC->MapWidget->Activate(4);
                Check(TEXT("map travel save failure stays on map"),PC->MapWidget&&P->GetActorLocation().Equals(Before));S->WritesBlocked=false;PC->CloseMap();
            }
            S->WritesBlocked=true;Check(TEXT("failed save blocks travel"),!PC->TravelTo(TEXT("garden")));Check(TEXT("no movement on write failure"),P->GetActorLocation().Equals(Before));S->WritesBlocked=false;
            P->Disturbance->Start(P);Check(TEXT("combat blocks travel"),!PC->TravelTo(TEXT("garden")));P->Disturbance->ResetEncounter();
            P->Garden->Guardian->Cleared=true;Check(TEXT("pending repair cannot be discarded by travel"),!PC->TravelTo(TEXT("institute")));P->Garden->Guardian->Cleared=false;
            P->Vital->Health=0;Check(TEXT("down state blocks travel"),!PC->TravelTo(TEXT("garden")));P->Vital->Restore();
            Check(TEXT("travel succeeds"),PC->TravelTo(TEXT("garden")));
            Check(TEXT("travel clears stale navigation marker"),!PC->HasMapMarker&&!PC->MapWidget);
            Check(TEXT("resumed at garden checkpoint"),!PC->IsMenuOpen() && !UGameplayStatics::IsGamePaused(W) && P->GetActorLocation().Equals(FVector(0,2200,120)));
            Check(TEXT("all companions rebuilt"),P->Companions.Num()==Ids.Num() && S->State.Party==Ids);
            for(auto Partner:P->Companions)Check(TEXT("companion present near destination"),Ids.Contains(Partner->IndividualId) && FVector::Dist2D(Partner->GetActorLocation(),P->GetActorLocation())<600);
            Check(TEXT("autosave retains destination"),S->LoadSlot(0) && S->State.Checkpoint.Equals(FVector(0,2200,120)));
            PC->ToggleMenu();PC->ActivateMenuRow(9);PC->ActivateMenuRow(4);Check(TEXT("earned stage record shown"),PC->MenuStatus().Contains(TEXT("当前任务")));
            PC->MenuBack();Check(TEXT("back retains pause"),!PC->ArchivePage && PC->IsMenuOpen());PC->ActivateMenuRow(9);
            Check(TEXT("return institute"),PC->TravelTo(TEXT("institute")));Check(TEXT("resources unchanged"),S->State.Materials.FindRef(TEXT("research.sample"))==4 && S->State.Party==Ids);
            PC->ToggleMenu();PC->ActivateMenuRow(9);PC->ActivateMenuRow(4);
            PC->ActivateMenuRow(8);Check(TEXT("map reopened after round trip"),PC->MapWidget&&UGameplayStatics::IsGamePaused(W));
            if(PC->MapWidget){PC->MapWidget->SelectedPoint=1;PC->MapWidget->Activate(2);}
            FScreenshotRequest::RequestScreenshot(TEXT("VZ_Map.png"),true,false);

        }
        UE_LOG(LogTemp,Display,TEXT("VZ_TRAVEL: %s unlock, navigation, write failure, combat, party identity, autosave, records and return"),Ok?TEXT("PASS"):TEXT("FAIL"));
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Ok](float){FPlatformMisc::RequestExitWithStatus(false,Ok?0:1);return false;}),1.f);
    },3.f,false);
#endif
}
