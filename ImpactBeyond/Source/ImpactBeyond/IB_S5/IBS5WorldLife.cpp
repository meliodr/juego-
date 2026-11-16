#include "IBS5WorldLife.h"
#include "Animation/AnimSequence.h"
#include "AudioDeviceHandle.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"

AIBS5Citizen::AIBS5Citizen()
{
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CitizenCapsule"));
    SetRootComponent(Capsule);
    Capsule->InitCapsuleSize(34.0f, 90.0f);
    Capsule->SetCollisionProfileName(TEXT("Pawn"));
    Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CitizenMesh"));
    Mesh->SetupAttachment(Capsule);
    Mesh->SetRelativeLocation(FVector(0, 0, -90));
    Mesh->SetRelativeRotation(FRotator(0, -90, 0));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Outfit = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CitizenOutfit"));
    Outfit->SetupAttachment(Mesh);
    Outfit->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Outfit->SetLeaderPoseComponent(Mesh);
    Outfit->SetVisibility(false);
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> RunAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
    Mesh->SetSkeletalMesh(Manny.Object);
    Idle = IdleAsset.Object;
    Walk = WalkAsset.Object;
    Run = RunAsset.Object;
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CitizenState"));
    Label->SetupAttachment(Capsule);
    Label->SetRelativeLocation(FVector(0, 0, 125));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(22.0f);
    Label->SetTextRenderColor(FColor(195, 225, 255));
}

void AIBS5Citizen::SetAppearance(USkeletalMesh* Appearance)
{
    Outfit->SetSkeletalMesh(Appearance);
    Outfit->SetLeaderPoseComponent(Mesh, true);
    Outfit->SetVisibility(Appearance != nullptr);
    Mesh->SetVisibility(Appearance == nullptr, false);
}

void AIBS5Citizen::UpdatePresentation(const FString& State, FVector LocalUp, FVector Facing, bool bMoving, bool bFleeing)
{
    Facing = FVector::VectorPlaneProject(Facing, LocalUp).GetSafeNormal();
    if (!Facing.IsNearlyZero()) SetActorRotation(FRotationMatrix::MakeFromXZ(Facing, LocalUp).Rotator());
    UAnimSequence* Desired = bMoving ? (bFleeing ? Run.Get() : Walk.Get()) : Idle.Get();
    if (Desired && Desired != Playing)
    {
        Mesh->PlayAnimation(Desired, true);
        Playing = Desired;
    }
    Label->SetText(FText::FromString(State));
    Label->SetTextRenderColor(bFleeing ? FColor(255, 190, 85) : FColor(195, 225, 255));
}

AIBS5WorldLife::AIBS5WorldLife()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("LifeRoot")));
    Ambience = CreateDefaultSubobject<UAudioComponent>(TEXT("LocalAmbience"));
    Ambience->SetupAttachment(RootComponent);
    Ambience->bAutoActivate = false;
    Ambience->bAutoDestroy = false;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PostMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    for (int32 Index = 0; Index < 3; ++Index)
    {
        UPointLightComponent* Light = CreateDefaultSubobject<UPointLightComponent>(*FString::Printf(TEXT("LocalStreetLight%d"), Index));
        Light->SetupAttachment(RootComponent);
        Light->SetMobility(EComponentMobility::Movable);
        Light->SetIntensity(1800.0f);
        Light->SetAttenuationRadius(800.0f);
        Light->SetLightColor(FLinearColor(1.0f, 0.72f, 0.4f));
        Light->SetCastShadows(false);
        Light->SetVisibility(false);
        StreetLights.Add(Light);
        UStaticMeshComponent* Post = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("LampPost%d"), Index));
        Post->SetupAttachment(RootComponent);
        Post->SetStaticMesh(PostMesh.Object);
        Post->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Post->SetWorldScale3D(FVector(0.08, 0.08, 4.0));
        Post->SetVisibility(false);
        LampPosts.Add(Post);
    }
}

