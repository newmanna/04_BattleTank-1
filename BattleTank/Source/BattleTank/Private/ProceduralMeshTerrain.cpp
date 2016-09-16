// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
//#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h" // still needed for CalculateTangentsForMesh function
#include "RuntimeMeshComponent.h" 
#include "RuntimeMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h" // contains the line trace that is used in blueprint 
#include "ProceduralMeshTerrain.h"


AProceduralMeshTerrain::AProceduralMeshTerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent")); // Testing
	RootComponent = RuntimeMeshComponent;
}


void AProceduralMeshTerrain::BeginPlay()
{
	Super::BeginPlay();
	RuntimeMeshComponent->OnComponentHit.AddDynamic(this, &AProceduralMeshTerrain::OnHit);
	//GenerateMesh(false);
}


void AProceduralMeshTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SectionUpdateQueue.Num() > 0 && bAllowedToUpdateSection)
	{
		bAllowedToUpdateSection = false;
		UpdateMeshSection(SectionUpdateQueue[0]);
		SectionUpdateQueue.RemoveAt(0); // TODO look if this causes any memory leak with many projectiles
	}
}


void AProceduralMeshTerrain::GenerateMesh(bool CalculateTangentsForMesh)
{
	TArray<FProcMeshTangent> TangentsProcMesh;
	int32 ArraySize = (SectionXY) * (SectionXY);
	IsVertexOnEdge.SetNum(ArraySize, true);
	Vertices.SetNum(ArraySize, true);
	UV.SetNum(ArraySize, true);
	SavedSection.SetNum(ComponentXY * ComponentXY, true);

	URuntimeMeshLibrary::CreateGridMeshTriangles(SectionXY, SectionXY, false, OUT Triangles);

	// create a mesh section each iteration
	for (int32 i = 0; i < ComponentXY*ComponentXY; i++)
	{
		auto ComponentOffsetX = i / ComponentXY;
		auto ComponentOffsetY = i % ComponentXY;
		FillVerticesArray(ComponentOffsetX, ComponentOffsetY, i);
		
		if (CalculateTangentsForMesh) 
		{
			UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV, OUT Normals, OUT TangentsProcMesh);
		}

		RuntimeMeshComponent->CreateMeshSection(
			i, 
			Vertices,
			Triangles,
			Normals, 
			UV, 
			VertexColors, 
			Tangents, 
			true);

		FillSavedSectionStruct(i);
	}
}


void AProceduralMeshTerrain::FillSavedSectionStruct(int32 SectionIndex)
{
	SavedSection[SectionIndex].Vertices = Vertices;
	SavedSection[SectionIndex].Triangles = Triangles;
	SavedSection[SectionIndex].UV = UV;
	SavedSection[SectionIndex].Normals = Normals;
	SavedSection[SectionIndex].VertexColors = VertexColors;
	SavedSection[SectionIndex].Tangents = Tangents;
}


void AProceduralMeshTerrain::FillVerticesArray(float OffsetX, float OffsetY, int32 SectionIndex)
{
	FVector2D SectionRootOffsetFromMeshRoot = FVector2D(OffsetX, OffsetY) * (SectionXY-1);
	FVector2D SectionRootOffset;
	FVector2D MeshRootOffset;
	FVector VertexLocation;
	for (int32 j = 0; j < Vertices.Num(); j++)
	{
		SectionRootOffset = FVector2D(j / SectionXY, j % SectionXY);
		MeshRootOffset = SectionRootOffset + SectionRootOffsetFromMeshRoot;

		VertexLocation = FVector(MeshRootOffset.X, MeshRootOffset.Y, 0) * QuadSize;
		CopyLandscapeHeightBelow(OUT VertexLocation); // TODO figuere out how to use heightmap instead. or use noise, and as a bonus figure out how to export heightmap for save/load	
		
		Vertices[j] = VertexLocation;
		UV[j] = FVector2D(MeshRootOffset.X, MeshRootOffset.Y);

		IsVertexOnEdge[j] = bIsVertOnEdge(SectionRootOffset);
		UE_LOG(LogTemp, Warning, TEXT("VertexCoordinates: %s  OnEdge?: %i"), *SectionRootOffset.ToString(), IsVertexOnEdge[j]);
	}
}


void AProceduralMeshTerrain::CopyLandscapeHeightBelow(FVector &Coordinates)
{
	FHitResult Hit;
	TArray<AActor*> ToIgnore;
	UKismetSystemLibrary::LineTraceSingle_NEW(
		this,
		Coordinates + GetActorLocation(),
		Coordinates + GetActorLocation() - FVector(0, 0, LineTraceLength),
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		ToIgnore,
		EDrawDebugTrace::None,
		OUT Hit,
		true);
	float LineTraceHeight = Hit.Location.Z - GetActorLocation().Z + LineTraceHeightOffset;
	Coordinates = FVector(Coordinates.X, Coordinates.Y, LineTraceHeight);
}


