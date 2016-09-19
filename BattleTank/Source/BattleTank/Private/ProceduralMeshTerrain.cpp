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
	GenerateMesh(false);
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
		IsVertexOnEdge[j] = IsSectionBorder(SectionRootOffset);
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


inline bool AProceduralMeshTerrain::IsSectionBorder(FVector2D Coordinates)
{
	if (Coordinates.X == 0 || Coordinates.X == SectionXY - 1) { return true; }
	if (Coordinates.Y == 0 || Coordinates.Y == SectionXY - 1) { return true; }
	return false;
}


inline bool AProceduralMeshTerrain::IsSectionEdge(FVector2D Coordinates)
{
	FVector2D Ratio = Coordinates / FVector2D(SectionXY, SectionXY);
	//Left Bottom
	if (Ratio == FVector2D(0, 0)) { return true; }
	// Left Top
	if (Ratio == FVector2D(1, 0)) { return true; }
	// Right Bottom
	if (Ratio == FVector2D(0, 1)) { return true; }
	// Right Top
	if (Ratio == FVector2D(1, 1)) { return true; }
	return false;
}


inline FVertDistanceFromBorder AProceduralMeshTerrain::DistanceToBorder(FVector2D Coordinates)
{
	FVertDistanceFromBorder Distances;
	Distances.Top = SectionXY - 1 - Coordinates.X;
	Distances.Bottom = Coordinates.X;
	Distances.Left = Coordinates.Y;
	Distances.Right = SectionXY - 1 - Coordinates.Y;
	return Distances;
}


void AProceduralMeshTerrain::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	FVector2D CenterCoordinates;
	int32 SectionIndex;
	int32 CenterVertex;
	bool bHitInfoValid = GetSectionHitInfo(Hit.Location, OUT CenterCoordinates, OUT SectionIndex, OUT CenterVertex);
	
	// update vert location and add to queue
	if (bHitInfoValid)
	{
		const float Falloff = 0.1; // TODO replace with float curve
		const float MaxCrater = 100;
		const int32 HitRadius = 1;
		int32 HitDiameter = HitRadius + HitRadius + 1;

		/*
		auto CenterToBorder = DistanceToBorder(CenterCoordinates);
		UE_LOG(LogTemp, Warning, TEXT("Top: %i Bottom: %i Left: %i Right: %i"), CenterToBorder.Top, CenterToBorder.Bottom, CenterToBorder.Left, CenterToBorder.Right);

		for (int X = -HitRadius; X <= HitRadius; X++)
		{
			for (int Y = -HitRadius; Y <= HitRadius; Y++)
			{
				// is crossing section at bottom
				if (X < 0 && (-CenterToBorder.Bottom - X) >= 0)
				{
					int32 CurrentVert = CenterVertex + (X * SectionXY + Y);
					UE_LOG(LogTemp, Warning, TEXT("UnderBottomTest"))
				}

			}
		}
		*/

		for (int XComp = 0; XComp < ComponentXY; XComp++)
		{
			for (int YComp = 0; YComp < ComponentXY; YComp++)
			{
				int32 GlobalXYVerts = (SectionXY-1) * ComponentXY +1; // if 5x5 SectionXY and 3x3 ComponentXY, result == 13. 
				int32 ToAddWhenIteratingComponentX = GlobalXYVerts * (SectionXY - 1); // if 5x5 SectionXY and 3x3 ComponentXY, result == 52. (actually 53 verts, but we started with index 0)
				int32 ToAddWhenIteratingComponentY = (SectionXY - 1);
				int32 SectionRoot = (ToAddWhenIteratingComponentX * XComp) + (ToAddWhenIteratingComponentY * YComp); //if 5x5 SectionXY and 3x3 ComponentXY, result == 0, 4, 8, 12, 52, 56, 60, 104, 108, 112
				for (int XSection = 0; XSection < SectionXY; XSection++)
				{
					for (int YSection = 0; YSection < SectionXY; YSection++)
					{
						int32 ToAddWhenIteratingSectionX = GlobalXYVerts;
						int32 ToAddWhenIteratingSectionY = YSection;
						int32 IndexToAdd = ToAddWhenIteratingSectionX * XSection + ToAddWhenIteratingSectionY;
						int32 IndexTotal = SectionRoot + IndexToAdd;
						UE_LOG(LogTemp, Warning, TEXT("IndexTotal: %i"), IndexTotal);
					}
				}
			}
		}

		/*
		int32 AffectedVert = CenterVertex + (X * SectionXY + Y); // this alone is correct as long as staying on same section

		for (int X = -HitRadius; X <= HitRadius; X++)
		{
			for (int Y = -HitRadius; Y <= HitRadius; Y++)
			{
				// get Vertex Z height that will be subtracted from current
				int32 DistanceFromImpact = X + Y;
				float CraterForce = MaxCrater * (1 - (DistanceFromImpact * Falloff));

				int32 CenterXToBorder = (X > 0) ? (DistanceToBorder(LocalCoordinates).Top) : (DistanceToBorder(LocalCoordinates).Bottom);
				int32 CenterYToBorder = (Y > 0) ? (DistanceToBorder(LocalCoordinates).Right) : (DistanceToBorder(LocalCoordinates).Left);
				UE_LOG(LogTemp, Warning, TEXT("Distance X: %i Distance Y: %i"), CenterXToBorder, CenterYToBorder)






				//int32 VertIndex = CenterVertex;
				//SavedSection[SectionIndex].Vertices[CenterVertex] += FVector(0, 0, -CraterForce);
			}
		}*/
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


bool AProceduralMeshTerrain::GetSectionHitInfo(FVector HitLocation, FVector2D& SectionCoordinates, int32& SectionIndex, int32& HitVertex) // TODO return struct instead of out parameters
{
	// Get Coordinates relative To Actor RootComponent
	FVector RelativeHitLocation = HitLocation - GetActorLocation();
	int32 XCoordinate = FMath::RoundToInt(RelativeHitLocation.X / QuadSize);
	int32 YCoordinate = FMath::RoundToInt(RelativeHitLocation.Y / QuadSize);
	//FVector2D CoordinatesRelativeToRoot = FVector2D(XCoordinate, YCoordinate);

	// Get Index of section that was hit 
	int32 SectionSize = (SectionXY - 1) * QuadSize;
	int32 XCompHit = RelativeHitLocation.X / SectionSize;
	int32 YCompHit = RelativeHitLocation.Y / SectionSize;
	FVector2D ComponentCoordinates = FVector2D(XCompHit, YCompHit);
	SectionIndex = ComponentXY * XCompHit + YCompHit; // TODO add the middle of section to tarray and get closest one by comparing distances. will be needed once we start with procedural generation, as the sections most likely wont be in order 

	// Get Coordinates relative to Section Root
	int32 XCoordinateSection = XCoordinate % SectionXY + XCompHit; // +XCompHit because: edge vertex of every section is in the same location, with every section you have to subtract XY to match coordinates
	int32 YCoordinateSection = YCoordinate % SectionXY + YCompHit;
	SectionCoordinates = FVector2D(XCoordinateSection, YCoordinateSection);

	// Get the index of the vertex coordinates closest to hit location
	HitVertex = SectionCoordinates.X * SectionXY + SectionCoordinates.Y;

	// check if hit section and hit vertex exist inside save struct
	if (!SavedSection.IsValidIndex(SectionIndex)) { return false; }
	if (!SavedSection[SectionIndex].Vertices.IsValidIndex(HitVertex)) { return false; }
	return true;
}


/*
UE_LOG(LogTemp, Warning, TEXT("HitCoords: %s"), *FVector2D(X, Y).ToString())




*/