void AIBS5WorldLife::BeginPlay()
{
    Super::BeginPlay();
    // Reuse the level's lighting and its art configuration.
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        if (!Sun || It->GetLightComponent()->Intensity > Sun->GetLightComponent()->Intensity) Sun = *It;
    }
    if (Sun)
    {
        OriginalSunIntensity = Sun->GetLightComponent()->Intensity;
        OriginalSunRotation = Sun->GetActorRotation();
        Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    }
    for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
    {
        SkyLight = It->GetLightComponent();
        OriginalSkyIntensity = SkyLight->Intensity;
        break;
    }
    if (bEnablePersistence) LoadWorld();
    if (GetWorld()->GetAudioDevice())
    {
        AmbienceWave = NewObject<USoundWaveProcedural>(this);
        AmbienceWave->SetSampleRate(22050);
        AmbienceWave->NumChannels = 1;
        AmbienceWave->Duration = INDEFINITELY_LOOPING_DURATION;
        Ambience->SetSound(AmbienceWave);
        Ambience->SetVolumeMultiplier(0.08f);
    }
}

void AIBS5WorldLife::ConfigurePlanet(FVector Center, bool bInSpherical)
{
    if (Center.ContainsNaN()) return;
    PlanetCenter = Center;
    bSpherical = bInSpherical;
}

void AIBS5WorldLife::SetObserver(APawn* Pawn) { Observer = Pawn; }

void AIBS5WorldLife::SetCitizenAppearance(USkeletalMesh* Appearance)
{
    CitizenAppearance = Appearance;
    for (AIBS5Citizen* Citizen : Citizens)
        if (IsValid(Citizen)) Citizen->SetAppearance(Appearance);
}

void AIBS5WorldLife::SetRegionContext(FName RegionId, FVector GroundCenter, FVector LocalUp, bool bCollisionReady)
{
    const bool bValid = !RegionId.IsNone() && !GroundCenter.ContainsNaN() && !LocalUp.ContainsNaN() && !LocalUp.IsNearlyZero();
    const bool bChanged = RegionId != ActiveRegion;
    if (bChanged) ClearCitizens();
    ActiveRegion = RegionId;
    bRegionReady = bValid && bCollisionReady;
    if (!bRegionReady)
    {
        ClearCitizens();
        return;
    }
    RegionCenter = GroundCenter;
    Up = LocalUp.GetSafeNormal();
    East = FVector::CrossProduct(FVector::UpVector, Up).GetSafeNormal();
    if (East.IsNearlyZero()) East = FVector::ForwardVector;
    North = FVector::CrossProduct(Up, East).GetSafeNormal();
    Regions.FindOrAdd(ActiveRegion);
    if (Citizens.IsEmpty()) SpawnCitizens();
}

FVector AIBS5WorldLife::LocalUpAt(FVector Position) const
{
    return bSpherical ? (Position - PlanetCenter).GetSafeNormal(UE_SMALL_NUMBER, Up) : Up;
}

bool AIBS5WorldLife::GroundPoint(FVector Candidate, FVector& Result) const
{
    const FVector GroundUp = LocalUpAt(Candidate);
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(S5Ground), false, this);
    for (const AIBS5Citizen* Citizen : Citizens) Params.AddIgnoredActor(Citizen);
    if (!GetWorld()->LineTraceSingleByObjectType(Hit, Candidate + GroundUp * 350.0, Candidate - GroundUp * 700.0,
        FCollisionObjectQueryParams(ECC_WorldStatic), Params)) return false;
    if (FVector::DotProduct(Hit.Normal, GroundUp) < 0.65) return false;
    Result = Hit.ImpactPoint + GroundUp * 92.0;
    return true;
}

void AIBS5WorldLife::SpawnCitizens()
{
    const int32 Count = FMath::Clamp(CitizenBudget, 3, 6);
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const double Angle = Index * UE_TWO_PI / Count;
        FVector Position;
        if (!GroundPoint(RegionCenter + East * (250.0 + 130.0 * Index) + North * FMath::Sin(Angle) * 300.0, Position)) continue;
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        AIBS5Citizen* Citizen = GetWorld()->SpawnActor<AIBS5Citizen>(Position, FRotationMatrix::MakeFromXZ(East, Up).Rotator(), Params);
        if (Citizen)
        {
            Citizen->SetAppearance(CitizenAppearance);
            Citizens.Add(Citizen);
        }
    }
}

void AIBS5WorldLife::ClearCitizens()
{
    for (AIBS5Citizen* Citizen : Citizens) if (IsValid(Citizen)) Citizen->Destroy();
    Citizens.Reset();
    for (UPointLightComponent* Light : StreetLights) Light->SetVisibility(false);
    for (UStaticMeshComponent* Post : LampPosts) Post->SetVisibility(false);
}

