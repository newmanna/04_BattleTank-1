// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "TankAimingComponent.h"  ...as of lecture 133, ben wants us to use forward declarations instead of #includes in .h. it has ben move to the .cpp
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

class UTankBarrel; // forward declaration
class UTankTurret; // forward declaration
class AProjectile; // forward declaration
class UTankAimingComponent; // forward declaration
class UTankMovementComponent; // forward declaration

UCLASS()
class BATTLETANK_API ATank : public APawn
{
	GENERATED_BODY()


public:
	void AimAt(FVector HitLocation);

	UFUNCTION(BlueprintCallable, Category = Setup)
	void Fire();


protected:
	UPROPERTY(BlueprintReadOnly)
	UTankAimingComponent* TankAimingComponent = nullptr;

	UPROPERTY (BlueprintReadOnly)
	UTankMovementComponent* MovementComponent = nullptr;


private:
	// Sets default values for this pawn's properties
	ATank();

	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = Setup)
	TSubclassOf<AProjectile> ProjectileBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = Firing)
	float LaunchSpeed = 8000;

	UPROPERTY(EditDefaultsOnly, Category = Firing)
	float ReloadTimeInSeconds = 2;
	
	// local barrel reference for spawning projectile
	UTankBarrel* Barrel = nullptr; // TODO Remove 


	double LastFireTime = 0;
};
