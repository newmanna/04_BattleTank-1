// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankAimingComponent.h"
#include "BattleTanksPlayerController.h"


void ABattleTanksPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!GetPawn()) { return; } // suggestion from a user, prevents crash when opening player controller BP
	auto AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
	if (!ensure(AimingComponent)) { return; }
		FoundAimingComponent(AimingComponent);
}


void ABattleTanksPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTowardsCrosshair();
}


void ABattleTanksPlayerController::AimTowardsCrosshair()
{
	if (!GetPawn()) { return; } // suggestion from a user, prevents crash when opening player controller BP
	auto AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
	if (!ensure(AimingComponent)) { return; }

	FVector HitLocation;
	if (GetSightRayHitLocation(HitLocation))
	{
		AimingComponent->AimAt(HitLocation);
	}
}


//Get world location of line trace through crosshair, returns true if hitting landscape
bool ABattleTanksPlayerController::GetSightRayHitLocation(FVector& HitLocation) const
{
	int32 ViewPortSizeX, ViewPortSizeY;
	GetViewportSize(ViewPortSizeX, ViewPortSizeY);
	auto ScreenLocation = FVector2D(CrossHairXLocation * ViewPortSizeX, CrossHairYLocation * ViewPortSizeY);

	// de-project the crosshair position in pixel coordinates
	FVector LookDirection;
	if (GetLookDirection(ScreenLocation, LookDirection))
	{
		GetLookVectorHitLocation(LookDirection, HitLocation);
	}
	return true;
}


bool ABattleTanksPlayerController::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const
{
	// deproject the screen position to world direction
	FVector CameraWorldLocation; // to be discarded
	return DeprojectScreenPositionToWorld(
		ScreenLocation.X,
		ScreenLocation.Y,
		CameraWorldLocation,
		LookDirection);
}


bool ABattleTanksPlayerController::GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const
{
	//int32 LineTraceRange = 100000;
	FVector Start = PlayerCameraManager->GetCameraLocation();
	FVector End = Start + LookDirection * LineTraceRange;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,							// OUT Parameter
		Start,								// Start vector
		End,								// End vector
		ECollisionChannel::ECC_Visibility);	// trace Type
		
	HitLocation = HitResult.Location;


	/*
	DrawDebugLine(
		GetWorld(),
		Start,					// TraceStart
		HitResult.Location,		// TraceEnd
		FColor(255, 0, 0),  	// Red Green Blue
		false, 1, 0,			// Ispersistent, lifetime, DepthPriority
		1);						// thickness

		*/
	return HitResult.bBlockingHit;
}
