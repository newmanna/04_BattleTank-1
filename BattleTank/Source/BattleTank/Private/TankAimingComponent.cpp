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
	
	if ((FPlatformTime::Seconds() - LastFireTime) < ReloadTimeInSeconds)
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
			false,
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

		Barrel->Elevate(DeltaRotator.Pitch);
		Turret->Rotate(DeltaTest.Yaw);

		// TODO: something wrong
}


void UTankAimingComponent::Fire()
{
	if (FiringState != EFiringState::Reloading)
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
	}
}







/*
static bool SuggestProjectileVelocity
(
	GetWorld(),
	FVector & TossVelocity,		// OUT
	FVector StartLocation,		// Barrel loc
	FVector EndLocation,		// hit loc
	float TossSpeed,			// launchSpeed
	bool bHighArc,				// false
	float CollisionRadius,		// 10
	float OverrideGravityZ,		// false
	ESuggestProjVelocityTraceOption::Type TraceOption, // weather or not to trace desired arc if clear path Enum
	const FCollisionResponseParams & ResponseParam,
	const TArray < AActor * > & ActorsToIgnore,
	bool bDrawDebug				// debug arc 
	)



	// Called when the game starts
	void UTankAimingComponent::BeginPlay()
	{
	Super::BeginPlay();

	}


	// Called every frame
	void UTankAimingComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
	{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	}

	*/

