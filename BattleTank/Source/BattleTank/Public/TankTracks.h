// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "TankTracks.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class BATTLETANK_API UTankTracks : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = Input)
	void SetThrottle(float Throttle);
	
	// Max force per track in Newtons
	UPROPERTY(EditDefaultsOnly)
	float TrackMaxDrivingForce = 45000000; // assume 40 tonne tank, and 1g acceleration
};
