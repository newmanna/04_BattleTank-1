// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankBarrel.h"
#include "TankAimingComponent.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UTankAimingComponent::SetBarrelReference(UTankBarrel* BarrelToSet)
{
	Barrel = BarrelToSet;
	UE_LOG(LogTemp, Warning, TEXT("Barrel set to: %s"), *Barrel->GetName())
}


void UTankAimingComponent::AimAt(FVector HitLocation, float LaunchSpeed)
{
	if (!Barrel) { return; }



	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	FCollisionResponseParams CollisionParams;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	
	// calculate the desired launch velocity
	bool BHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity
		(
			this,
			OutLaunchVelocity,		// OUT
			StartLocation,			// Barrel loc
			HitLocation,			// hit loc
			LaunchSpeed,			// launchSpeed
			ESuggestProjVelocityTraceOption::DoNotTrace // weather or not to trace desired arc if clear path Enum
			);
	if(BHaveAimSolution)
	{
		auto AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowards(AimDirection);
	}
}




void UTankAimingComponent::MoveBarrelTowards(FVector AimDirection)
{
	// make rotator from AimRotation
	FRotator BarrelRotator = Barrel->GetForwardVector().Rotation();
	FRotator AimAsRotator = AimDirection.Rotation();
	FRotator DeltaRotator = AimAsRotator - BarrelRotator;
	UE_LOG(LogTemp, Warning, TEXT("AimAsRotator: %s"), *DeltaRotator.ToString())

		Barrel->Elevate(5); // TODO remove magic number






	//FRotator BarrelOnlyPitchRot = FRotator(AimAsRotator.Pitch, 0, 0);

	//Barrel->SetWorldRotation(DeltaRotator);

	//FRotator BarrelOnlyPitchRot = FRotator(BarrelRotator.Pitch, 0, 0);
	//Barrel->SetWorldRotation(BarrelOnlyPitchRot);

	// extraxt pitch of AimDirection

	// set barrel pitch 


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

