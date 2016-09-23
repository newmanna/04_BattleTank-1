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
	float TrackMaxDrivingForce = 40000000; // assume 40 tonne tank, and 1g acceleration

private:
	UTankTracks();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void ApplySidewaysForce();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);

	//bool HoverSuspension();
	//void ApplyDownwardsForce(FVector Force);

	void DriveTrack();

	float CurrentThrottle = 0;


	UPROPERTY(EditDefaultsOnly, Category = "HoverSuspension")
	float DesiredDistanceAboveGround = 195;
	
	UPROPERTY(EditDefaultsOnly, Category = "HoverSuspension")
	float TrackTotalHoverForce = -1500000;

	UPROPERTY(EditDefaultsOnly, Category = "HoverSuspension")
	float SocketOffsetFromTrackBottom = 50;

	FVector ForceInterped;

};