FVector AIBS5WorldLife::GetIncidentLocation() const
{
    return !Citizens.IsEmpty() && IsValid(Citizens[0]) ? Citizens[0]->GetActorLocation() : RegionCenter;
}

bool AIBS5WorldLife::RequestIncident()
{
    if (Citizens.Num() < 3 || !Incident.Begin(bRegionReady)) return false;
    IncidentRegion = ActiveRegion;
    DisturbancePoint = GetIncidentLocation();
    FIBS5RegionLife& Region = Regions.FindOrAdd(ActiveRegion);
    Region.Threat = 1.0;
    return true;
}

bool AIBS5WorldLife::TryResolveIncident(APawn* Helper)
{
    const bool bValidHelper = IsValid(Helper) && Helper == Observer.Get() && bRegionReady && ActiveRegion == IncidentRegion
        && !Citizens.IsEmpty() && IsValid(Citizens[0]);
    const double Distance = bValidHelper ? FVector::Distance(Helper->GetActorLocation(), GetIncidentLocation()) : 0.0;
    if (!Incident.Resolve(bValidHelper, Distance)) return false;
    FIBS5RegionLife& Region = Regions.FindOrAdd(ActiveRegion);
    Region.Threat = 0.0;
    ++Region.Rescued;
    if (bConsequencesEnabled && !Incident.PlayerCaused)
    {
        Reputation = IBS5::ClampReputation(Reputation + 5.0);
        Region.Reputation = IBS5::ClampReputation(Region.Reputation + 5.0);
    }
    PanicSeconds = 3.0;
    if (bEnablePersistence) SaveWorld();
    return true;
}

bool AIBS5WorldLife::ReportDisturbance(FVector Location, bool bPlayerCaused, float Severity, const FGuid& CausationId, bool bCivilianHarm)
{
    if (!bRegionReady || Location.ContainsNaN() || !FMath::IsFinite(Severity) || Severity <= 0.0f
        || !CausationId.IsValid() || SeenCauses.Contains(CausationId)) return false;
    if (FVector::DistSquared(Location, RegionCenter) > FMath::Square(5000.0)) return false;
    RecordedCauses.Add(CausationId);
    SeenCauses.Add(CausationId);
    DisturbancePoint = Location;
    PanicSeconds = 12.0;
    FIBS5RegionLife& Region = Regions.FindOrAdd(ActiveRegion);
    Region.Threat = FMath::Max(Region.Threat, static_cast<double>(FMath::Clamp(Severity, 1.0f, 5.0f)));
    // A witnessed hostile act has a delayed report; enemy violence never blames the player.
    if (bConsequencesEnabled && bPlayerCaused && bCivilianHarm && !Citizens.IsEmpty())
    {
        const IBS5::Consequence Consequence = IBS5::WitnessedDisturbance(bPlayerCaused, bConsequencesEnabled, Severity);
        Region.PendingHeat = FMath::Min(100.0, Region.PendingHeat + Consequence.Heat);
        Region.PendingReputation = FMath::Max(-100.0, Region.PendingReputation + Consequence.Reputation);
        if (Region.ReportDelay <= 0.0) Region.ReportDelay = 2.0;
    }
    // Helping a bystander in a player-created disturbance earns no incident reward.
    if (bPlayerCaused && bCivilianHarm && Incident.Stage == IBS5::IncidentStage::Active && IncidentRegion == ActiveRegion)
        Incident.PlayerCaused = true;
    return true;
}

