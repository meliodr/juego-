#if WITH_DEV_AUTOMATION_TESTS
#include "IBPlayReviewMode.h"
#include "IBPlayCharacter.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "ImageUtils.h"
#include "InputKeyEventArgs.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "Slate/SlateViewportProvider.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"

class FIBReviewRoute final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test; double Started=0,Created=FPlatformTime::Seconds(); int32 Shot=0;
    TWeakObjectPtr<APlayerController> Controller; TSet<FKey> Keys; FString Folder,Record;
    void Key(FKey K,bool Down) { if(!Controller.IsValid()||Keys.Contains(K)==Down) return; Controller->InputKey(FInputKeyEventArgs::CreateSimulated(K,Down?IE_Pressed:IE_Released,Down?1.f:0.f)); if(Down) Keys.Add(K); else Keys.Remove(K); }
    bool End(const FString& Error=FString()) { for(FKey K:Keys.Array()) Key(K,false); if(!Error.IsEmpty()) Test->AddError(Error); return true; }
    bool Capture(AIBPlayReviewGameMode* Mode,AIBPlayCharacter* Pawn,const TCHAR* Label)
    {
        FString Reason; if(!Mode->bReviewReady||!Mode->ValidateScene(Reason)) { Test->AddError(TEXT("Unsafe review capture rejected: ")+Reason); return false; }
        const double Speed=Pawn->GetVelocity().Size(); const bool Flying=Pawn->GetCharacterMovement()->IsFlying();
        if(!FMath::IsFinite(Speed)||(Shot==0&&(Flying||Speed<10))||(Shot==1&&!Pawn->GetCharacterMovement()->IsFalling())||(Shot>=2&&!Flying)||(Shot==2&&Speed>150)||(Shot==3&&Speed<500)||(Shot==4&&Speed<8000)) { Test->AddError(TEXT("Review input did not reach the requested motion state")); return false; }
        FVector Eye; FRotator Look; Controller->GetPlayerViewPoint(Eye,Look); if(Eye.ContainsNaN()||Look.ContainsNaN()||Controller->GetViewTarget()!=Pawn) { Test->AddError(TEXT("Invalid review camera")); return false; }
        auto* Viewport=GEngine->GameViewport.Get(); const FIntPoint Size=Viewport&&Viewport->Viewport?Viewport->Viewport->GetSizeXY():FIntPoint::ZeroValue;
        if(Size!=FIntPoint(1920,1080)||!FApp::CanEverRender()||!FApp::GetGraphicsRHI().Contains(TEXT("D3D12"))) { Test->AddError(TEXT("Review requires rendered DX12 at 1920x1080")); return false; }
        const TSharedPtr<SWindow> Window=Viewport->GetWindow();
        ISlateViewportProvider* Provider=Window&&FSlateApplication::IsInitialized()?FSlateApplication::Get().GetRenderer()->GetViewportProvider(*Window):nullptr;
        if(!Provider) { Test->AddError(TEXT("No review framebuffer")); return false; }
        TArray<FColor> Pixels;
        ENQUEUE_RENDER_COMMAND(IBReviewFrame)([Provider,Window,Size,&Pixels](FRHICommandListImmediate& Cmd) {
            const FTextureRHIRef Frame=Provider->GetBackBufferResource();
            if(Frame.IsValid()&&Frame->GetSizeX()==Size.X&&Frame->GetSizeY()==Size.Y) Cmd.ReadSurfaceData(Frame,FIntRect(0,0,Size.X,Size.Y),Pixels,FReadSurfaceDataFlags(RCM_UNorm));
        });
        FlushRenderingCommands();
        if(Pixels.Num()!=Size.X*Size.Y) { Test->AddError(TEXT("Review readback failed")); return false; }
        double Sum=0,Square=0; for(const FColor& C:Pixels) { const double L=.2126*C.R+.7152*C.G+.0722*C.B; Sum+=L; Square+=L*L; }
        if(Sum/Pixels.Num()<2||Square/Pixels.Num()-FMath::Square(Sum/Pixels.Num())<.05) { Test->AddError(TEXT("Review frame is black or uniform")); return false; }
        TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,TArrayView64<const FColor>(Pixels.GetData(),Pixels.Num()),PNG);
        if(PNG.IsEmpty()||!FFileHelper::SaveArrayToFile(PNG,*(Folder/Label+TEXT(".png")))) { Test->AddError(TEXT("Review PNG write failed")); return false; }
        Record+=FString::Printf(TEXT("%s SAFE_CC0_TEMPLATE_ONLY speed_cm_s=%.3f flying=%d\n"),Label,Pawn->GetVelocity().Size(),Pawn->GetCharacterMovement()->IsFlying());
        return FFileHelper::SaveStringToFile(Record,*(Folder/TEXT("review.txt")));
    }
public:
    explicit FIBReviewRoute(FAutomationTestBase* InTest):Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World=nullptr; for(const FWorldContext& C:GEngine->GetWorldContexts()) if(C.WorldType==EWorldType::Game) { World=C.World(); break; }
        auto* Mode=World?Cast<AIBPlayReviewGameMode>(World->GetAuthGameMode()):nullptr;
        if(!Mode||!Mode->bReviewReady) return FPlatformTime::Seconds()-Created>10?End(TEXT("Safe review mode did not become ready")):false;
        AIBPlayCharacter* Pawn=nullptr; for(TActorIterator<AIBPlayCharacter> It(World);It;++It) { Pawn=*It; break; }
        if(!Pawn) return End(TEXT("No review pawn")); Controller=Cast<APlayerController>(Pawn->GetController());
        if(!Controller.IsValid()) return End(TEXT("No review controller"));
        FString Reason; if(!Mode->ValidateScene(Reason)) return End(Reason);
        if(Started==0) { Started=FPlatformTime::Seconds(); Folder=FPaths::ProjectSavedDir()/TEXT("IBPresentationReview")/FGuid::NewGuid().ToString(EGuidFormats::Digits); IFileManager::Get().MakeDirectory(*Folder,true); Test->AddInfo(TEXT("Safe review output: ")+Folder); }
        const double T=FPlatformTime::Seconds()-Started;
        Key(EKeys::W,(T>=2&&T<4)||(T>=10&&T<15));
        Key(EKeys::SpaceBar,(T>=4&&T<4.12)||(T>=7&&T<7.6)); Key(EKeys::F,T>=6.5&&T<6.62);
        Key(EKeys::E,(T>=12&&T<12.12)||(T>=12.3&&T<12.42)); Key(EKeys::LeftShift,T>=12&&T<15); Key(EKeys::X,T>=15&&T<16.5);
        static const double Times[]={3.4,4.3,8.5,11.4,13.5,15.3}; static const TCHAR* Names[]={TEXT("ground"),TEXT("jump"),TEXT("hover"),TEXT("cruise"),TEXT("boost"),TEXT("brake")};
        if(Shot<6&&T>=Times[Shot]) { if(!Capture(Mode,Pawn,Names[Shot])) return End(TEXT("Review capture failed")); ++Shot; }
        return T>16.5?End():false;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPreviewSafeMotion,"ImpactBeyond.Presentation.PreviewSafeMotion",EAutomationTestFlags::ClientContext|EAutomationTestFlags::ProductFilter)
bool FIBPreviewSafeMotion::RunTest(const FString&) { ADD_LATENT_AUTOMATION_COMMAND(FIBReviewRoute(this)); return true; }
#endif
