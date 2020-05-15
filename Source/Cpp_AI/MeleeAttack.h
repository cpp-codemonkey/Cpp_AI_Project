#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "NPC.h"
#include "MeleeAttack.generated.h"

UCLASS()
class CPP_AI_API UMeleeAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UMeleeAttack(FObjectInitializer const& object_initializer);

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& owner_comp, uint8* node_memory) override;
private:
	bool montage_has_finished(ANPC* const npc);
};
