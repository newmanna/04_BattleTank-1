// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "BattleTanksPlayerController.h"


void ABattleTanksPlayerController::BeginPlay()
{
	Super::BeginPlay();

	auto ControlledTank = GetControlledTank();
	if (ControlledTank)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController possessing: %s"), *ControlledTank->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController NOT possessing a Tank"));
	}
}

void ABattleTanksPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimTowardsCrosshair();
}

ATank* ABattleTanksPlayerController::GetControlledTank() const
{
	return Cast<ATank>(GetPawn());
}


void ABattleTanksPlayerController::AimTowardsCrosshair()
{
	if (!GetControlledTank()) { return; }

	FVector HitLocation;
	if (GetSightRayHitLocation(HitLocation))
	{
		GetControlledTank()->AimAt(HitLocation);
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
	int32 LineTraceRange = 100000;
	FVector Start = PlayerCameraManager->GetCameraLocation();
	FVector End = Start + LookDirection * LineTraceRange;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,							// OUT Parameter
		Start,								// Start vector
		End,								// End vector
		ECollisionChannel::ECC_Visibility);	// trace Type
		
	HitLocation = HitResult.Location;

	DrawDebugLine(
		GetWorld(),
		Start,					// TraceStart
		HitResult.Location,		// TraceEnd
		FColor(255, 0, 0),  	// Red Green Blue
		false, 1, 0,			// Ispersistent, lifetime, DepthPriority
		1);						// thickness

	return HitResult.bBlockingHit;
}





/*
FVector CameraStart;
FRotator CameraRotation;
GetPlayerViewPoint(CameraStart, CameraRotation); // OUT parameters
FVector CameraEnd = CameraStart + CameraRotation.Vector() * 10000;

//Setup query parameters: SomeText???, complexCollision?, ActorToIgnore
FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());

//LineTrace
FHitResult Hit; // this stores the result
GetWorld()->LineTraceSingleByChannel(
Hit,								// OUT Parameter
CameraStart,						// Start vector
CameraEnd,							// End vector
ECollisionChannel::ECC_WorldStatic,	//Obj trace Type
TraceParameters						// setup parameters above
);

HitLocation = Hit.Location;

DrawDebugLine(
GetWorld(),
CameraStart,			// TraceStart
HitLocation,			// TraceEnd
FColor(255, 0, 0),  	// Red Green Blue
false, -1, 0,			//Ispersistent, lifetime, DepthPriority
15						// thickness
);

// Hit.IsValidBlockingHit()

return true;

*/