bool AProceduralMeshTerrain::bIsVertOnEdge(FVector2D VertexCoordinates)
{
	if (VertexCoordinates.X == 0 || VertexCoordinates.X == SectionXY - 1) { return true; }
	if (VertexCoordinates.Y == 0 || VertexCoordinates.Y == SectionXY - 1) { return true; }
	return false;
}


void AProceduralMeshTerrain::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	FVector2D LocalCoordinates;
	int32 SectionIndex;
	int32 HitVertex;
	bool bHitInfoValid = GetValidSectionInfo(Hit.Location, OUT LocalCoordinates, OUT SectionIndex, OUT HitVertex);
	
	if (bHitInfoValid)
	{
		auto Test = SavedSection[SectionIndex].Vertices[HitVertex];
		SavedSection[SectionIndex].Vertices[HitVertex] = Test + FVector(0, 0, -100);

		// add sections that need to be updated to a queue
		SectionUpdateQueue.Add(SectionIndex);		
	}
}


void AProceduralMeshTerrain::UpdateMeshSection(int32 SectionIndex)
{
	TArray<FProcMeshTangent> TangentsProcMesh;
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		SavedSection[SectionIndex].Vertices, 
		Triangles, 
		SavedSection[SectionIndex].UV, 
		OUT SavedSection[SectionIndex].Normals,
		OUT TangentsProcMesh);


	RuntimeMeshComponent->UpdateMeshSection(
		SectionIndex,
		SavedSection[SectionIndex].Vertices,
		SavedSection[SectionIndex].Normals,
		SavedSection[SectionIndex].UV,
		SavedSection[SectionIndex].VertexColors,
		SavedSection[SectionIndex].Tangents);
	bAllowedToUpdateSection = true;

}


bool AProceduralMeshTerrain::GetValidSectionInfo(FVector HitLocation, FVector2D& SectionCoordinates, int32& SectionIndex, int32& HitVertex)
{
	// Get Coordinates relative To RootComponent
	FVector RelativeHitLocation = HitLocation - GetActorLocation();
	int32 XCoordinate = FMath::RoundToInt(RelativeHitLocation.X / QuadSize);
	int32 YCoordinate = FMath::RoundToInt(RelativeHitLocation.Y / QuadSize);
	//FVector2D CoordinatesRelativeToRoot = FVector2D(XCoordinate, YCoordinate);

	// Get hit Section
	int32 SectionSize = (SectionXY - 1) * QuadSize;
	int32 XCompHit = RelativeHitLocation.X / SectionSize;
	int32 YCompHit = RelativeHitLocation.Y / SectionSize;
	FVector2D ComponentCoordinates = FVector2D(XCompHit, YCompHit);
	SectionIndex = ComponentXY * XCompHit + YCompHit;

	// Get Coordinates relative to Section Root
	int32 XCoordinateSection = XCoordinate % SectionXY + XCompHit; // +XCompHit because: edge vertex of every section is in the same location, with every section you have to subtract XY to match coordinates
	int32 YCoordinateSection = YCoordinate % SectionXY + YCompHit;
	SectionCoordinates = FVector2D(XCoordinateSection, YCoordinateSection);

	// Get vertex closest to hit location
	HitVertex = SectionCoordinates.X * SectionXY + SectionCoordinates.Y;

	// check if hit section and hit vertex exist inside save struct
	if (!SavedSection.IsValidIndex(SectionIndex)) { return false; }
	if (!SavedSection[SectionIndex].Vertices.IsValidIndex(HitVertex)) { return false; }
	return true;
}

















/*void AProceduralMeshTerrain::Tick(float DeltaTime)
{
Super::Tick(DeltaTime);
}*/

/*

void AProceduralMeshTerrain::CopyLandscapeHeightBelow(FVector &Coordinates)
{
FHitResult Hit;
TArray<AActor*> ToIgnore;
UKismetSystemLibrary::LineTraceSingle_NEW(
this,
Coordinates + GetActorLocation(),
Coordinates + GetActorLocation() - FVector(0, 0, LineTraceLength),
UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
false,
ToIgnore,
EDrawDebugTrace::None,
OUT Hit,
true);
float LineTraceHeight = Hit.Location.Z - GetActorLocation().Z + LineTraceHeightOffset;
Coordinates = FVector(Coordinates.X, Coordinates.Y, LineTraceHeight);
}



*/