#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VZInputSettings.h"
#include "VZPlayerController.generated.h"
class SVZPauseMenu;
class UVZMapWidget;

UCLASS()
class VARIANTZEROUE_API AVZPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AVZPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    void ToggleMenu();
    void CloseMenu();
    void OpenMap();
    void CloseMap();
    UPROPERTY() TObjectPtr<UVZMapWidget> MapWidget;
    bool HasMapMarker=false;
    FVector MapMarker=FVector::ZeroVector;
    FString MapMarkerLabel;
    void ActivateMenuRow(int32 Row);
    FString MenuLabel(int32 Row) const;
    FString MenuStatus() const;
    bool IsMenuOpen() const { return Menu.IsValid(); }
    int32 SelectedSlot=1;
    float MasterVolume=1;
    float MusicVolume=1;
    float EffectsVolume=1;
    int32 DialogueFontSize=18;
    bool PresentationPage=false;
    int32 PendingAction=-1;
    FVZInputSettings Bindings;
    bool BindingsPage=false;
    bool ArchivePage=false;
    FString ArchiveText;
    bool RosterPage=false;
    int32 RosterIndex=0;
    FString RenameDraft;
    void SelectCompanion(int32 Delta);
    bool EditCompanion(bool Rename,int32 Direction=0);
    bool ToggleCompanionDeployment();
    bool TrainCompanion(int32 ExpectedLevel);
    bool TravelTo(const FString& Destination);
    int32 BindingIndex=0;
    int32 CaptureDevice=-1;
    bool CaptureBinding(FKey Key);
    void MenuBack();
private:
    TSharedPtr<SVZPauseMenu> Menu;
    bool Verification=false;
    bool VerificationOk=true;
    int32 VerificationStep=0;
    double VerificationStart=0;
    float PausedAttackClock=0;
    FVector PausedPosition;
    FString SettingsPath() const;
    void ApplyVolume();
    void LoadPresentationSettings();
    bool SavePresentationSettings();
};
