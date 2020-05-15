// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
//#include "GameFramework/Pawn.h"
//#include "BehaviorTree/BehaviorTreeComponent.h"
//#include "BehaviorTree/BehaviorTreeTypes.h"
//#include "UObject/UObjectGlobals.h"
#include "FindRandomLocation.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class CPP_AI_API UFindRandomLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UFindRandomLocation(FObjectInitializer const& object_initializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& owner_comp, uint8* node_memory);
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (AllowPrivateAccess = "true"))
	float search_radius = 1500.0f;
};
