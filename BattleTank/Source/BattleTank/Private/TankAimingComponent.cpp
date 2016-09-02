// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankBarrel.h"
#include "TankTurret.h"

#include "TankAimingComponent.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false; // TODO should this tick?

	// ...
}

void UTankAimingComponent::SetBarrelReference(UTankBarrel* BarrelToSet)
{
	if (!BarrelToSet) { return; }
	Barrel = BarrelToSet;
}

void UTankAimingComponent::SetTurretReference(UTankTurret* TurretToSet)
{
	if (!TurretToSet) { return; }
	Turret = TurretToSet;
}


void UTankAimingComponent::AimAt(FVector HitLocation, float LaunchSpeed)
{
	if (!Barrel) { return; }

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
		auto AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowards(AimDirection);

		auto Time = GetWorld()->GetTimeSeconds();
		//UE_LOG(LogTemp, Warning, TEXT("TossVelocity: %s   AimSolution true, Time: %f"), *OutLaunchVelocity.ToString(), Time)
	}
	else
	{
		auto Time = GetWorld()->GetTimeSeconds();
		//UE_LOG(LogTemp, Warning, TEXT("TossVelocity: %s   AimSolution false, Time: %f"), *OutLaunchVelocity.ToString(), Time)
	}
}


void UTankAimingComponent::MoveBarrelTowards(FVector AimDirection)
{
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

