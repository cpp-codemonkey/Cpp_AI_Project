// Fill out your copyright notice in the Description page of Project Settings.


#include "StopInvestigating.h"
#include "NPC_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "blackboard_keys.h"

UStopInvestigating::UStopInvestigating(FObjectInitializer const& object_initializer)
{
	NodeName = TEXT("Stop Investigating");
}

EBTNodeResult::Type UStopInvestigating::ExecuteTask(UBehaviorTreeComponent& owner_comp, uint8* node_memory)
{
	// get the NPC controller and write false to the blackboard key "IsInvestigating"
	auto const cont = Cast<ANPC_AIController>(owner_comp.GetAIOwner());
	cont->get_blackboard()->SetValueAsBool(GetSelectedBlackboardKey(), false);

	// finish with success
	FinishLatentTask(owner_comp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
