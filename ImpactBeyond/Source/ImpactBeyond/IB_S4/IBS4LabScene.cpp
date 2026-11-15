#include "IBS4LabScene.h"
#include "IBS4LabPawn.h"
#include "IBS4PowerComponent.h"
#include "IBS4PreparedTarget.h"
#include "IBS4Threat.h"
#include "IBS4PresentationComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AIBS4LabScene::AIBS4LabScene()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("RobotFeedback"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Cube = Mesh.Object;
}
void AIBS4LabScene::BeginPlay()
{
    Super::BeginPlay();
    const auto Box = [this](FVector Position, FVector Size, bool Physics)
    {
        auto* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Position, FRotator::ZeroRotator);
        if (!Actor) return;
        Actor->SetOwner(this);
        auto* Mesh = Actor->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(Cube); Mesh->SetWorldScale3D(Size / 100);
        Mesh->SetCollisionProfileName(Physics ? TEXT("PhysicsActor") : TEXT("BlockAll"));
        if (Physics)
        {
            Mesh->ComponentTags.Add(TEXT("IBS4Grabbable"));
            Mesh->SetUseCCD(true); Mesh->SetMassOverrideInKg(NAME_None, 40);
            Mesh->SetSimulatePhysics(true);
        }
    };
    // Deliberately leave the 500x500 patch footprint empty; no invisible floor
    // covers the crater after its tiles descend.
    Box(FVector(-800, 0, -50), FVector(2400, 3600, 100), false);
    Box(FVector(1700, 0, -50), FVector(1600, 3600, 100), false);
    Box(FVector(650, -1025, -50), FVector(500, 1550, 100), false);
    Box(FVector(650, 1025, -50), FVector(500, 1550, 100), false);
    // Physical perimeter prevents the laboratory from being mistaken for an open world.
    Box(FVector(-2050, 0, 150), FVector(100, 3800, 300), false);
    Box(FVector(2550, 0, 150), FVector(100, 3800, 300), false);
    Box(FVector(250, -1850, 150), FVector(4500, 100, 300), false);
    Box(FVector(250, 1850, 150), FVector(4500, 100, 300), false);
    const auto Target = [this](EIBS4TargetKind Kind, FName Id, FVector Position)
    {
        auto* Actor = GetWorld()->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), FTransform(Position), this);
        if (!Actor) return static_cast<AIBS4PreparedTarget*>(nullptr);
        Actor->Kind = Kind; Actor->ObjectId = Id;
        UGameplayStatics::FinishSpawningActor(Actor, FTransform(Position));
        Targets.Add(Actor);
        Actor->Response->OnResponse.AddDynamic(this, &AIBS4LabScene::Incident);
        return Actor;
    };
    Target(EIBS4TargetKind::Wall, TEXT("Wall01"), FVector(1400, 0, 0));
    Target(EIBS4TargetKind::Crater, TEXT("Crater01"), FVector(650, 0, 0));
    Target(EIBS4TargetKind::Glass, TEXT("Glass01"), FVector(1000, 650, 0));
    Target(EIBS4TargetKind::Door, TEXT("Door01"), FVector(1400, 650, 0));
    Robot = Target(EIBS4TargetKind::Robot, TEXT("RobotCannon01"), FVector(1200, -1100, 0));
    for (int32 I = 0; I < 3; ++I)
    {
        auto* Coupon = Target(EIBS4TargetKind::MaterialCoupon, FName(*FString::Printf(TEXT("Material%d"), I)), FVector(1900, -900 + I * 350, 0));
        if (Coupon) Coupon->Response->Surface = static_cast<EIBS4Surface>(I);
    }
    for (int32 I = 0; I < 5; ++I) Box(FVector(200 + I * 170, -650, 65), FVector(70, 70, 100), true);
    auto* Hero = Cast<AIBS4LabPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
    const auto Dress = [this](ACharacter* Character)
    {
        if (!HumanMesh) return;
        Character->GetMesh()->SetSkeletalMeshAsset(HumanMesh);
        Character->GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
        if (HumanAnim) Character->GetMesh()->SetAnimInstanceClass(HumanAnim);
    };
    if (Hero) Dress(Hero);
    for (int32 I = 0; I < 3; ++I)
    {
        const FVector Position = I == 2 ? FVector(-1100, 0, 100) : FVector(-700 + I * 700, 1100, 100);
        auto* Threat = GetWorld()->SpawnActorDeferred<AIBS4Threat>(AIBS4Threat::StaticClass(), FTransform(Position), this);
        if (!Threat) continue;
        Threat->bTank = I == 2; Threat->Target = Hero;
        UGameplayStatics::FinishSpawningActor(Threat, FTransform(Position));
        Threats.Add(Threat);
        if (I < 2) Dress(Threat);
        else
        {
            bUsingTankProxy = !TankMesh;
            Threat->TankVisual->SetStaticMesh(TankMesh ? TankMesh.Get() : Cube.Get());
            Threat->TankVisual->SetRelativeScale3D(TankMesh ? TankScale : FVector(2.5, 1.8, 1));
            // This cube is a labelled test launcher, not a claimed tank model.
        }
    }
    UE_LOG(LogTemp, Display, TEXT("IBS4 lab created: targets=%d threats=%d humanMesh=%s tankProxy=%d"), Targets.Num(), Threats.Num(), *GetNameSafe(HumanMesh), bUsingTankProxy);
}
void AIBS4LabScene::Incident(const FIBS4Hit& Hit, float Delta)
{
    auto* Hero = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Delta >= 0 || Hit.Instigator != Hero) return;
    for (AIBS4Threat* Threat : Threats) if (IsValid(Threat)) Threat->ReportIncident(Hit.AttackId, Hit.Instigator, true);
    bRobotAlert = true;
}
void AIBS4LabScene::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!IsValid(Robot) || !Robot->bCannonEnabled || !bRobotAlert) { RobotFireAt = -1; return; }
    auto* Hero = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Hero) return;
    const double Now = GetWorld()->GetTimeSeconds();
    const FVector Start = Robot->GetPart()->GetComponentLocation();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Robot), false, Robot);
    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, Hero->GetActorLocation(), ECC_Visibility, Query) || Hit.GetActor() != Hero)
    { RobotFireAt = -1; return; }
    auto* Feedback = FindComponentByClass<UIBS4PresentationComponent>();
    if (RobotFireAt < 0 && Now >= RobotNextWarning)
    {
        RobotFireAt = Now + 1.5;
        if (Feedback) Feedback->Beam(Start, Hero->GetActorLocation(), true);
    }
    if (RobotFireAt >= 0 && Now >= RobotFireAt)
    {
        // Recheck capability at the actual fire edge; a detached cannon cannot fire.
        if (Robot->bCannonEnabled) if (auto* Receiver = Hero->FindComponentByClass<UIBS4ResponseComponent>())
            Receiver->Receive(UIBS4ResponseComponent::MakeHit(GetWorld(), Robot, EIBS4Weapon::Bullet, 0, Hit));
        if (Feedback) Feedback->Impact(Hit.ImpactPoint, EIBS4Surface::Metal, false);
        RobotFireAt = -1; RobotNextWarning = Now + 3;
    }
}
bool AIBS4LabScene::SaveLab()
{
    auto* Save = Cast<UIBS4LabSave>(UGameplayStatics::CreateSaveGameObject(UIBS4LabSave::StaticClass()));
    if (!Save) return false;
    for (AIBS4PreparedTarget* Target : Targets) if (IsValid(Target)) Save->Broken.Add(Target->ObjectId, Target->bBroken);
    if (IsValid(Robot) && Robot->bBroken) Save->DetachedCannon = Robot->GetPart()->GetComponentTransform();
    return UGameplayStatics::SaveGameToSlot(Save, TEXT("IBS4_ResponseLab_v1"), 0);
}
bool AIBS4LabScene::LoadLab()
{
    auto* Save = Cast<UIBS4LabSave>(UGameplayStatics::LoadGameFromSlot(TEXT("IBS4_ResponseLab_v1"), 0));
    if (!Save || Save->Version != 1) return false;
    bool Complete = true;
    for (AIBS4PreparedTarget* Target : Targets)
    {
        if (!IsValid(Target)) continue;
        const bool* Broken = Save->Broken.Find(Target->ObjectId);
        if (!Broken) { Complete = false; continue; }
        // Restore intact state first to clear accumulated damage and physical debris.
        if (!Target->ResetTarget()) { Complete = false; continue; }
        if (*Broken) Target->SetBrokenState(true);
    }
    // The cannon respawns at the prepared joint rather than teleporting a saved
    // physics body into occupied space. Exact loose-body persistence is deferred.
    bRobotAlert = false; RobotFireAt = -1;
    for (AIBS4Threat* Threat : Threats) if (IsValid(Threat)) Threat->ClearAlert();
    return Complete;
}
void AIBS4LabHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* Pawn = Cast<AIBS4LabPawn>(GetOwningPawn());
    if (!Pawn || !Canvas) return;
    DrawText(TEXT("S4 RESPONSE LAB | PROXY ART | NO NIAGARA PACK VERIFIED"), FLinearColor::Yellow, 24, 22);
    DrawText(TEXT("WASD / left stick move | Mouse / right stick aim | Space-Ctrl / R1-L1 height"), FLinearColor::White, 24, 46);
    DrawText(TEXT("LMB / R2 laser or throw | E / Square grab-release | Q / Triangle punch"), FLinearColor::White, 24, 66);
    DrawText(TEXT("F / R3 explicit flight-land | G / Options test alert | R / Touchpad reset"), FLinearColor::White, 24, 86);
    DrawText(FString::Printf(TEXT("H01 HP %.0f | air intent %s | hits %d | pushes %d | cargo %s"), Pawn->Response->Health,
        Pawn->Flight->bFlightIntent ? TEXT("YES") : TEXT("LAND"), Pawn->Response->AcceptedHits, Pawn->Response->AppliedPushes,
        Pawn->Powers->IsHolding() ? TEXT("HELD") : TEXT("FREE")), FLinearColor::Green, 24, 112);
    float Row = 138;
    for (TActorIterator<AIBS4Threat> It(GetWorld()); It; ++It)
    {
        DrawText(FString::Printf(TEXT("%s: %s | shots %d"), It->bTank ? TEXT("TANK TEST LAUNCHER") : TEXT("OFFICER"),
            *It->StateLabel, It->ShotsFired), FLinearColor::Yellow, 24, Row);
        Row += 20;
    }
    DrawLine(Canvas->ClipX / 2 - 5, Canvas->ClipY / 2, Canvas->ClipX / 2 + 5, Canvas->ClipY / 2, FLinearColor::White);
    DrawLine(Canvas->ClipX / 2, Canvas->ClipY / 2 - 5, Canvas->ClipX / 2, Canvas->ClipY / 2 + 5, FLinearColor::White);
}
AIBS4LabGameMode::AIBS4LabGameMode()
{
    DefaultPawnClass = AIBS4LabPawn::StaticClass();
    HUDClass = AIBS4LabHUD::StaticClass();
}
