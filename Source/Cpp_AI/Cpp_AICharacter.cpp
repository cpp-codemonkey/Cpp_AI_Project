// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Cpp_AICharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "ai_tags.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "HealthBar.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "NPC.h"

//////////////////////////////////////////////////////////////////////////
// ACpp_AICharacter

ACpp_AICharacter::ACpp_AICharacter() : 
	health(max_health), 
	widget_component(CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthValue")))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	setup_stimulus();

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

//////////////////////////////////////////////////////////////////////////
// Input

void ACpp_AICharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACpp_AICharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACpp_AICharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACpp_AICharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACpp_AICharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACpp_AICharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACpp_AICharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACpp_AICharacter::OnResetVR);

	// exit game functionality
	PlayerInputComponent->BindAction("ExitGame", IE_Pressed, this, &ACpp_AICharacter::on_exit_game);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACpp_AICharacter::on_attack);
	PlayerInputComponent->BindAction("Distract", IE_Pressed, this, &ACpp_AICharacter::on_distract);
}

void ACpp_AICharacter::on_exit_game()
{
	GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
}

void ACpp_AICharacter::setup_stimulus()
{
	stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("stimulus"));
	stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	stimulus->RegisterWithPerceptionSystem();
}

void ACpp_AICharacter::on_attack()
{
	if (montage)
	{
		PlayAnimMontage(montage);
	}
}

void ACpp_AICharacter::on_distract()
{
	if (distraction_sound)
	{
		FVector const loc = GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), distraction_sound, loc);
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), loc, 1.0f, this, 0.0f, tags::noise_tag);
	}
}

void ACpp_AICharacter::on_attack_overlap_begin(
	UPrimitiveComponent* const overlapped_component, 
	AActor* const other_actor, 
	UPrimitiveComponent* other_component, 
	int const other_body_index, 
	bool const from_sweep, 
	FHitResult const& sweep_result)
{
	if (ANPC* const npc = Cast<ANPC>(other_actor))
	{
		float const new_health = npc->get_health() - npc->get_max_health() * 0.1f;
		npc->set_health(new_health);
	}
}

void ACpp_AICharacter::on_attack_overlap_end(
	UPrimitiveComponent* const overlapped_component, 
	AActor* const other_actor, 
	UPrimitiveComponent* other_component, 
	int const other_body_index)
{
}

void ACpp_AICharacter::BeginPlay()
{
	Super::BeginPlay();
	// limit pitch of camera
	APlayerCameraManager* const cam_manager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	cam_manager->ViewPitchMin = -50.0f;
	cam_manager->ViewPitchMax = 10.0f;

	// set material color of character
	UMaterialInstanceDynamic* const material_instance = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
	if (material_instance)
	{
		material_instance->SetVectorParameterValue("BodyColor", FLinearColor(0.0f, 1.0f, 0.0f, 1.0f));
		GetMesh()->SetMaterial(0, material_instance);
	}

	// attach delegates to the collision box
	if (right_fist_collision_box)
	{
		right_fist_collision_box->OnComponentBeginOverlap.AddDynamic(this, &ACpp_AICharacter::on_attack_overlap_begin);
		right_fist_collision_box->OnComponentEndOverlap.AddDynamic(this, &ACpp_AICharacter::on_attack_overlap_end);
	}
}

float ACpp_AICharacter::get_health() const
{
	return health;
}

float ACpp_AICharacter::get_max_health() const
{
	return max_health;
}

void ACpp_AICharacter::set_health(float const new_health)
{
	health = new_health;
	if (health <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("You lose sucker!!!"));
		auto const controller = UGameplayStatics::GetPlayerController(this, 0);
		controller->SetCinematicMode(true, false, false, true, true);
		GetMesh()->SetSimulatePhysics(true);
		GetMovementComponent()->MovementState.bCanJump = false;
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}
}

void ACpp_AICharacter::Tick(float const delta_time)
{
	Super::Tick(delta_time);
	auto const uw = Cast<UHealthBar>(widget_component->GetUserWidgetObject());
	if (uw)
	{
		uw->set_bar_value_percent(health / max_health);
	}
}

void ACpp_AICharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACpp_AICharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ACpp_AICharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ACpp_AICharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACpp_AICharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACpp_AICharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACpp_AICharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
