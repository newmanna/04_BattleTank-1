// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankAimingComponent.h"
#include "BattleTanksAIController.h"


void ABattleTanksAIController::BeginPlay()
{
	Super::BeginPlay();
}


void ABattleTanksAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (!GetPawn()) { return; } // suggestion from a user


	auto PlayerTank = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto ControlledTank = GetPawn();
	
	if (!ensure(PlayerTank && ControlledTank)) { return; }
	
	// Move towards the player
	MoveToActor(PlayerTank, AcceptanceRadius);

	//Aim towards the player
	auto AimingComponent = ControlledTank->FindComponentByClass<UTankAimingComponent>();
	AimingComponent->AimAt(PlayerTank->GetActorLocation());
			
	// Fire towards player, if aiming or locked
	if (AimingComponent->GetFiringState() == EFiringState::Locked)
	{
		AimingComponent->Fire();
	}
}
