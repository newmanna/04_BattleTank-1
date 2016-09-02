// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// #include "Tank.h"  ...as of lecture 133, ben wants us to use forward declarations instead of #includes in .h. it has ben move to the .cpp

#include "GameFramework/PlayerController.h"
#include "BattleTanksPlayerController.generated.h"


class ATank; // forward declaration
/**
 * 
 */
UCLASS()
class BATTLETANK_API ABattleTanksPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	ATank* GetControlledTank() const;


	void CheckIfPossessingTank();

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
