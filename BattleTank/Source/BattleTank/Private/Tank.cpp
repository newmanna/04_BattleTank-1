// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankAimingComponent.h" // notice how this has to be under #include "BattleTank.h"
#include "Tank.h"


// Sets default values
ATank::ATank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// No need to protect pointers as added to the constructor
	TankAimingComponent = CreateDefaultSubobject<UTankAimingComponent>(FName("AimingComponent"));
}

void ATank::SetBarrelReference(UTankBarrel* BarrelToSet)
{
	TankAimingComponent->SetBarrelReference(BarrelToSet);
}

void ATank::SetTurretReference(UTankTurret* TurretToSet)
{
	TankAimingComponent->SetTurretReference(TurretToSet);
}


// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void ATank::AimAt(FVector HitLocation)
{
	TankAimingComponent->AimAt(HitLocation, LaunchSpeed);
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