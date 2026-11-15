#include "IBS4PresentationComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Sound/SoundWaveProcedural.h"

UIBS4PresentationComponent::UIBS4PresentationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UIBS4PresentationComponent::BeginPlay()
{
    Super::BeginPlay();
    const auto MakeMesh = [this](const TCHAR* Name, const TCHAR* Path)
    {
        auto* Mesh = NewObject<UStaticMeshComponent>(GetOwner(), Name);
        GetOwner()->AddInstanceComponent(Mesh);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Path));
        Mesh->SetupAttachment(GetOwner()->GetRootComponent());
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCastShadow(false);
        Mesh->RegisterComponent();
        Mesh->SetVisibility(false);
        return Mesh;
    };
    BeamMesh = MakeMesh(TEXT("S4BeamProxy"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    SparkMesh = MakeMesh(TEXT("S4ImpactProxy"), TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Audio = NewObject<UAudioComponent>(GetOwner(), TEXT("S4ImpactAudio"));
    GetOwner()->AddInstanceComponent(Audio);
    Audio->bAutoActivate = false;
    Audio->bAutoDestroy = false;
    Audio->SetupAttachment(GetOwner()->GetRootComponent());
    Audio->RegisterComponent();
    Wave = NewObject<USoundWaveProcedural>(this);
    Wave->SetSampleRate(22050);
    Wave->NumChannels = 1;
    Wave->Duration = 0.15f;
    Audio->SetSound(Wave);
}
void UIBS4PresentationComponent::Beam(FVector Start, FVector End, bool Visible)
{
    if (!BeamMesh) return;
    BeamMesh->SetVisibility(Visible);
    if (Visible)
    {
        BeamMesh->SetWorldLocation((Start + End) * 0.5);
        BeamMesh->SetWorldRotation(FRotationMatrix::MakeFromZ(End - Start).Rotator());
        BeamMesh->SetWorldScale3D(FVector(0.012, 0.012, FVector::Distance(Start, End) / 100));
        BeamUntil = GetWorld()->GetTimeSeconds() + 0.11;
    }
    NiagaraBeam(Start, End, Visible);
}
void UIBS4PresentationComponent::Impact(FVector Point, EIBS4Surface Surface, bool Heavy)
{
    if (!SparkMesh) return;
    const double Now = GetWorld()->GetTimeSeconds();
    SparkMesh->SetWorldLocation(Point);
    SparkMesh->SetWorldScale3D(FVector(Heavy ? 0.65 : 0.07));
    SparkMesh->SetVisibility(true);
    SparkUntil = Now + (Heavy ? 0.18 : 0.06);
    if (Volume > 0 && Now >= NextAudio)
    {
        NextAudio = Now + 0.12;
        Audio->Stop(); Wave->ResetAudio();
        TArray<int16> PCM; PCM.SetNumUninitialized(3308);
        FRandomStream Noise(67 + static_cast<int32>(Surface));
        const float Frequency = Surface == EIBS4Surface::Metal ? 1700 : Surface == EIBS4Surface::Glass ? 3200 : 180;
        for (int32 I = 0; I < PCM.Num(); ++I)
        {
            const float Time = I / 22050.f;
            const float Envelope = FMath::Exp(-Time * (Heavy ? 20.f : 45.f));
            PCM[I] = static_cast<int16>(12000 * Envelope * (0.55f * Noise.FRandRange(-1.f, 1.f) + 0.45f * FMath::Sin(2 * PI * Frequency * Time)));
        }
        Wave->QueueAudio(reinterpret_cast<const uint8*>(PCM.GetData()), PCM.Num() * sizeof(int16));
        Audio->SetWorldLocation(Point);
        Audio->SetVolumeMultiplier(FMath::Clamp(Volume, 0.f, 0.5f));
        Audio->Play();
        AudioUntil = Now + 0.17;
    }
    NiagaraImpact(Point, Surface, Heavy);
}
void UIBS4PresentationComponent::TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt, Type, Fn);
    const double Now = GetWorld()->GetTimeSeconds();
    if (BeamMesh && BeamMesh->IsVisible() && Now > BeamUntil)
    {
        BeamMesh->SetVisibility(false);
        NiagaraBeam(FVector::ZeroVector, FVector::ZeroVector, false);
    }
    if (SparkMesh && Now > SparkUntil) SparkMesh->SetVisibility(false);
    if (Audio && Now > AudioUntil && Audio->IsPlaying())
    {
        Audio->Stop();
        Wave->ResetAudio();
    }
}
