// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "BattleTanksAIController.h"


void ABattleTanksAIController::BeginPlay()
{
	Super::BeginPlay();

	auto PlayerTank = GetPlayerTank();
	if (PlayerTank)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController found PlayerTank: %s"), *PlayerTank->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIController could't find PlayerTank"));
	}
}


void ABattleTanksAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("TICKING"));
	
}





ATank* ABattleTanksAIController::GetControlledTank() const
{
	return Cast<ATank>(GetPawn());
}

ATank* ABattleTanksAIController::GetPlayerTank() const
{
	auto PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) { return nullptr; }
	return Cast<ATank>(PlayerPawn);
}