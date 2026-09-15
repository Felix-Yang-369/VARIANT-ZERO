#include "VZMap.h"
#include "VZPlayerController.h"
#include "VZPrototype.h"
#include "VZPrologue.h"
#include "VZGarden.h"
#include "VZChuyaStory.h"
#include "VZTravel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

TArray<FVZMapPoint> FVZMapRules::Points(const FVZWorldState& S)
{
    TArray<FVZMapPoint> R;
    if(!FVZSaveStore::Validate(S))return R;
    R.Add({TEXT("原初研究所"),FVector(0,0,120),TEXT("institute")});
    if(FVZGardenRules::Stage(S)>=1)R.Add({TEXT("花庭入口"),FVector(0,2200,120),TEXT("garden")});
    R.Add({TEXT("培养台 · 伙伴整备"),FVector(-650,350,120),FString()});
    const auto Targets=FVZGardenRules::Stage(S)>=3?FVZChuyaStoryRules::Targets(S):S.Repairs.Contains(TEXT("prologue.pollinated"))?FVZGardenRules::Targets(S):FVZPrologueRules::Targets(S);
    for(const auto& T:Targets)R.Add({T.Value,T.Key,FString()});
    return R;
}
FVector2D FVZMapRules::Project(const FVector& P)
{
    // North is +Y. Uniform centimetre scale in the chapter's surveyed bounds.
    return FVector2D(370+P.X*.07,650-P.Y*.07);
}
void UVZMapWidget::NativeConstruct()
{
    Super::NativeConstruct();SetIsFocusable(true);
    if(!WidgetTree->RootWidget)WidgetTree->RootWidget=WidgetTree->ConstructWidget<UCanvasPanel>();
    RefreshPoints();
}
void UVZMapWidget::RefreshPoints()
{
    if(auto* P=Cast<AVZEcologist>(GetOwningPlayerPawn()))Points=FVZMapRules::Points(P->State()->State);
    SelectedPoint=FMath::Clamp(SelectedPoint,0,FMath::Max(0,Points.Num()-1));
}
void UVZMapWidget::SelectNext(int32 Direction)
{
    if(Points.Num())SelectedPoint=(SelectedPoint+Direction+Points.Num())%Points.Num();
}
void UVZMapWidget::Activate(int32 Action)
{
    auto* PC=Cast<AVZPlayerController>(GetOwningPlayer());if(!PC)return;
    if(Action==0){SelectNext(-1);return;}
    if(Action==1){SelectNext(1);return;}
    if(Action==5){PC->CloseMap();return;}
    if(Action==3){PC->HasMapMarker=false;PC->ArchiveText=TEXT("导航标记已清除。");return;}
    if(!Points.IsValidIndex(SelectedPoint))return;
    const auto Point=Points[SelectedPoint];
    if(Action==2){PC->HasMapMarker=true;PC->MapMarker=Point.Position;PC->MapMarkerLabel=Point.Label;PC->ArchiveText=TEXT("已标记。返回探索后可看到方向与距离。");}
    if(Action==4)
    {
        if(Point.TravelId.IsEmpty()){PC->ArchiveText=TEXT("这里是任务目标。请步行前往；只有已解锁的移动点支持快速移动。");return;}
        PC->TravelTo(Point.TravelId);
    }
}
int32 UVZMapWidget::NativePaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Clip,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const
{
    Layer=Super::NativePaint(Args,G,Clip,Out,Layer,Style,Enabled)+1;
    const auto* P=Cast<AVZEcologist>(GetOwningPlayerPawn());const auto* PC=Cast<AVZPlayerController>(GetOwningPlayer());if(!P||!PC)return Layer;
    const float Scale=FMath::Min(G.GetLocalSize().X/1200.f,G.GetLocalSize().Y/800.f);
    const FVector2D Offset=(G.GetLocalSize()-FVector2D(1200,800)*Scale)*.5;
    const FLinearColor Jade(.36,.9,.66),Gold(1,.76,.3),Paper(.9,.95,.91),Muted(.52,.65,.61);
    auto Box=[&](FVector2D Pos,FVector2D Size,FLinearColor Color){FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(Size*Scale,FSlateLayoutTransform(Offset+Pos*Scale)),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,Color);};
    auto Text=[&](FVector2D Pos,const FString& Value,int32 Size,FLinearColor Color){FSlateDrawElement::MakeText(Out,Layer+1,G.ToPaintGeometry(FVector2D(1100,80)*Scale,FSlateLayoutTransform(Offset+Pos*Scale)),Value,FCoreStyle::GetDefaultFontStyle("Regular",FMath::RoundToInt(Size*Scale)),ESlateDrawEffect::None,Color);};
    FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,FLinearColor(.012,.025,.021,1));
    Text({40,25},TEXT("生态师手记  /  地图"),30,Paper);Text({40,70},TEXT("研究所与无访花庭 · 已勘察区域示意"),17,Muted);
    Box({40,115},{650,630},FLinearColor(.025,.065,.052));
    for(int32 I=0;I<9;++I)Box({80,135.f+I*70},{570,1},FLinearColor(.06,.12,.095));
    for(int32 I=0;I<9;++I)Box({80.f+I*70,135},{1,560},FLinearColor(.06,.12,.095));
    Box(FVZMapRules::Project(FVector(-1500,1300,0)),{210,182},FLinearColor(.12,.23,.18));
    Box(FVZMapRules::Project(FVector(-300,2200,0)),{42,63},FLinearColor(.16,.27,.20));
    const bool Garden=FVZGardenRules::Stage(P->State()->State)>=1;
    if(Garden)
    {
        Box(FVZMapRules::Project(FVector(-1300,7000,0)),{182,336},FLinearColor(.10,.25,.19));
        Box(FVZMapRules::Project(FVector(-100,6500,0)),{14,280},FLinearColor(.18,.34,.25));
        Text({450,185},TEXT("无访花庭"),18,Jade);
        Text({445,225},P->State()->State.Repairs.Contains(TEXT("garden.waterway"))?TEXT("水循环已恢复"):TEXT("供水尚未恢复"),14,Muted);
    }
    else Text({260,290},TEXT("尚未勘察"),22,Muted);
    Text({450,605},TEXT("原初研究所"),18,Jade);Text({70,130},TEXT("北 ↑"),20,Paper);
    Box({80,704},{70,3},Paper);Text({160,691},TEXT("10 米"),14,Muted);
    for(int32 I=0;I<Points.Num();++I)
    {
        const auto Pos=FVZMapRules::Project(Points[I].Position);
        if(I==SelectedPoint)Box(Pos-FVector2D(10,10),{20,20},Paper);
        Box(Pos-FVector2D(6,6),{12,12},Points[I].TravelId.IsEmpty()?Gold:Jade);
        Text(Pos+FVector2D(12,-13),FString::FromInt(I+1),15,Paper);
    }
    if(PC->HasMapMarker)Text(FVZMapRules::Project(PC->MapMarker)+FVector2D(-8,-30),TEXT("◇"),25,Gold);
    FVector2D PlayerPos=FVZMapRules::Project(P->GetActorLocation());PlayerPos.X=FMath::Clamp(PlayerPos.X,55.,675.);PlayerPos.Y=FMath::Clamp(PlayerPos.Y,130.,720.);
    Box(PlayerPos-FVector2D(4,4),{8,8},FLinearColor(.3,.75,1));
    const float Yaw=FMath::DegreesToRadians(P->GetControlRotation().Yaw);
    TArray<FVector2D> Direction={Offset+PlayerPos*Scale,Offset+(PlayerPos+FVector2D(FMath::Cos(Yaw),-FMath::Sin(Yaw))*22)*Scale};
    FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Direction,ESlateDrawEffect::None,FLinearColor(.3,.75,1),true,2);
    Text({730,120},TEXT("蓝：你   绿：移动点   金：任务"),18,Paper);
    if(Points.IsValidIndex(SelectedPoint))
    {
        const auto& Point=Points[SelectedPoint];
        Text({730,166},FString::Printf(TEXT("%d / %d  %s"),SelectedPoint+1,Points.Num(),*Point.Label),18,Gold);
        Text({730,202},FString::Printf(TEXT("直线距离 %.0f 米 · %s"),FVector::Dist2D(P->GetActorLocation(),Point.Position)/100,Point.TravelId.IsEmpty()?TEXT("任务目标"):TEXT("已解锁移动点")),16,Muted);
    }
    static const TCHAR* Labels[]={TEXT("← 上一个地点"),TEXT("下一个地点 →"),TEXT("设置导航标记"),TEXT("清除导航标记"),TEXT("快速移动到选中地点"),TEXT("返回研究记录")};
    for(int32 I=0;I<6;++I){Box({730,250.f+I*52},{425,44},I==SelectedAction?FLinearColor(.14,.38,.26):FLinearColor(.05,.11,.08));Text({750,259.f+I*52},Labels[I],18,Paper);}
    // Wrap status to keep text within the panel at all viewport aspect ratios.
    FString Status=PC->ArchiveText;for(int32 I=0;I<3;++I)Text({730,578.f+I*27},Status.Mid(I*23,23),16,Muted);
    Text({730,685},TEXT("地图暂停模拟；导航标记仅在本次探索保留。"),15,Muted);
    Text({40,760},TEXT("左右：地点  ·  上下：操作  ·  Enter / A：确认  ·  Esc / B：返回  ·  鼠标可点击地点和操作"),16,Paper);
    return Layer+2;
}
FReply UVZMapWidget::NativeOnKeyDown(const FGeometry&,const FKeyEvent& E)
{
    const auto K=E.GetKey();
    if(K==EKeys::Escape||K==EKeys::Gamepad_FaceButton_Right||K==EKeys::Gamepad_Special_Left){Activate(5);return FReply::Handled();}
    if(K==EKeys::Left||K==EKeys::Gamepad_DPad_Left)SelectNext(-1);
    if(K==EKeys::Right||K==EKeys::Gamepad_DPad_Right)SelectNext(1);
    if(K==EKeys::Up||K==EKeys::Gamepad_DPad_Up)SelectedAction=(SelectedAction+5)%6;
    if(K==EKeys::Down||K==EKeys::Gamepad_DPad_Down)SelectedAction=(SelectedAction+1)%6;
    if(!E.IsRepeat()&&(K==EKeys::Enter||K==EKeys::SpaceBar||K==EKeys::Gamepad_FaceButton_Bottom))Activate(SelectedAction);
    return FReply::Handled();
}
FReply UVZMapWidget::NativeOnMouseButtonDown(const FGeometry& G,const FPointerEvent& E)
{
    if(E.GetEffectingButton()!=EKeys::LeftMouseButton)return FReply::Handled();
    const float Scale=FMath::Min(G.GetLocalSize().X/1200.f,G.GetLocalSize().Y/800.f);
    const FVector2D Pos=(G.AbsoluteToLocal(E.GetScreenSpacePosition())-(G.GetLocalSize()-FVector2D(1200,800)*Scale)*.5)/Scale;
    for(int32 I=0;I<6;++I)if(Pos.X>=730&&Pos.X<=1155&&Pos.Y>=250+I*52&&Pos.Y<=294+I*52){SelectedAction=I;Activate(I);return FReply::Handled();}
    for(int32 I=Points.Num()-1;I>=0;--I)if(FVector2D::Distance(Pos,FVZMapRules::Project(Points[I].Position))<=18){SelectedPoint=I;break;}
    return FReply::Handled();
}
