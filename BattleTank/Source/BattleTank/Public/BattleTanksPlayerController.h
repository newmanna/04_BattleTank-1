// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "BattleTanksPlayerController.generated.h"


class UTankAimingComponent; // forward declaration
/**
 * 
 */
UCLASS()
class BATTLETANK_API ABattleTanksPlayerController : public APlayerController
{
	GENERATED_BODY()



protected:
	// triggers event in BP when called, does not have to be implemented
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void FoundAimingComponent(UTankAimingComponent* AimCompRef);

	UFUNCTION() // has to be ufunction so it can be subscribed to OnDeath delegate
	void OnPossessedTankDeath();

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetPawn(APawn* InPawn) override;

	// Aim to crosshair & helper functions
	void AimTowardsCrosshair();
	bool GetSightRayHitLocation(FVector& HitLocation) const;
	bool GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const;
	bool GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const;

	UPROPERTY(EditDefaultsOnly)
	float CrossHairXLocation = 0.5;
	
	UPROPERTY(EditDefaultsOnly)
	float CrossHairYLocation = 0.733;	
	
	UPROPERTY(EditDefaultsOnly)
	float LineTraceRange = 100000;

};
