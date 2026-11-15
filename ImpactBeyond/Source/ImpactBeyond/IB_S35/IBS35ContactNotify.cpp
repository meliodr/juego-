#include "IB_S35/IBS35ContactNotify.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UIBS35ContactNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (MeshComp && MeshComp->GetOwner())
        if (auto* Punch = MeshComp->GetOwner()->FindComponentByClass<UIBS35PunchComponent>()) Punch->NotifyContact();
}
