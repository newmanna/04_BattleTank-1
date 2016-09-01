// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "TankBarrel.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent)) // makes this class BP spawnable and hides category collision
class BATTLETANK_API UTankBarrel : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	// -1 is max downwards movement, and +1 is max up movement
	void Elevate(float RelativeSpeed);
	

private:
	UPROPERTY(EditAnywhere, Category = Setup)
	float MaxDegreesPerSecond = 20;
	UPROPERTY(EditAnywhere, Category = Setup)
	float MinElevationDegrees = 0;
	UPROPERTY(EditAnywhere, Category = Setup)
	float MaxElevationDegrees = 45;
};
