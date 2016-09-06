// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankBarrel.h"
#include "TankTurret.h"
#include "Projectile.h"
#include "TankAimingComponent.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UTankAimingComponent::BeginPlay()
{
	Super::BeginPlay();

	// so that first fire is after initial reload
	LastFireTime = FPlatformTime::Seconds();
}


void UTankAimingComponent::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
	Barrel = BarrelToSet;
	Turret = TurretToSet;
}


void UTankAimingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	//UE_LOG(LogTemp, Warning, TEXT("test"))
	if (RoundsLeft <= 0)
	{
		FiringState = EFiringState::OutOfAmmo;
	}
	else if ((FPlatformTime::Seconds() - LastFireTime) < ReloadTimeInSeconds)
	{
		FiringState = EFiringState::Reloading;
	}
	else if (isBarrelMoving())
	{
		FiringState = EFiringState::Aiming;
	}
	else
	{
		FiringState = EFiringState::Locked;
	}

}

int32 UTankAimingComponent::GetRoundsLeft() const
{
	return RoundsLeft;
}



EFiringState UTankAimingComponent::GetFiringState() const
{
	return FiringState;
}


bool UTankAimingComponent::isBarrelMoving()
{
	if (!ensure(Barrel)) { return false; }
	auto BarrelForward = Barrel->GetForwardVector();
	
	return !BarrelForward.Equals(AimDirection, 0.1);
}



void UTankAimingComponent::AimAt(FVector HitLocation)
{
	if (!ensure(Barrel)) { return; }

	// calculate the desired launch velocity
	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	//FCollisionResponseParams CollisionParams;		// not used
	//TArray<AActor*> ActorsToIgnore;				// not used
	//ActorsToIgnore.Add(GetOwner());				// not used
	
	bool BHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity
		(
			this,
			OutLaunchVelocity,
			StartLocation,
			HitLocation,
			LaunchSpeed,
			bUseHighArc,
			0,
			0,
			ESuggestProjVelocityTraceOption::DoNotTrace //,  // TODO fix this
			//CollisionParams,
			//ActorsToIgnore,
			//true
			);
	if(BHaveAimSolution)
	{
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowards(AimDirection);

		// auto Time = GetWorld()->GetTimeSeconds();
		// UE_LOG(LogTemp, Warning, TEXT("TossVelocity: %s   AimSolution true, Time: %f"), *OutLaunchVelocity.ToString(), Time)
	}
	else
	{
		// auto Time = GetWorld()->GetTimeSeconds();
		// UE_LOG(LogTemp, Warning, TEXT("TossVelocity: %s   AimSolution false, Time: %f"), *OutLaunchVelocity.ToString(), Time)
	}
}


void UTankAimingComponent::MoveBarrelTowards(FVector AimDirection)
{
	if (!ensure(Barrel && Turret)) { return; }
	// work out difference between current barrel rotation and AimDirection
	FRotator BarrelRotator = Barrel->GetForwardVector().Rotation();
	FRotator AimAsRotator = AimDirection.Rotation();
	FRotator DeltaRotator = AimAsRotator - BarrelRotator;

	// Negatete Rotator when Yaw over 180 or below -180, to fix wrong Rotation Direction
	FRotator DeltaTest = (DeltaRotator.Yaw > 180 || DeltaRotator.Yaw < -180) ? (DeltaRotator * -1) : (DeltaRotator); // TernaryOperator: (Condition) ? (true) : (false)
	FRotator DeltaTest2 = (FMath::Abs(DeltaRotator.Yaw) > 180) ? (DeltaRotator * -1) : (DeltaRotator); // TernaryOperator: (Condition) ? (true) : (false)

		Barrel->Elevate(DeltaRotator.Pitch);
		Turret->Rotate(DeltaTest.Yaw);
}


void UTankAimingComponent::Fire()
{
	if (FiringState == EFiringState::Locked || FiringState == EFiringState::Aiming)
	{
		//spawn projectile
		if (!ensure(Barrel)) { return; }
		if (!ensure(ProjectileBlueprint)) { return; }

		auto Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileBlueprint,
			Barrel->GetSocketLocation(FName("Projectile")),
			Barrel->GetSocketRotation(FName("Projectile"))
			);

		Projectile->LaunchProjectile(LaunchSpeed);
		LastFireTime = FPlatformTime::Seconds();

		RoundsLeft--;
	}
}