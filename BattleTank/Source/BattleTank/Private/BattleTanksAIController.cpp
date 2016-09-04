// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"

#include "TankMovementComponent.h"

#include "BattleTanksAIController.h"


void ABattleTanksAIController::BeginPlay()
{
	Super::BeginPlay();

	//GetWorld()->GetTimerManager().SetTimer(FireTimer, this )
}


void ABattleTanksAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

		auto PlayerTank = Cast<ATank>(GetWorld()->GetFirstPlayerController()->GetPawn());
		auto ControlledTank = Cast<ATank>(GetPawn());

		if (PlayerTank)
		{
			MoveToActor(PlayerTank, AcceptanceRadius);
			ControlledTank->AimAt(PlayerTank->GetActorLocation());
			ControlledTank->Fire();
			
		}
}