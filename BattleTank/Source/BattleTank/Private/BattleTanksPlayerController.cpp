// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "BattleTanksPlayerController.h"


void ABattleTanksPlayerController::BeginPlay()
{
	Super::BeginPlay();

	auto ControlledTank = GetControlledTank();
	if (ControlledTank)
	{
		UE_LOG(LogTemp, Warning, TEXT("Succesfully possesed: %s"), *ControlledTank->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could Not find Tank"));
	}
}





ATank* ABattleTanksPlayerController::GetControlledTank() const
{
	return Cast<ATank>(GetPawn());
}