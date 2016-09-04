// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tank.h"
#include "AIController.h"
#include "BattleTanksAIController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLETANK_API ABattleTanksAIController : public AAIController
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// how close AI tank can come to player
	float AcceptanceRadius = 400;

};
