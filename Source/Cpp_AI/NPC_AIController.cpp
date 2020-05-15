// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Cpp_AICharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "blackboard_keys.h"
#include "ai_tags.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

ANPC_AIController::ANPC_AIController(FObjectInitializer const& object_initializer)
{
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> obj(TEXT("BehaviorTree'/Game/AI/NPC_BT.NPC_BT'"));
	if (obj.Succeeded())
	{
		btree = obj.Object;
	}
	behavior_tree_component = object_initializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));
	blackboard = object_initializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComp"));
	setup_perception_system();
}

void ANPC_AIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(btree);
	behavior_tree_component->StartTree(*btree);
}

void ANPC_AIController::OnPossess(APawn* const pawn)
{
	Super::OnPossess(pawn);
	if (blackboard)
	{
		blackboard->InitializeBlackboard(*btree->BlackboardAsset);
	}
}

UBlackboardComponent* ANPC_AIController::get_blackboard() const
{
	return blackboard;
}

void ANPC_AIController::on_updated(TArray<AActor*> const& updated_actors)
{
	for (size_t x = 0; x < updated_actors.Num(); ++x)
	{
		FActorPerceptionBlueprintInfo info;
		GetPerceptionComponent()->GetActorsPerception(updated_actors[x], info);
		for (size_t k = 0; k < info.LastSensedStimuli.Num(); ++k)
		{
			FAIStimulus const stim = info.LastSensedStimuli[k];
			if (stim.Tag == tags::noise_tag)
			{
				get_blackboard()->SetValueAsBool(bb_keys::is_investigating, stim.WasSuccessfullySensed());
				get_blackboard()->SetValueAsVector(bb_keys::target_location, stim.StimulusLocation);
			}
			else if (stim.Type.Name == "Default__AISense_Sight")
			{
				get_blackboard()->SetValueAsBool(bb_keys::can_see_player, stim.WasSuccessfullySensed());
			}
		}
	}
}

void ANPC_AIController::setup_perception_system()
{
	// create and initialise sight configuration object
	sight_config = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (sight_config)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
		sight_config->SightRadius = 500.0f;
		sight_config->LoseSightRadius = sight_config->SightRadius + 25.0f;
		sight_config->PeripheralVisionAngleDegrees = 90.0f;
		sight_config->SetMaxAge(5.0f);
		sight_config->AutoSuccessRangeFromLastSeenLocation = 520.0f;
		sight_config->DetectionByAffiliation.bDetectEnemies = 
			sight_config->DetectionByAffiliation.bDetectFriendlies = 
			sight_config->DetectionByAffiliation.bDetectNeutrals = true;

		// add sight configuration component to perception component
		GetPerceptionComponent()->SetDominantSense(*sight_config->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*sight_config);
	}

	// create and initialise hearing config object
	hearing_config = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing config"));
	if (hearing_config)
	{
		hearing_config->HearingRange = 3000.0f;
		hearing_config->DetectionByAffiliation.bDetectEnemies =
			hearing_config->DetectionByAffiliation.bDetectFriendlies =
			hearing_config->DetectionByAffiliation.bDetectNeutrals = true;

		// add sight configuration component to perception component
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ANPC_AIController::on_updated);
		GetPerceptionComponent()->ConfigureSense(*hearing_config);
	}
}
