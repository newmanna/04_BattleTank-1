// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "BattleTanksAIController.h"


void ABattleTanksAIController::BeginPlay()
{
	Super::BeginPlay();

	auto ControlledTank = GetControlledTank();
	if (ControlledTank)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController possessing: %s"), *ControlledTank->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIController NOT possessing a Tank"));
	}
}





ATank * ABattleTanksAIController::GetControlledTank() const
{
	return Cast<ATank>(GetPawn());
}


