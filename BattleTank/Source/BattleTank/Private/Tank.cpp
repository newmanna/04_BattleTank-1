// Fill äout your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "Projectile.h"
#include "TankBarrel.h"
#include "TankAimingComponent.h" // notice how this has to be under #include "BattleTank.h"
#include "TankMovementComponent.h"
#include "Tank.h"


// Sets default values
ATank::ATank()
{
	PrimaryActorTick.bCanEverTick = false;
	UE_LOG(LogTemp, Warning, TEXT("DERP Cpp Tank Constructor"))

}


void ATank::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("DERP Cpp Tank Begin Play"))

	Super::BeginPlay();

	auto* TankAimingComponent = this->FindComponentByClass<UTankAimingComponent>();
	if (TankAimingComponent)
	{
		this->TankAimingComponent = TankAimingComponent;
	}

}


void ATank::AimAt(FVector HitLocation)
{
	if (!ensure(TankAimingComponent)) { return; }
	TankAimingComponent->AimAt(HitLocation, LaunchSpeed);
}


void ATank::Fire()
{
	if (!ensure(Barrel)) { return; }
	bool isReloaded = (FPlatformTime::Seconds() - LastFireTime) > ReloadTimeInSeconds;

	if (isReloaded)
	{
		//spawn projectile
		auto Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileBlueprint,
			Barrel->GetSocketLocation(FName("Projectile")),
			Barrel->GetSocketRotation(FName("Projectile"))
			);

		Projectile->LaunchProjectile(LaunchSpeed);
		LastFireTime = FPlatformTime::Seconds();
	}
}










/*

//testing
FVector Start;
FVector End;
FHitResult HitResult;

for(int X = 0; X < 250; X++)
{
for(int Y = 0; Y < 250; Y++)
{
Start = FVector(X * 100, Y * 100, 100);
End = Start + FVector(0, 0, 200);

DrawDebugLine(
GetWorld(),
Start,					// TraceStart
End,					// TraceEnd
FColor(255, 0, 0),  	// Red Green Blue
false, -1, 0,			// Ispersistent, lifetime, DepthPriority
1);						// thickness


}
}

*/