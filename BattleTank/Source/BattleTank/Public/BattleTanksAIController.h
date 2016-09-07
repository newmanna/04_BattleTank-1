// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "BattleTanksAIController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLETANK_API ABattleTanksAIController : public AAIController
{
	GENERATED_BODY()


protected:
	// how close AI tank can come to player
	UPROPERTY(EditAnywhere, Category = "Setup")
	float AcceptanceRadius = 5000;

	UFUNCTION() // has to be ufunction so it can be subscribed to OnDeath delegate
	void OnPossessedTankDeath();

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetPawn(APawn* InPawn) override;




};
