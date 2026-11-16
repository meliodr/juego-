#include "ProfilingDebugging/CsvProfiler.h"
#if WITH_DEV_AUTOMATION_TESTS && CSV_PROFILER
#include "IBPlayCharacter.h"
#include "IBPlaySession.h"
#include "IB_S3/IBS3World.h"
#include "IB_S5/IBS5WorldLife.h"
#include "IB_S6H/IBS6HBridge.h"
#include "Async/Future.h"
#include "Camera/CameraTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "InputKeyEventArgs.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Rendering/SlateRenderer.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "RHIGlobals.h"
#include "Scalability.h"
#include "SceneView.h"
#include "Slate/SlateViewportProvider.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"

namespace
{
class FIBPlayPerformanceRoute final : public IAutomationLatentCommand
{
    FAutomationTestBase& Test;
    const double Duration;
    const double CreatedAt = FPlatformTime::Seconds();
    TWeakObjectPtr<AIBPlayCharacter> Hero;
    TWeakObjectPtr<APlayerController> Controller;
    TSharedFuture<FString> Csv;
    TSet<FKey> HeldKeys;
    TMap<FKey,double> Releases;
    double ReadyAt = 0;
    double Started = 0;
    double PreviousAt = 0;
    double EndedAt = 0;
    double FlightSeconds = 0;
    double TravelCm = 0;
    double PeakSpeedCmS = 0;
    double NextLaser = 0;
    int32 Phase = -1;
    int32 LaserPulses = 0;
    int32 AvoidanceFrames = 0;
    uint64 FirstFrame = 0;
    uint64 LastFrame = 0;
    int32 PreviousGPUCsv = -1;
    FVector Anchor = FVector::ZeroVector;
    FVector PreviousLocation = FVector::ZeroVector;
    FString Diagnostics;
    bool bCaptureStarted = false;
    bool bInitialMarker = false;

    FViewport* Viewport() const
    { return GEngine && GEngine->GameViewport ? GEngine->GameViewport->Viewport : nullptr; }

    void Key(FKey Input, bool Down)
    {
        if (!Controller.IsValid() || HeldKeys.Contains(Input) == Down) return;
        Controller->InputKey(FInputKeyEventArgs::CreateSimulated(Input,Down ? IE_Pressed : IE_Released,Down ? 1.f : 0.f));
        if (Down) HeldKeys.Add(Input); else HeldKeys.Remove(Input);
    }

    void Pulse(FKey Input,double Now)
    { Key(Input,true); Releases.Add(Input,Now+0.12); }

    void Turn(double Degrees, double Dt)
    {
        if (!Controller.IsValid()) return;
        auto Event = FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,static_cast<float>(Degrees));
        Event.DeltaTime = static_cast<float>(Dt); Event.NumSamples = 1;
        Controller->InputKey(Event);
    }

    void StopInputs()
    {
        const TArray<FKey> Keys = HeldKeys.Array();
        for (FKey Input : Keys) Key(Input,false);
        Releases.Empty();
    }

