// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"	//
#include "Kismet/KismetSystemLibrary.h" // testing the other line trace 
#include "ProceduralMeshTerrain.h"


AProceduralMeshTerrain::AProceduralMeshTerrain()
{
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;

	TerrainBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("TerrainBounds"));
	TerrainBounds->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	// GenerateMesh();
}


void AProceduralMeshTerrain::GenerateMesh(bool CalculateTangentsForMesh)
{
	// TODO check best solution to prevent constant memory re-allocation when filling array. is this even a problem?
	int32 ArraySize = XCoords * YCoords;
	Vertices.SetNum(ArraySize, true);
	UV.SetNum(ArraySize, true);

	// iterate through Vertices & UV to set desired XY(Z) Coordinates
	int32 IterationIndex;
	FVector Coordinates;
	FVector2D UVCoordinates;
	for (int32 i = 0; i < XCoords; i++)
	{
		for (int32 j = 0; j < YCoords; j++)
		{
			IterationIndex = YCoords * i + j;
			Coordinates = FVector((float)i * QuadSize, (float)j * QuadSize, 0); // TODO trace terrain below to copy Z height
			UVCoordinates = FVector2D(i, j);
			UV[IterationIndex] = UVCoordinates;

			CopyLandscapeHeightBelow(OUT Coordinates);
			Vertices[IterationIndex] = Coordinates;
		}
	}
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(XCoords, YCoords, false, OUT Triangles);
	TArray<FProcMeshTangent> Tangents;
	if (CalculateTangentsForMesh)
	{
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV, OUT Normals, OUT Tangents); // very costly
	}
	ProceduralMesh->CreateMeshSection_LinearColor(1, Vertices, Triangles, Normals, UV, VertexColors, Tangents, true);
	UpdateTerrainBoundsExtent();
}


void AProceduralMeshTerrain::CopyLandscapeHeightBelow(FVector &Coordinates)
{
	FHitResult Hit;
	TArray<AActor*> toignore;
	UKismetSystemLibrary::LineTraceSingle_NEW(
		this,
		Coordinates + GetActorLocation(),
		Coordinates + GetActorLocation() - FVector(0, 0, LineTraceLength),
		UEngineTypes::ConvertToTraceType(ECC_Camera),
		false,
		toignore,
		EDrawDebugTrace::ForOneFrame,
		OUT Hit,
		true);
	float LineTraceHeight = Hit.Location.Z - GetActorLocation().Z + LineTraceHeightOffset;
	Coordinates = FVector(Coordinates.X, Coordinates.Y, LineTraceHeight);
}


void AProceduralMeshTerrain::UpdateTerrainBoundsExtent()
{
	auto ExtentX = XCoords * QuadSize / 2;
	auto ExtentY = YCoords * QuadSize / 2;
	TerrainBounds->SetBoxExtent(FVector(ExtentX, ExtentY, LineTraceLength), false);
	TerrainBounds->SetRelativeLocation(FVector(ExtentX, ExtentY, -LineTraceLength));
}



void AProceduralMeshTerrain::BeginPlay()
{
	Super::BeginPlay();
	//GenerateMesh();
}


void AProceduralMeshTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

