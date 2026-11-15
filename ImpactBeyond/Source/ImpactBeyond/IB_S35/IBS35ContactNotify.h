#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "IBS35ContactNotify.generated.h"

UCLASS(meta=(DisplayName="IB S35 Contact"))
class IMPACTBEYOND_API UIBS35ContactNotify : public UAnimNotify
{
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