    bool CheckView(const TCHAR* Label, bool ReadBack)
    {
        FViewport* Screen = Viewport();
        ULocalPlayer* Local = Controller.IsValid() ? Controller->GetLocalPlayer() : nullptr;
        FSceneViewProjectionData Projection;
        FMinimalViewInfo View;
        if (!Screen || !Local || !Local->GetProjectionData(Screen,Projection))
        { Test.AddError(TEXT("Rendered profile has no valid local view projection.")); return false; }
        if(!Controller->PlayerCameraManager) { Test.AddError(TEXT("No active player camera manager.")); return false; }
        View=Controller->PlayerCameraManager->GetCameraCacheView();
        const FIntPoint Extent = Screen->GetSizeXY();
        const bool Valid = Extent == FIntPoint(1920,1080) && !View.Location.ContainsNaN() && !View.Rotation.ContainsNaN()
            && FMath::IsFinite(View.FOV) && View.FOV > 1 && View.FOV < 179
            && Projection.GetConstrainedViewRect().Width() > 0 && Projection.GetConstrainedViewRect().Height() > 0
            && !Projection.ComputeViewProjectionMatrix().ContainsNaN()
            && Controller->GetViewTarget() == Hero.Get();
        if (!Valid) { Test.AddError(TEXT("Rendered profile requires a finite production view at actual 1920x1080.")); return false; }
        if (!ReadBack) return true;
        // Deterministic local render validation only. Pixel arrays never leave this function,
        // no screenshot or image asset is written, and GPU readback is outside the CSV interval.
        TArray<FColor> Pixels;
        const FIntRect Region(Extent.X/3,Extent.Y/3,Extent.X*2/3,Extent.Y*2/3);
        const TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
        ISlateViewportProvider* Provider = Window && FSlateApplication::IsInitialized()
            ? FSlateApplication::Get().GetRenderer()->GetViewportProvider(*Window) : nullptr;
        if (!Provider)
        { Test.AddError(TEXT("DX12 viewport has no Slate framebuffer provider.")); return false; }
        // FSceneViewport releases its transient render-thread target after Draw.
        // Slate owns the actual offscreen texture or window backbuffer instead.
        // Retain the window through the flush and the texture through the read.
        ENQUEUE_RENDER_COMMAND(IBPlayReadPresentedFrame)(
            [Provider, Window, Extent, Region, &Pixels](FRHICommandListImmediate& RHICmdList)
            {
                const FTextureRHIRef Framebuffer = Provider->GetBackBufferResource();
                if (Framebuffer.IsValid() && Framebuffer->GetSizeX() == Extent.X && Framebuffer->GetSizeY() == Extent.Y)
                    RHICmdList.ReadSurfaceData(Framebuffer,Region,Pixels,FReadSurfaceDataFlags(RCM_UNorm));
            });
        FlushRenderingCommands();
        if (Pixels.Num() != Region.Area())
        { Test.AddError(TEXT("DX12 viewport color readback failed.")); return false; }
        double Sum = 0, Square = 0;
        int32 NonBlack = 0;
        for (const FColor& Pixel : Pixels)
        {
            const double Luma = 0.2126*Pixel.R+0.7152*Pixel.G+0.0722*Pixel.B;
            Sum += Luma; Square += Luma*Luma; NonBlack += Luma > 3;
        }
        const double Mean = Sum/Pixels.Num();
        const double Variance = FMath::Max(0.0,Square/Pixels.Num()-Mean*Mean);
        const double Fraction = static_cast<double>(NonBlack)/Pixels.Num();
        Diagnostics += FString::Printf(TEXT("%s_luminance_mean=%.6f\n%s_luminance_variance=%.6f\n%s_nonblack_fraction=%.6f\n"),Label,Mean,Label,Variance,Label,Fraction);
        Pixels.Empty();
        const bool HasScene = Fraction > 0.02 && Variance > 0.05;
        Test.TestTrue(FString::Printf(TEXT("%s DX12 frame is not black or uniform"),Label),HasScene);
        return HasScene;
    }

