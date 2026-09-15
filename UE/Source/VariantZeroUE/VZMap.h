#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VZState.h"
#include "VZMap.generated.h"

struct FVZMapPoint
{
    FString Label;
    FVector Position;
    FString TravelId;
};
class VARIANTZEROUE_API FVZMapRules
{
public:
    static TArray<FVZMapPoint> Points(const FVZWorldState& State);
    static FVector2D Project(const FVector& Position);
};

UCLASS()
class VARIANTZEROUE_API UVZMapWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    int32 SelectedPoint=0;
    int32 SelectedAction=2;
    TArray<FVZMapPoint> Points;
    void RefreshPoints();
    void SelectNext(int32 Direction);
    void Activate(int32 Action);
protected:
    virtual void NativeConstruct() override;
    virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& Clip,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry,const FPointerEvent& Event) override;
};
