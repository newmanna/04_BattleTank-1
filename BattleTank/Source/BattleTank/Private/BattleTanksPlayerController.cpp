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
		UE_LOG(LogTemp, Warning, TEXT("HitLocation: %s"), *HitLocation.ToString());
	}
}




//Get world location of line trace through crosshair, returns true if hitting landscape
bool ABattleTanksPlayerController::GetSightRayHitLocation(FVector& HitLocation) const
{
	FVector CameraStart;
	FRotator CameraRotation;
	PlayerCameraManager->GetCameraViewPoint(
		CameraStart, 
		CameraRotation); // OUT parameters
	
	FVector CameraEnd = CameraStart + (CameraRotation.Vector() * 1000);

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


	bool BlockingHit = Hit.bBlockingHit;

	return BlockingHit;
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