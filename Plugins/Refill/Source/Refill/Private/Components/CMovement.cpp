// Fill out your copyright notice in the Description page of Project Settings.

#include "Refill.h"
#include "CMovement.h"


// Sets default values for this component's properties
UCMovement::UCMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCMovement::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());

	if (Character) {
		UInputComponent* PlayerInputComponent = Character->InputComponent;

		if (PlayerInputComponent) {
			// Set up gameplay key bindings
			PlayerInputComponent->BindAxis("MoveForward", this, &UCMovement::MoveForward);
			PlayerInputComponent->BindAxis("MoveRight", this, &UCMovement::MoveRight);
			// Default Camera view bindings
			PlayerInputComponent->BindAxis("CameraPitch", this, &UCMovement::AddControllerPitchInput);
			PlayerInputComponent->BindAxis("CameraYaw", this, &UCMovement::AddControllerYawInput);
		}
	}

	bPlayerHasMoved = false;

}

void UCMovement::MoveForward(const float Val){
	float SpeedFactor = 1.0f; // TODO Hardcoded

	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is forward
		FRotator Rotation = Character->Controller->GetControlRotation();
		// Limit pitch when walking or falling
		if (Character->GetCharacterMovement()->IsMovingOnGround() || Character->GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		// add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		Character->AddMovementInput(Direction, Val * SpeedFactor);
	}
}

void UCMovement::MoveRight(const float Val){
	float SpeedFactor = 1.0f; // TODO Hardcoded

	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Character->Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		Character->AddMovementInput(Direction, Val * SpeedFactor);
	}
}

void UCMovement::AddControllerPitchInput(const float Val){
	if (Character != nullptr) {
		Character->AddControllerPitchInput(Val);
	}
}

void UCMovement::AddControllerYawInput(const float Val) {
	if (Character != nullptr) {
		Character->AddControllerYawInput(Val);
	}
}


// Called every frame
void UCMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

