#pragma once


#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AttackNotifyState.generated.h"

UCLASS()
class CPP_AI_API UAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:	
	void NotifyBegin(USkeletalMeshComponent* const mesh, UAnimSequenceBase* const animation, float const total_duration) override;
	void NotifyEnd(USkeletalMeshComponent* const mesh, UAnimSequenceBase* const animation) override;
};