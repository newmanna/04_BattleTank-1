// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "RuntimeMeshSection.h"


// Sets default values
ARuntimeMeshSection::ARuntimeMeshSection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARuntimeMeshSection::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARuntimeMeshSection::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

