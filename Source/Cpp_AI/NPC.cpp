// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Cpp_AICharacter.h"

// Sets default values
ANPC::ANPC() :
	health(max_health),
	widget_component(CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar")))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	//GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	if (widget_component)
	{
		widget_component->SetupAttachment(RootComponent);
		widget_component->SetWidgetSpace(EWidgetSpace::Screen);
		widget_component->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
		static ConstructorHelpers::FClassFinder<UUserWidget> widget_class(TEXT("/Game/UI/HealthBar_BP"));
		if (widget_class.Succeeded())
		{
			widget_component->SetWidgetClass(widget_class.Class);
		}
	}
}

UAnimMontage* ANPC::get_montage() const
{
	return montage;
}

float ANPC::get_health() const
{
	return health;
}

float ANPC::get_max_health() const
{
	return max_health;
}

void ANPC::set_health(float const new_health)
{
	health = new_health;
	if (health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("You Win You Genius!!!"));
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
	
	// set material color of character
	UMaterialInstanceDynamic* const material_instance = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
	if (material_instance)
	{
		material_instance->SetVectorParameterValue("BodyColor", FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
		GetMesh()->SetMaterial(0, material_instance);
	}

	// attach delegates
	if (right_fist_collision_box)
	{
		right_fist_collision_box->OnComponentBeginOverlap.AddDynamic(this, &ANPC::on_attack_overlap_begin);
		right_fist_collision_box->OnComponentEndOverlap.AddDynamic(this, &ANPC::on_attack_overlap_end);
	}
}

void ANPC::on_attack_overlap_begin(
	UPrimitiveComponent* const overlapped_component, 
	AActor* const other_actor, 
	UPrimitiveComponent* other_component, 
	int const other_body_index, 
	bool const from_sweep, 
	FHitResult const& sweep_result)
{
	if (ACpp_AICharacter* const player = Cast<ACpp_AICharacter>(other_actor))
	{
		float const new_health = player->get_health() - player->get_max_health() * 0.07f;
		player->set_health(new_health);
	}
}

void ANPC::on_attack_overlap_end(
	UPrimitiveComponent* const overlapped_component, 
	AActor* const other_actor, 
	UPrimitiveComponent* other_component, 
	int const other_body_index)
{
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto const uw = Cast<UHealthBar>(widget_component->GetUserWidgetObject());
	if (uw)
	{
		uw->set_bar_value_percent(health / max_health);
	}
}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

APatrolPath* ANPC::get_patrol_path()
{
	return patrol_path;
}

void ANPC::melee_attack()
{
	if (montage)
	{
		PlayAnimMontage(montage);
	}
}