    void Metadata()
    {
        const auto Set = [this](const TCHAR* Name,const FString& Value)
        { FCsvProfiler::SetMetadata(Name,*Value); Diagnostics += FString(Name)+TEXT("=")+Value+TEXT("\n"); };
        Set(TEXT("IB_RHI"),FApp::GetGraphicsRHI());
        Set(TEXT("IB_CPU"),FPlatformMisc::GetCPUBrand());
        Set(TEXT("IB_GPU"),GRHIGlobals.GpuInfo.AdapterName);
        Set(TEXT("IB_PrimaryDisplayGPU"),FPlatformMisc::GetPrimaryGPUBrand());
        Set(TEXT("IB_Engine"),FEngineVersion::Current().ToString());
        Set(TEXT("IB_Build"),FApp::GetBuildVersion());
        Set(TEXT("IB_RenderExtent"),TEXT("1920x1080"));
        Set(TEXT("IB_Map"),Hero->GetWorld()->GetMapName());
        Set(TEXT("IB_TimeSource"),TEXT("FPlatformTime real wall clock; no fixed timestep"));
        Set(TEXT("IB_Route"),TEXT("Warmup10s; Observe15s; Takeoff10s; Orbit45s; Laser15s; Brake5s"));
        Set(TEXT("IB_DurationRequestedSeconds"),FString::SanitizeFloat(Duration));
        FString Revision;
        if (FParse::Value(FCommandLine::Get(),TEXT("IBBuildRevision="),Revision)) Set(TEXT("IB_Revision"),Revision);
        for (const TCHAR* Name : {TEXT("r.VSync"),TEXT("t.MaxFPS"),TEXT("r.ScreenPercentage"),TEXT("r.DynamicRes.OperationMode"),TEXT("r.AntiAliasingMethod"),TEXT("r.GPUCsvStatsEnabled")})
            if (const IConsoleVariable* Value = IConsoleManager::Get().FindConsoleVariable(Name)) Set(Name,Value->GetString());
        const auto Quality = Scalability::GetQualityLevels();
        Set(TEXT("IB_Quality"),FString::Printf(TEXT("view=%d shadows=%d textures=%d effects=%d post=%d foliage=%d shading=%d"),
            Quality.ViewDistanceQuality,Quality.ShadowQuality,Quality.TextureQuality,Quality.EffectsQuality,
            Quality.PostProcessQuality,Quality.FoliageQuality,Quality.ShadingQuality));
    }

    bool Fail(const TCHAR* Reason)
    {
        Test.AddError(Reason); StopInputs();
        if (bCaptureStarted && !Csv.IsValid()) { EndedAt = FPlatformTime::Seconds(); LastFrame = GFrameCounter; Csv = FCsvProfiler::Get()->EndCapture(); return false; }
        return true;
    }

    void Orbit(double Dt)
    {
        const FVector Position = Hero->GetActorLocation();
        const FVector Up = (Position-Hero->Planet->GetActorLocation()).GetSafeNormal();
        const FVector Forward = FVector::VectorPlaneProject(Hero->GetActorForwardVector(),Up).GetSafeNormal();
        const FVector ToAnchor = FVector::VectorPlaneProject(Anchor-Position,Up);
        double TurnRate = 25;
        bool Go = true;
        if (ToAnchor.SizeSquared() > FMath::Square(6500.0))
        {
            const FVector Inward = ToAnchor.GetSafeNormal();
            const double Angle = FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Forward,Inward),Up),FVector::DotProduct(Forward,Inward));
            TurnRate = FMath::Clamp(FMath::RadiansToDegrees(Angle)*2,-70.0,70.0);
            Go = FMath::Abs(Angle) < PI/3;
        }
        FHitResult Obstacle;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBPlayPerformanceAvoid),false,Hero.Get());
        const double LookAhead = FMath::Max(900.0,Hero->GetVelocity().Size()*0.7);
        const bool Blocked = Hero->GetWorld()->SweepSingleByChannel(Obstacle,Position,Position+Forward*LookAhead,
            Hero->GetActorQuat(),ECC_Pawn,Hero->GetCapsuleComponent()->GetCollisionShape(),Query);
        if (Blocked) { Go = false; TurnRate = 65; ++AvoidanceFrames; }
        Key(EKeys::X,!Go); Key(EKeys::W,Go);
        Turn(FMath::Clamp(TurnRate*Dt,-3.0,3.0),Dt);
    }

