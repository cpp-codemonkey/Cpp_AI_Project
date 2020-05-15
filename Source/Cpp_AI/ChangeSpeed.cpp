// Fill out your copyright notice in the Description page of Project Settings.


#include "ChangeSpeed.h"
#include "NPC.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UChangeSpeed::UChangeSpeed()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Change Speed");
}

void UChangeSpeed::OnBecomeRelevant(UBehaviorTreeComponent& owner_comp, uint8* node_memory)
{
	Super::OnBecomeRelevant(owner_comp, node_memory);
	auto const cont = owner_comp.GetAIOwner();
	ANPC* const npc = Cast<ANPC>(cont->GetPawn());
	npc->GetCharacterMovement()->MaxWalkSpeed = speed;
}

FString UChangeSpeed::GetStaticServiceDescription() const
{
	return FString("Change the NPC speed");
}
