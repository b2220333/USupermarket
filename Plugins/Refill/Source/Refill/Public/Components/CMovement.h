// Handles the character movement

#pragma once

#include "Components/ActorComponent.h"
#include "CMovement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REFILL_API UCMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Handles moving forward/backward
	void MoveForward(const float Val);

	// Handles strafing Left/Right
	void MoveRight(const float Val);

	// Handles mouse x-axis input
	void AddControllerPitchInput(const float Val);

	// Handles mouse y-axis input
	void AddControllerYawInput(const float Val);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// The player character instance
	ACharacter* Character;	 
};
