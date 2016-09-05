// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "TankAimingComponent.h"  ...as of lecture 133, ben wants us to use forward declarations instead of #includes in .h. it has ben move to the .cpp
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"


UCLASS()
class BATTLETANK_API ATank : public APawn
{
	GENERATED_BODY()


	// Sets default values for this pawn's properties
	ATank();
};