public:
    FIBPlayPerformanceRoute(FAutomationTestBase& InTest,double InDuration) : Test(InTest),Duration(InDuration) {}
    bool Update() override
    {
        double Now = FPlatformTime::Seconds();
        if (Csv.IsValid())
        {
            if (!Csv.IsReady())
            {
                if (Now-EndedAt > 60) { Test.AddError(TEXT("CSV writer did not complete within 60 seconds.")); return true; }
                return false;
            }
            StopInputs();
            if (PreviousGPUCsv >= 0) if (auto* Value = IConsoleManager::Get().FindConsoleVariable(TEXT("r.GPUCsvStatsEnabled"))) Value->Set(PreviousGPUCsv,ECVF_SetByCode);
            if (Hero.IsValid() && Controller.IsValid()) CheckView(TEXT("after"),true);
            const FString Path = Csv.Get();
            Diagnostics += FString::Printf(TEXT("measured_wall_seconds=%.6f\nengine_frames=%llu\nflight_seconds=%.6f\ntravel_cm=%.6f\npeak_speed_cm_s=%.6f\nlaser_input_pulses=%d\navoidance_frames=%d\n"),
                EndedAt-Started,LastFrame-FirstFrame,FlightSeconds,TravelCm,PeakSpeedCmS,LaserPulses,AvoidanceFrames);
            const bool Written = !Path.IsEmpty() && FFileHelper::SaveStringToFile(Diagnostics,*(Path+TEXT(".view_metrics.txt")));
            Test.TestTrue(TEXT("CSV capture and numeric view diagnostics saved"),Written);
            Test.TestTrue(TEXT("Measured route includes physical flight"),FlightSeconds >= 30);
            Test.TestTrue(TEXT("Input route really changes position"),TravelCm >= 1000);
            Test.AddInfo(FString::Printf(TEXT("Rendered DX12 profile: %s; %.3f real seconds, %.2fm movement, %.2fs flying. No FPS target asserted; use captured FrameTime/GameThreadTime/RenderThreadTime/GPUTime p95/p99."),*Path,EndedAt-Started,TravelCm/100,FlightSeconds));
            return true;
        }
        if (Now-CreatedAt > Duration+150) return Fail(TEXT("Playable rendered profile timed out during setup or route."));
        UWorld* World = GEngine->GetCurrentPlayWorld();
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        AIBPlayCharacter* Pilot = PC ? Cast<AIBPlayCharacter>(PC->GetPawn()) : nullptr;
        if (!Pilot || !Pilot->Planet || !Pilot->Planet->IsOperational() || !Pilot->Session || !Pilot->Session->Life
            || !Pilot->Session->Life->bRegionReady)
            return bCaptureStarted ? Fail(TEXT("Production pawn or ready region was lost during capture.")) : false;
        Hero = Pilot; Controller = PC;
        if (Pilot->Session->bPersistence || Pilot->Session->Life->bEnablePersistence)
            return Fail(TEXT("Rendered QA must isolate player saves with -IBPlayQA."));
        if (ReadyAt == 0) ReadyAt = Now+10;
        if (Now < ReadyAt) return false;
        if (Started == 0)
        {
            if (FCsvProfiler::IsCapturing()) return Fail(TEXT("Another CSV capture is already active."));
            if (!CheckView(TEXT("before"),true)) return true;
            if (Pilot->IsGameplayLocked()) return Fail(TEXT("Close gameplay menus before the measured route."));
            // Verified in installed UE 5.8 RHI/Private/GPUProfiler.cpp; default is zero.
            if (auto* Value = IConsoleManager::Get().FindConsoleVariable(TEXT("r.GPUCsvStatsEnabled")))
            { PreviousGPUCsv = Value->GetInt(); Value->Set(1,ECVF_SetByCode); }
            else Diagnostics += TEXT("GPU_CSV_enable=unavailable; do not infer GPU timings from CPU wall time\n");
            Metadata();
            Started = PreviousAt = Now = FPlatformTime::Seconds(); FirstFrame = GFrameCounter;
            Anchor = PreviousLocation = Pilot->GetActorLocation();
            FCsvProfiler::Get()->BeginCapture(); bCaptureStarted = true;
        }
        const double Dt = FMath::Max(0.0,Now-PreviousAt);
        const double Elapsed = Now-Started;
        if (!bInitialMarker && FCsvProfiler::IsCapturing()) { CSV_EVENT_GLOBAL(TEXT("IB:PlayObserve")); bInitialMarker = true; }
        PreviousAt = Now;
        if (Pilot->GetActorLocation().ContainsNaN() || !CheckView(TEXT("live"),false)) return Fail(TEXT("Invalid position or rendered view during route."));
        TravelCm += FVector::Dist(PreviousLocation,Pilot->GetActorLocation()); PreviousLocation = Pilot->GetActorLocation();
        PeakSpeedCmS = FMath::Max(PeakSpeedCmS,Pilot->GetVelocity().Size());
        if (Pilot->GetCharacterMovement()->IsFlying()) FlightSeconds += Dt;
        TArray<FKey> Due;
        for (const auto& Pair : Releases) if (Now >= Pair.Value) Due.Add(Pair.Key);
        for (FKey Input : Due) { Key(Input,false); Releases.Remove(Input); }
        const double Normalized = Elapsed*90/Duration;
        const int32 NextPhase = Normalized < 15 ? 0 : Normalized < 25 ? 1 : Normalized < 70 ? 2 : Normalized < 85 ? 3 : 4;
        if (NextPhase != Phase)
        {
            Phase = NextPhase; StopInputs();
            if (Phase == 0) { CSV_EVENT_GLOBAL(TEXT("IB:PlayObserve")); }
            if (Phase == 1)
            {
                CSV_EVENT_GLOBAL(TEXT("IB:PlayTakeoff"));
                if (!Pilot->GetCharacterMovement()->IsFlying()) Pulse(EKeys::F,Now);
                Pulse(EKeys::Q,Now);
            }
            if (Phase == 2) { CSV_EVENT_GLOBAL(TEXT("IB:PlayOrbit")); Anchor = Pilot->GetActorLocation(); }
            if (Phase == 3)
            {
                CSV_EVENT_GLOBAL(TEXT("IB:PlayLaser")); Key(EKeys::X,true);
                Pilot->Heroes->SelectHero(0); Pilot->Heroes->EquipAbility(104); NextLaser = Now+0.5;
            }
            if (Phase == 4) { CSV_EVENT_GLOBAL(TEXT("IB:PlayBrake")); Key(EKeys::X,true); }
        }
        if (Phase == 0)
        {
            const bool Walking = Normalized >= 3 && Normalized < 5;
            Key(EKeys::W,Walking);
            if (Normalized >= 5 && Normalized < 8) Turn(12*Dt,Dt);
        }
        if (Phase == 1) Key(EKeys::SpaceBar,Normalized >= 16 && Normalized < 19);
        if (Phase == 2) Orbit(Dt);
        if (Phase == 3 && Now >= NextLaser)
        { Pulse(EKeys::LeftMouseButton,Now); ++LaserPulses; NextLaser = Now+2.5; }
        if (Elapsed >= Duration)
        {
            StopInputs(); EndedAt = Now; LastFrame = GFrameCounter; Csv = FCsvProfiler::Get()->EndCapture();
        }
        return false;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlayPerformanceTest,"ImpactBeyond.Performance.PlayableDX12",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBPlayPerformanceTest::RunTest(const FString& Parameters)
{
    const FString RHI = FApp::GetGraphicsRHI();
    if (!FApp::CanEverRender() || !(RHI.Contains(TEXT("D3D12")) || RHI.Contains(TEXT("DirectX 12")) || RHI.Contains(TEXT("DX12"))))
    { AddError(TEXT("Run the packaged production map with a rendering DX12 RHI, never NullRHI.")); return false; }
    if (FApp::UseFixedTimeStep() || GEngine->bUseFixedFrameRate) { AddError(TEXT("Performance samples require real frame timing; remove fixed timestep/frame-rate settings.")); return false; }
    if (!FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA"))) { AddError(TEXT("Use -IBPlayQA to isolate saved games.")); return false; }
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World || !World->GetMapName().Contains(TEXT("Lvl_IB_Playable")))
    { AddError(TEXT("Use /Game/IB_Playable/Lvl_IB_Playable for the production profile.")); return false; }
    if (!FMath::IsNearlyEqual(World->GetWorldSettings()->GetEffectiveTimeDilation(),1.f))
    { AddError(TEXT("Performance route requires normal world time dilation.")); return false; }
    float Seconds = 90;
    FParse::Value(FCommandLine::Get(),TEXT("IBPlayPerfSeconds="),Seconds);
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FIBPlayPerformanceRoute>(*this,FMath::Clamp(Seconds,90.f,120.f)));
    return true;
}
#endif
