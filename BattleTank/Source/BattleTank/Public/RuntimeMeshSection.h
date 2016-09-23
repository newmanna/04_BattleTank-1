// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RuntimeMeshSection.generated.h"

//class ARuntimeMeshComponent;
class AProceduralMeshTerrain;

UCLASS()
class BATTLETANK_API ARuntimeMeshSection : public AActor
{
	GENERATED_BODY()
	
public:	
	ARuntimeMeshSection();
	
	void InitializeOnSpawn(int32 SectionIndex, AProceduralMeshTerrain* Terrain);
	void CreateSection();
	void UpdateSection();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);


private:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	//UPROPERTY(VisibleAnywhere, Category = "Components")
	//USphereComponent* SceneRoot = nullptr;


	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	int32 SectionIndexLocal = 0;
	//int32 SectionXYLocal = 0;
	//int32 QuadSizeLocal = 0;
	TArray<int32> IndexBufferLocal;
	FVector2D SectionCoordinates;

	AProceduralMeshTerrain* OwningTerrain = nullptr;
};