void AIBS5WorldLife::UpdateCitizens(float DeltaSeconds)
{
    if (!bRegionReady) return;
    const bool bEventHere = Incident.Stage == IBS5::IncidentStage::Active && IncidentRegion == ActiveRegion;
    for (int32 Index = 0; Index < Citizens.Num(); ++Index)
    {
        AIBS5Citizen* Citizen = Citizens[Index];
        if (!IsValid(Citizen)) continue;
        const FVector CitizenUp = LocalUpAt(Citizen->GetActorLocation());
        const bool bVictim = bEventHere && Index == 0;
        const bool bRobber = bEventHere && !Incident.Rescue && Index == 1;
        const bool bFlee = PanicSeconds > 0.0 || (bEventHere && Index > 1);
        FVector Goal = RegionCenter + East * (250.0 + 130.0 * Index)
            + North * FMath::Sin(RoutineTime * 0.08 + Index) * 450.0;
        FString State = TEXT("Paseando");
        if (bVictim)
        {
            Goal = Citizen->GetActorLocation();
            State = Incident.Rescue ? TEXT("Necesita ayuda") : TEXT("Robo - puedes intervenir");
        }
        else if (bRobber)
        {
            Goal = GetIncidentLocation() + East * 130.0;
            State = TEXT("Ladron");
        }
        else if (bFlee)
        {
            const FVector Away = FVector::VectorPlaneProject(Citizen->GetActorLocation() - DisturbancePoint, CitizenUp).GetSafeNormal(UE_SMALL_NUMBER, North);
            Goal = RegionCenter + Away * 850.0 + East * Index * 60.0;
            State = TEXT("Buscando refugio");
        }
        FVector DesiredGround;
        FVector Move = FVector::VectorPlaneProject(Goal - Citizen->GetActorLocation(), CitizenUp);
        const bool bWantsMove = Move.SizeSquared() > FMath::Square(40.0);
        const double Speed = bFlee ? 340.0 : 110.0;
        const FVector OldLocation = Citizen->GetActorLocation();
        if (bWantsMove && GroundPoint(OldLocation + Move.GetClampedToMaxSize(Speed * DeltaSeconds), DesiredGround))
        {
            FHitResult Hit;
            Citizen->SetActorLocation(DesiredGround, true, &Hit);
        }
        const bool bMoved = FVector::DistSquared(OldLocation, Citizen->GetActorLocation()) > 0.1;
        if (bFlee && !bWantsMove) State = TEXT("Refugiado");
        Citizen->UpdatePresentation(State, CitizenUp, Move, bMoved, bFlee);
        if (Observer.IsValid())
        {
            const FVector TowardObserver = Observer->GetActorLocation() - Citizen->Label->GetComponentLocation();
            Citizen->Label->SetWorldRotation(FRotationMatrix::MakeFromXZ(TowardObserver, CitizenUp).Rotator());
            Citizen->Label->SetVisibility(TowardObserver.SizeSquared() < FMath::Square(2200.0));
        }
    }
}

void AIBS5WorldLife::UpdateLighting()
{
    const double Angle = (Clock.Phase - 0.5) * UE_TWO_PI;
    const FVector SunDirection = bSpherical ? FVector(FMath::Cos(Angle), -FMath::Sin(Angle), 0.0)
        : Up * FMath::Cos(Angle) + East * FMath::Sin(Angle);
    const double Altitude = FVector::DotProduct(SunDirection, Up);
    const float Daylight = static_cast<float>(FMath::Clamp((Altitude + 0.12) / 0.3, 0.0, 1.0));
    if (Sun)
    {
        Sun->SetActorRotation((-SunDirection).Rotation());
        Sun->GetLightComponent()->SetIntensity(FMath::Max(0.02f, OriginalSunIntensity * Daylight));
    }
    if (SkyLight) SkyLight->SetIntensity(OriginalSkyIntensity * FMath::Lerp(0.25f, 1.0f, Daylight));
    for (int32 Index = 0; Index < StreetLights.Num(); ++Index)
    {
        UPointLightComponent* Light = StreetLights[Index];
        Light->SetWorldLocation(RegionCenter + East * (Index - 1) * 600.0 + Up * 400.0);
        Light->SetVisibility(bRegionReady && !Citizens.IsEmpty() && Daylight < 0.4f);
        LampPosts[Index]->SetWorldLocation(RegionCenter + East * (Index - 1) * 600.0 + Up * 200.0);
        LampPosts[Index]->SetWorldRotation(FRotationMatrix::MakeFromXZ(East, Up).Rotator());
        LampPosts[Index]->SetVisibility(bRegionReady && !Citizens.IsEmpty());
    }
    UpdateAmbience(Daylight);
}

