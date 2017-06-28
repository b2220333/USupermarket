// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "GameFramework/Actor.h"
//#include "Obb.generated.h"

/**
 * The parameters for anything that can have a temperature
 */
 //USTRUCT()
struct FObjectOrientedBoundingBox
{
	//GENERATED_BODY()
public:
	FVector LowerLeftFrontCorner;
	FVector UpperLeftFrontCorner;
	FVector LowerRightFrontCorner;
	FVector UpperRightFrontCorner;
	FVector LowerLeftBackCorner;
	FVector UpperLeftBackCorner;
	FVector LowerRightBackCorner;
	FVector UpperRightBackCorner;
	FVector Origin;

	/** The lenght of each side*/
	FVector Dimensions;

	/** Half the length of each side*/
	FVector Extend;

	FObjectOrientedBoundingBox(AActor* Actor)
	{
		CalculateCorners(Actor);
		CalculateExtend();
	}

private:
	void CalculateCorners(AActor* Actor)
	{
		AStaticMeshActor* EmptyActor = Actor->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
		EmptyActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		for (size_t i = 0; i < 9; i++)
		{
			EmptyActor->AttachToActor(Actor, FAttachmentTransformRules::KeepWorldTransform);
			FVector RelativePositionInActor;

			switch (i)
			{
			case 0:
				// Lower left front
				RelativePositionInActor = FVector(-100, -100, -100);
				break;
			case 1:
				// Upper left front
				RelativePositionInActor = FVector(-100, -100, 100);
				break;
			case 2:
				// Lower right front
				RelativePositionInActor = FVector(-100, 100, -100);
				break;
			case 3:
				// Upper right front
				RelativePositionInActor = FVector(-100, 100, 100);
				break;
			case 4:
				// Lower left back
				RelativePositionInActor = FVector(100, -100, -100);
				break;
			case 5:
				// Upper left back
				RelativePositionInActor = FVector(100, -100, 100);
				break;
			case 6:
				// Lower right back
				RelativePositionInActor = FVector(100, 100, -100);
				break;
			case 7:
				// Upper right back
				RelativePositionInActor = FVector(100, 100, 100);
				break;
			case 8:
				// Origin
				RelativePositionInActor = FVector(0, 0, 0);
				break;
			}

			EmptyActor->SetActorRelativeLocation(RelativePositionInActor);
			EmptyActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			FVector WorldLocation = EmptyActor->GetActorLocation();

			switch (i)
			{
			case 0:
				// Lower left front
				LowerLeftFrontCorner = WorldLocation;
				break;
			case 1:
				// Upper left front
				UpperLeftFrontCorner = WorldLocation;
				break;
			case 2:
				// Lower right front
				LowerRightFrontCorner = WorldLocation;
				break;
			case 3:
				// Upper right front
				UpperRightFrontCorner = WorldLocation;
				break;
			case 4:
				// Lower left back
				LowerLeftBackCorner = WorldLocation;
				break;
			case 5:
				// Upper left back
				UpperLeftBackCorner = WorldLocation;
				break;
			case 6:
				// Lower right back
				LowerRightBackCorner = WorldLocation;
				break;
			case 7:
				// Upper right back
				UpperRightBackCorner = WorldLocation;
				break;
			case 8:
				// Origin
				Origin = WorldLocation;
				break;
			}


		}
		EmptyActor->Destroy();
	}

	void CalculateExtend()
	{
		float XValue = FVector::Dist(LowerLeftFrontCorner, LowerLeftBackCorner);
		float YValue = FVector::Dist(LowerLeftFrontCorner, LowerRightFrontCorner);
		float ZValue = FVector::Dist(LowerLeftFrontCorner, UpperLeftFrontCorner);
		Dimensions = FVector(XValue, YValue, ZValue);
		Extend = 0.5f * Dimensions;
	}
};