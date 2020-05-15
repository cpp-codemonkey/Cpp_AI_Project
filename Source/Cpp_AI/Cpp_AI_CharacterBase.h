#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cpp_AI_CharacterBase.generated.h"

UCLASS()
class CPP_AI_API ACpp_AI_CharacterBase : public ACharacter
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACpp_AI_CharacterBase();

	// Called every frame
	virtual void Tick(float const delta_time) override;

	virtual void attack_start();
	virtual void attack_end();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* right_fist_collision_box;
};