void AIBS5WorldLife::UpdateAmbience(float Daylight)
{
    if (!AmbienceWave) return;
    const bool bNear = bRegionReady && Observer.IsValid()
        && FVector::DistSquared(Observer->GetActorLocation(), RegionCenter) < FMath::Square(9000.0);
    if (!bNear)
    {
        Ambience->Stop();
        AmbienceWave->ResetAudio();
        return;
    }
    // Same engine procedural-audio path already used for wind and impacts.
    // Quiet synthetic breeze/chirps provide a day/night cue without an external pack.
    if (AmbienceWave->GetAvailableAudioByteCount() < 22050 * sizeof(int16))
    {
        TArray<int16> Samples;
        Samples.SetNumUninitialized(22050);
        for (int32 Index = 0; Index < Samples.Num(); ++Index)
        {
            const double Time = static_cast<double>(AmbientSample++) / 22050.0;
            const double Pulse = FMath::Max(0.0, FMath::Sin(Time * (Daylight > 0.5f ? 2.1 : 14.0)));
            const double Envelope = FMath::Pow(Pulse, 10.0);
            const double Frequency = Daylight > 0.5f ? 1800.0 + FMath::Sin(Time * 7.0) * 250.0 : 2800.0;
            const double Tone = FMath::Sin(Time * UE_TWO_PI * Frequency) * Envelope * 0.13;
            const double Breeze = (AmbientRandom.FRand() * 2.0 - 1.0) * 0.035;
            Samples[Index] = static_cast<int16>((Tone + Breeze) * 32767.0);
        }
        AmbienceWave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
    }
    if (!Ambience->IsPlaying()) Ambience->Play();
}

void AIBS5WorldLife::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0f) return;
    Clock.Tick(DeltaSeconds);
    Incident.Tick(DeltaSeconds);
    RoutineTime += DeltaSeconds;
    PanicSeconds = FMath::Max(0.0, PanicSeconds - DeltaSeconds);
    for (auto& Entry : Regions)
    {
        FIBS5RegionLife& Region = Entry.Value;
        if (Region.ReportDelay > 0.0)
        {
            Region.ReportDelay = FMath::Max(0.0, Region.ReportDelay - DeltaSeconds);
            if (Region.ReportDelay == 0.0)
            {
                if (bConsequencesEnabled)
                {
                    Region.Alert = FMath::Clamp(Region.Alert + Region.PendingHeat, 0.0, 100.0);
                    Region.Reputation = IBS5::ClampReputation(Region.Reputation + Region.PendingReputation);
                    Reputation = IBS5::ClampReputation(Reputation + Region.PendingReputation);
                }
                Region.PendingHeat = 0.0;
                Region.PendingReputation = 0.0;
            }
        }
        Entry.Value.Alert = FMath::Max(0.0, Entry.Value.Alert - DeltaSeconds * 0.8);
        if (Incident.Stage != IBS5::IncidentStage::Active || Entry.Key != IncidentRegion)
            Entry.Value.Threat = FMath::Max(0.0, Entry.Value.Threat - DeltaSeconds * 0.2);
    }
    if (Incident.Remaining <= 0.0 && bRegionReady) RequestIncident();
    UpdateCitizens(DeltaSeconds);
    LightTimer += DeltaSeconds;
    SaveTimer += DeltaSeconds;
    if (LightTimer >= 0.2)
    {
        UpdateLighting();
        LightTimer = 0.0;
    }
    if (bEnablePersistence && SaveTimer >= 30.0)
    {
        SaveWorld();
        SaveTimer = 0.0;
    }
}

void AIBS5WorldLife::SetDayLengthMinutes(double Minutes) { Clock.SetDuration(Minutes); }
void AIBS5WorldLife::SetSolarHour(double Hour) { if (FMath::IsFinite(Hour)) Clock.Phase = IBS5::Wrap01(Hour / 24.0); }
void AIBS5WorldLife::AdvanceSolarHours(float Hours) { Clock.AdvanceHours(Hours); }
void AIBS5WorldLife::SetClockPaused(bool bPaused) { Clock.Paused = bPaused; }
double AIBS5WorldLife::GetLocalSolarHour() const
{
    return Clock.LocalHour(bSpherical ? FMath::Atan2(Up.Y, Up.X) : 0.0);
}

FString AIBS5WorldLife::GetStatusText() const
{
    const double Hour = GetLocalSolarHour();
    const FIBS5RegionLife* Region = Regions.Find(ActiveRegion);
    const FString EventText = Incident.Stage == IBS5::IncidentStage::Active && IncidentRegion == ActiveRegion
        ? (Incident.Rescue ? TEXT("Civil necesita ayuda - acercate e interactua") : TEXT("Robo cercano - intervenir es opcional")) : TEXT("");
    return FString::Printf(TEXT("%02d:%02d | Alerta %d | Reputacion %.0f | %s"), static_cast<int32>(Hour),
        static_cast<int32>((Hour - FMath::FloorToDouble(Hour)) * 60.0), Region ? IBS5::AlertLevel(Region->Alert) : 0, Reputation, *EventText);
}

