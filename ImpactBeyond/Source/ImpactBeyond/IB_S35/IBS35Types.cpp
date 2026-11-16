#include "IB_S35/IBS35Types.h"

bool FIBS35ObjectProfile::IsValid() const
{
    const auto Positive = [](float V) { return FMath::IsFinite(V) && V > 0; };
    return Positive(MassKg) && !HalfExtent.ContainsNaN() && HalfExtent.GetMin() >= 5
        && HalfExtent.GetMax() <= 500 && !LeftContact.ContainsNaN() && !RightContact.ContainsNaN()
        && Positive(GrabRange) && Positive(MaxCarryError) && Positive(CarrySpeed)
        && Positive(LiftSeconds) && Positive(MinThrowSpeed) && Positive(MaxThrowSpeed)
        && MaxThrowSpeed >= MinThrowSpeed;
}

FIBS35ObjectProfile FIBS35ObjectProfile::Preset(EIBS35ObjectClass Kind, int32 VariantIndex, float Scale)
{
    FIBS35ObjectProfile P;
    P.ObjectClass = Kind;
    switch (Kind)
    {
    case EIBS35ObjectClass::Car:
        if (VariantIndex == 1) { P.Variant = TEXT("Sedan"); P.MassKg = 1450; P.HalfExtent = FVector(225,95,75); }
        else if (VariantIndex == 2) { P.Variant = TEXT("Electric"); P.MassKg = 1650; P.HalfExtent = FVector(200,92,72); }
        break;
    case EIBS35ObjectClass::Container:
        P.Variant = TEXT("Container"); P.MassKg = 2200; P.HalfExtent = FVector(150,100,110); break;
    case EIBS35ObjectClass::Pole:
        P.Variant = TEXT("Pole"); P.MassKg = 85; P.HalfExtent = FVector(180,12,12); P.bMeleeTool = true; break;
    case EIBS35ObjectClass::Rock:
        P.Variant = TEXT("Rock"); P.MassKg = 350; P.HalfExtent = FVector(65,55,50); break;
    case EIBS35ObjectClass::LoosePart:
        P.Variant = TEXT("LoosePart"); P.MassKg = 45; P.HalfExtent = FVector(40,30,20); break;
    }
    const float S = FMath::IsFinite(Scale) ? FMath::Clamp(Scale, 0.5f, 1.5f) : 1;
    P.HalfExtent *= S;
    P.MassKg *= S * S * S;
    const float Width = FMath::Min(30.0, P.HalfExtent.Y * 0.75);
    P.LeftContact = FVector(-10, -Width, -P.HalfExtent.Z);
    P.RightContact = FVector(-10, Width, -P.HalfExtent.Z);
    P.LiftSeconds = FMath::Clamp(0.35f + P.MassKg / 6000, 0.35f, 0.9f);
    return P;
}
