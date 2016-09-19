// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"  // forward declaration not enought to access "FRuntimeMeshTangent"
#include "ProceduralMeshTerrain.generated.h"


USTRUCT(BlueprintType)
struct FSectionProperties
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<int32> Triangles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector2D> UV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Normals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FColor> VertexColors; // FLinearColor

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FRuntimeMeshTangent> Tangents;

	FSectionProperties()
	{
	}
};

USTRUCT(BlueprintType)
struct FVertDistanceFromBorder
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int32 Top;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int32 Bottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int32 Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	int32 Right;

	FVertDistanceFromBorder()
	{
	}
};


UCLASS()
class BATTLETANK_API AProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralMeshTerrain();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);

	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshGeneration")
	void GenerateMesh(bool CalculateTangentsForMesh);

	// TODO give parameters better names
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


private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void CopyLandscapeHeightBelow(FVector &Coordinates);
	void FillVerticesArray(float OffsetX, float OffsetY, int32 SectionIndex);
	void FillSavedSectionStruct(int32 SectionIndex);
	inline bool GetSectionHitInfo(FVector HitLocation, FVector2D& SectionCoordinates, int32& SectionIndex, int32& HitVertex);
	inline bool IsSectionBorder(FVector2D VertexCoordinates);
	bool IsSectionEdge(FVector2D VertexCoordinates);
	inline FVertDistanceFromBorder DistanceToBorder(FVector2D VertexCoordinates);
	void UpdateMeshSection(int32 SectionIndex);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UV;
	TArray<FVector> Normals;
	TArray<FColor> VertexColors;
	TArray<FRuntimeMeshTangent> Tangents;

	TArray<FSectionProperties> SavedSection;

	TArray<bool> IsVertexOnEdge;	// TODO might be neccessary to store distance to edge as coordinates

	TArray<int32> SectionUpdateQueue; // TODO replace with TQueue

	bool bAllowedToUpdateSection = true;



};
