#include "IBS3MapWidget.h"
#include "IBS3Character.h"
#include "IBS3World.h"
#include "IBS3Geometry.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"

namespace { IBS3::V3 MapVector(FVector P) { return {P.X,P.Y,P.Z}; } }
void UIBS3MapWidget::NativeConstruct() { Super::NativeConstruct(); SetVisibility(ESlateVisibility::HitTestInvisible); }
void UIBS3MapWidget::NativeTick(const FGeometry& Geometry,float Dt)
{
    Super::NativeTick(Geometry,Dt); Refresh-=Dt;
    if(Refresh>0 || !IsValid(Pilot) || !IsValid(Pilot->Planet) || !Pilot->Planet->IsOperational()) return;
    Refresh=0.1f; AIBS3PlanetWorld* W=Pilot->Planet.Get();
    const double R=W->Manifest->RadiusCm;
    const IBS3::V3 P=MapVector(Pilot->GetActorLocation()), Center=MapVector(W->GetActorLocation());
    const IBS3::Geo Geo=IBS3::geographic(P,R,Center,Pilot->LastLongitude);
    const IBS3::Frame GeographicFrame=IBS3::frame(Geo);
    const IBS3::V3 Forward=MapVector(Pilot->TangentForward),Up=(P-Center).unit();
    const IBS3::Frame LocalFrame={Up.cross(Forward).unit(),Forward,Up};
    const auto Global=IBS3::globalMap(Geo); PlayerGlobal={Global.x,Global.y};
    NorthDirection={GeographicFrame.north.dot(LocalFrame.east),-GeographicFrame.north.dot(LocalFrame.north)};
    const bool Pole=std::abs(Geo.lat)>IBS3::Pi/2-1e-4;
    const double Heading=FMath::Fmod(FMath::RadiansToDegrees(IBS3::heading(Forward,GeographicFrame))+360,360);
    Telemetry=FString::Printf(TEXT("Lat %.5f  Lon %.5f | Alt %.1f m | %.1f m/s | %s"),FMath::RadiansToDegrees(Geo.lat),
        FMath::RadiansToDegrees(Geo.lon),Geo.altitude/100,Pilot->GetVelocity().Size()/100,
        Pole?TEXT("Polo: rumbo geografico indeterminado"):*FString::Printf(TEXT("Rumbo %03.0f"),Heading));
    RegionText=FString::Printf(TEXT("%s | %s | %s"),*W->RegionAt(Pilot->GetActorLocation()).ToString(),*W->ActiveCity.ToString(),*W->Status);
    Icons.Reset(); DestinationText=TEXT("Tab / cruceta derecha: elegir destino");
    for(const FIBS3City& C:W->Manifest->Cities)
    {
        const auto G=IBS3::globalMap({FMath::DegreesToRadians(C.LatitudeDegrees),FMath::DegreesToRadians(C.LongitudeDegrees)});
        const auto L=IBS3::localMap(P,MapVector(W->CityPosition(C)),LocalFrame,R,Center);
        Icon I; I.Global={G.x,G.y}; I.Local={L.x,L.y}; I.Label=C.CityId.ToString(); I.Discovered=W->Discovered.Contains(C.CityId);
        I.Destination=C.CityId==W->DestinationCity; I.Ambiguous=L.ambiguous; Icons.Add(I);
        if(I.Destination)
        {
            const double D=IBS3::surfaceDistance(P,MapVector(W->CityPosition(C)),R,Center);
            DestinationText=FString::Printf(TEXT("Destino %s | %.2f km%s"),*I.Label,D/100000,L.ambiguous?TEXT(" | antipoda: rumbo ambiguo"):TEXT(""));
        }
    }
}
int32 UIBS3MapWidget::NativePaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Clip,
    FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle& Style,bool ParentEnabled) const
{
    const int32 Base=Super::NativePaint(Args,G,Clip,Elements,Layer,Style,ParentEnabled);
    FPaintContext C(G,Clip,Elements,Base+1,Style,ParentEnabled);
    auto Line=[&](FVector2D A,FVector2D B,FLinearColor Color=FLinearColor(0.15f,0.45f,0.55f),float Width=1)
    { UWidgetBlueprintLibrary::DrawLine(C,A,B,Color,true,Width); };
    auto Text=[&](const FString& S,FVector2D P,FLinearColor Color=FLinearColor::White)
    { if(GEngine) UWidgetBlueprintLibrary::DrawTextFormatted(C,FText::FromString(S),P,GEngine->GetSmallFont(),15,NAME_None,Color); };
    auto Mark=[&](FVector2D P,FLinearColor Color)
    { Line(P+FVector2D(-5,0),P+FVector2D(5,0),Color,3); Line(P+FVector2D(0,-5),P+FVector2D(0,5),Color,3); };
    // A thick Slate line supplies a solid rectangular backing without a scene capture or new texture.
    Line({12,68},{FMath::Min(1150.0,G.GetLocalSize().X-12),68},FLinearColor(0.01f,0.015f,0.025f,0.88f),124);
    Text(TEXT("S3 LAB | placeholder tecnico | M / panel tactil: mapa"),{24,18});
    Text(Telemetry,{24,40}); Text(DestinationText,{24,62},FLinearColor::Yellow); Text(RegionText,{24,84});
    Text(TEXT("F/R3 vuelo | Shift/L3 travesia | E/Q o R2/L2 altura | X/cuadrado freno | G/circulo interaccion | F5/abajo guardar"),{24,106});
    const FVector2D Origin(24,145); const FVector2D Size(FMath::Max(160.0,FMath::Min(900.0,G.GetLocalSize().X-48)),350);
    if(Pilot && Pilot->bGlobalMap)
    {
        const double PanelHeight=Size.Y+55+FMath::DivideAndRoundUp(Icons.Num(),2)*24;
        Line(Origin+FVector2D(0,PanelHeight/2),Origin+FVector2D(Size.X,PanelHeight/2),FLinearColor(0.01f,0.015f,0.025f,0.92f),PanelHeight);
        for(int32 X=0;X<=12;++X) Line(Origin+FVector2D(Size.X*X/12,0),Origin+FVector2D(Size.X*X/12,Size.Y));
        for(int32 Y=0;Y<=6;++Y) Line(Origin+FVector2D(0,Size.Y*Y/6),Origin+FVector2D(Size.X,Size.Y*Y/6));
        for(int32 Index=0;Index<Icons.Num();++Index)
        {
            const Icon& I=Icons[Index];
            FVector2D P=Origin+I.Global*Size; const auto Color=I.Destination?FLinearColor::Yellow:(I.Discovered?FLinearColor::Green:FLinearColor::Gray);
            Mark(P,Color); Text(FString::FromInt(Index+1),{FMath::Clamp(P.X+8,Origin.X,Origin.X+Size.X-24),P.Y+8},Color);
            Text(FString::Printf(TEXT("%d  %s %s"),Index+1,*I.Label,I.Discovered?TEXT("[descubierta]"):TEXT("[?]")),
                Origin+FVector2D((Index%2)*(Size.X/2),Size.Y+40+(Index/2)*24),Color);
        }
        Mark(Origin+PlayerGlobal*Size,FLinearColor::White); Text(TEXT("-180                         Longitud                         +180 | N arriba"),Origin+FVector2D(0,Size.Y+12));
    }
    else
    {
        Line(Origin+FVector2D(0,145),Origin+FVector2D(260,145),FLinearColor(0.01f,0.015f,0.025f,0.88f),290);
        const FVector2D Center=Origin+FVector2D(120,120); const double Range=200000; // 2 km, heading-up.
        for(int32 I=0;I<48;++I)
        {
            const double A=IBS3::Tau*I/48,B=IBS3::Tau*(I+1)/48;
            Line(Center+FVector2D(std::cos(A),std::sin(A))*110,Center+FVector2D(std::cos(B),std::sin(B))*110);
        }
        Text(TEXT("N"),Center+NorthDirection*120,FLinearColor::White);
        for(const Icon& I:Icons)
        {
            if(I.Ambiguous) continue;
            FVector2D Offset=I.Local*(110/Range); const bool Outside=Offset.Size()>105;
            if(Outside && !I.Destination) continue;
            if(Outside) Offset=Offset.GetSafeNormal()*105;
            const FLinearColor Color=I.Destination?FLinearColor::Yellow:(I.Discovered?FLinearColor::Green:FLinearColor::Gray);
            Mark(Center+Offset,Color);
            if(I.Destination) Line(Center,Center+Offset,Color);
        }
        Mark(Center,FLinearColor::White); Line(Center,Center+FVector2D(0,-15),FLinearColor::White,3);
        Text(TEXT("2 km | rumbo arriba"),Origin+FVector2D(0,260));
    }
    return C.MaxLayer;
}
