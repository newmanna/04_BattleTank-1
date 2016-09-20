// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"  // forward declaration not enought to access "FRuntimeMeshTangent"
#include "ProceduralMeshTerrain.generated.h"


USTRUCT(BlueprintType)
struct FVertexCombined
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
	TArray<FColor> VertexColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FRuntimeMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<bool> IsOnBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<bool> IsOnEdge;

	FVertexCombined()
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
	void CopyLandscapeHeightBelow(FVector& Coordinates, FVector& Normal);
	void FillSectionVertStruct(float OffsetX, float OffsetY, int32 SectionIndex);
	void UpdateMeshSection(int32 SectionIndex);
	void FillIndexBuffer();
	void InitializeProperties();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	FVertexCombined GlobalProperties;
	FVertexCombined SectionProperties;
	TArray<int32> IndexBuffer;
	TArray<int32> SectionUpdateQueue; // TODO replace with TQueue
	TArray<int32> SectionCreateQueue; // TODO replace with TQueue

	bool bAllowedToUpdateSection = true;
};
