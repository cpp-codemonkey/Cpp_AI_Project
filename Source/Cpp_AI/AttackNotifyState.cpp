// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackNotifyState.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cpp_AI_CharacterBase.h"

void UAttackNotifyState::NotifyBegin(
	USkeletalMeshComponent* const mesh, 
	UAnimSequenceBase* const animation, 
	float const total_duration)
{
	if (mesh && mesh->GetOwner())
	{
		if (ACpp_AI_CharacterBase* const character = Cast<ACpp_AI_CharacterBase>(mesh->GetOwner()))
		{
			character->attack_start();
		}
	}
}

void UAttackNotifyState::NotifyEnd(
	USkeletalMeshComponent* const mesh, 
	UAnimSequenceBase* const animation)
{
	if (mesh && mesh->GetOwner())
	{
		if (ACpp_AI_CharacterBase* const character = Cast<ACpp_AI_CharacterBase>(mesh->GetOwner()))
		{
			character->attack_end();
		}
	}
}
