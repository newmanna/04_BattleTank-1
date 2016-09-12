// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralMeshTerrain.generated.h"

// forward declaration
class URuntimeMeshComponent;





USTRUCT(BlueprintType)
struct FSavedSectionProperties
{
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Vertices;
};




UCLASS()
class BATTLETANK_API AProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralMeshTerrain();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);



	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	int32 ComponentXY = 1;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	int32 SectionXY = 20;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float QuadSize = 100;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float LineTraceLength = 10000;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float LineTraceHeightOffset = 100;

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	bool bCalculateTangents = false;


protected:
	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshGeneration")
	void GenerateMesh(bool CalculateTangentsForMesh);


private:
	virtual void BeginPlay() override;
	//virtual void Tick(float DeltaSeconds) override;

	void CopyLandscapeHeightBelow(FVector &Coordinates);
	void FillVerticesArray(float OffsetX, float OffsetY);
	void GetCoordinates(FVector Location, FVector2D& LocalCoordinates, int32 SectionIndex);


	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UV;
	TArray<FVector> Normals;
	TArray<FColor> VertexColors; // FLinearColor

	TArray<FSavedSectionProperties> SavedSectionVerts;
};
