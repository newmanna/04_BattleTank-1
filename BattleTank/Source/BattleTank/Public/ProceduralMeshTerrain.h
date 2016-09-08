// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshTerrain.generated.h"

class UProceduralMeshComponent; // forward declaration

UCLASS()
class BATTLETANK_API AProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralMeshTerrain();

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
		int32 XCoords = 20;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
		int32 YCoords = 20;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
		float QuadSize = 100;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
		float LineTraceLength = 10000;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
		float LineTraceHeightOffset = 100;


	/////////////////////////////////////////////////////////////////
	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	TArray<FVector> Vertices;

	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	TArray<int32> Triangles;

	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	TArray<FVector2D> UV;

	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	TArray<FVector> Normals;

	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	TArray<FLinearColor> VertexColors;

	//UPROPERTY(VisibleInstanceOnly, Category = "ProceduralMeshDebug")
	//TArray<FProcMeshTangent> Tangents;
	/////////////////////////////////////////////////////////////////

protected:
	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshGeneration")
		void GenerateMesh(bool CalculateTangentsForMesh);

	void CopyLandscapeHeightBelow(FVector &Coordinates);

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void UpdateTerrainBoundsExtent();


	UPROPERTY(VisibleAnywhere, Category = "Components")
		UProceduralMeshComponent* ProceduralMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USceneComponent* SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UBoxComponent* TerrainBounds = nullptr;
	
	
};