bool AIBS5WorldLife::SaveWorld()
{
    if (!bEnablePersistence || !bPersistenceCompatible || SaveSlot.IsEmpty()) return false;
    UIBS5WorldSave* Save = Cast<UIBS5WorldSave>(UGameplayStatics::CreateSaveGameObject(UIBS5WorldSave::StaticClass()));
    if (!Save) return false;
    Save->SolarPhase = Clock.Phase;
    Save->DayMinutes = Clock.DayMinutes;
    Save->bClockPaused = Clock.Paused;
    Save->bConsequencesEnabled = bConsequencesEnabled;
    Save->Reputation = Reputation;
    Save->Regions = Regions;
    Save->IncidentRegion = IncidentRegion;
    Save->IncidentStage = static_cast<uint8>(Incident.Stage);
    Save->IncidentRemaining = Incident.Remaining;
    Save->IncidentSequence = static_cast<int32>(Incident.Sequence);
    Save->bIncidentPlayerCaused = Incident.PlayerCaused;
    Save->bIncidentRescue = Incident.Rescue;
    Save->RecordedCauses = RecordedCauses;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0);
    if (!bSaved) ++SaveFailures;
    return bSaved;
}

bool AIBS5WorldLife::LoadWorld()
{
    if (!bEnablePersistence || SaveSlot.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SaveSlot, 0)) return false;
    const UIBS5WorldSave* Save = Cast<UIBS5WorldSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
    if (!Save || Save->Version < 0 || Save->Version > 1)
    {
        bPersistenceCompatible = false;
        return false;
    }
    Clock.Restore(Save->SolarPhase, Save->DayMinutes, Save->bClockPaused);
    bConsequencesEnabled = Save->bConsequencesEnabled;
    Reputation = IBS5::ClampReputation(Save->Reputation);
    Regions = Save->Regions;
    for (auto& Entry : Regions)
    {
        Entry.Value.Alert = FMath::IsFinite(Entry.Value.Alert) ? FMath::Clamp(Entry.Value.Alert, 0.0, 100.0) : 0.0;
        Entry.Value.Threat = FMath::IsFinite(Entry.Value.Threat) ? FMath::Clamp(Entry.Value.Threat, 0.0, 5.0) : 0.0;
        Entry.Value.Reputation = IBS5::ClampReputation(Entry.Value.Reputation);
        Entry.Value.ReportDelay = FMath::IsFinite(Entry.Value.ReportDelay) ? FMath::Clamp(Entry.Value.ReportDelay, 0.0, 2.0) : 0.0;
        Entry.Value.PendingHeat = FMath::IsFinite(Entry.Value.PendingHeat) ? FMath::Clamp(Entry.Value.PendingHeat, 0.0, 100.0) : 0.0;
        Entry.Value.PendingReputation = FMath::IsFinite(Entry.Value.PendingReputation) ? FMath::Clamp(Entry.Value.PendingReputation, -100.0, 0.0) : 0.0;
    }
    Incident.Stage = Save->IncidentStage <= static_cast<uint8>(IBS5::IncidentStage::Ignored)
        ? static_cast<IBS5::IncidentStage>(Save->IncidentStage) : IBS5::IncidentStage::Waiting;
    Incident.Remaining = FMath::IsFinite(Save->IncidentRemaining) ? FMath::Clamp(Save->IncidentRemaining, 0.0, 120.0) : 45.0;
    Incident.Sequence = static_cast<uint32>(FMath::Max(0, Save->IncidentSequence));
    Incident.PlayerCaused = Save->bIncidentPlayerCaused;
    Incident.Rescue = Save->bIncidentRescue;
    IncidentRegion = Save->IncidentRegion;
    RecordedCauses = Save->RecordedCauses;
    SeenCauses.Reset();
    for (const FGuid& Cause : RecordedCauses) if (Cause.IsValid()) SeenCauses.Add(Cause);
    return true;
}

void AIBS5WorldLife::EndPlay(const EEndPlayReason::Type Reason)
{
    if (bEnablePersistence) SaveWorld();
    ClearCitizens();
    Ambience->Stop();
    if (IsValid(Sun))
    {
        Sun->SetActorRotation(OriginalSunRotation);
        Sun->GetLightComponent()->SetIntensity(OriginalSunIntensity);
    }
    if (IsValid(SkyLight)) SkyLight->SetIntensity(OriginalSkyIntensity);
    Super::EndPlay(Reason);
}
