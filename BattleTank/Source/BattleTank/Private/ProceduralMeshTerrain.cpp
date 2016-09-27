// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
//#include "ProceduralMeshComponent.h"
//#include "KismetProceduralMeshLibrary.h"
#include "RuntimeMeshComponent.h" 
#include "RuntimeMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h" // for BP Linetrace
#include "Tank.h"						// TODO make custom collision channel for landscape line trace and remove tank ignore reference
#include "RuntimeMeshSection.h"
#include "ProceduralMeshTerrain.h"


AProceduralMeshTerrain::AProceduralMeshTerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent"));
	RootComponent = RuntimeMeshComponent;
}


// C++ Equivalent of Construction Script
void AProceduralMeshTerrain::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
}


void AProceduralMeshTerrain::BeginPlay()
{
	Super::BeginPlay();
	GenerateMesh(false);
}


void AProceduralMeshTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// check if any sections request to be updated
	if (SectionUpdateQueue.Num() > 0 && bAllowedToUpdateSection)
	{
		bAllowedToUpdateSection = false;
		FillSectionVertStruct(SectionUpdateQueue[0]);
		if (SectionActors.IsValidIndex(SectionUpdateQueue[0])) { SectionActors[SectionUpdateQueue[0]]->UpdateSection(); }
	}
}


// Main Function 
void AProceduralMeshTerrain::GenerateMesh(bool CalculateTangentsForMesh)
{
	InitializeProperties();
	FillGlobalProperties();
	SpawnSectionActors();
}


void AProceduralMeshTerrain::InitializeProperties()
{
	FillIndexBuffer();

	int32 NumOfSections = ComponentXY * ComponentXY;
	SectionActors.SetNum(NumOfSections, true);

	// Triangles only have to be created once, as the order stays the same for every section 
	URuntimeMeshLibrary::CreateGridMeshTriangles(SectionXY, SectionXY, false, OUT SectionProperties.Triangles);

	// Size property arrays to total num of verts of terrain
	int32 MeshVertsPerSide = SectionXY * ComponentXY - (ComponentXY - 1);
	int32 TotalNumOfVerts = MeshVertsPerSide * MeshVertsPerSide;
	GlobalProperties.Normals.SetNum(TotalNumOfVerts, true);
	GlobalProperties.UV.SetNum(TotalNumOfVerts, true);
	GlobalProperties.Vertices.SetNum(TotalNumOfVerts, true);

	// Size property arrays to num of verts in single section
	int32 NumOfVertsInSingleSection = SectionXY * SectionXY;
	SectionProperties.Normals.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.Tangents.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.UV.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.VertexColors.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.Vertices.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.IsOnBorder.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.IsOnEdge.SetNum(NumOfVertsInSingleSection, true);
	SectionProperties.PositionInsideSection.SetNum(NumOfVertsInSingleSection, true);

	AddBorderVerticesToSectionProperties();
}


void AProceduralMeshTerrain::FillIndexBuffer()
{
	int32 ArraySizeGlobal = SectionXY * SectionXY * ComponentXY * ComponentXY;
	IndexBuffer.SetNum(ArraySizeGlobal, true);
	int32 Iterator = 0;
	for (int XComp = 0; XComp < ComponentXY; XComp++)
	{
		for (int YComp = 0; YComp < ComponentXY; YComp++)
		{
			int32 GlobalXYVerts = (SectionXY - 1) * ComponentXY + 1;
			int32 ToAddWhenIteratingComponentX = GlobalXYVerts * (SectionXY - 1);
			int32 ToAddWhenIteratingComponentY = (SectionXY - 1);
			int32 SectionRoot = (ToAddWhenIteratingComponentX * XComp) + (ToAddWhenIteratingComponentY * YComp);
			for (int XSection = 0; XSection < SectionXY; XSection++)
			{
				for (int YSection = 0; YSection < SectionXY; YSection++)
				{
					int32 ToAddWhenIteratingSectionX = GlobalXYVerts;
					int32 ToAddWhenIteratingSectionY = YSection;
					int32 IndexToAdd = ToAddWhenIteratingSectionX * XSection + ToAddWhenIteratingSectionY;
					int32 IndexTotal = SectionRoot + IndexToAdd;
					IndexBuffer[Iterator] = IndexTotal;
					Iterator++;
				}
			}
		}
	}
}


void AProceduralMeshTerrain::AddBorderVerticesToSectionProperties()
{
	// Get all Vertices that are bordering the Section. Whenever one of these are changing Location, neighbour sections have to be updated too 
	for (int32 X = 0; X < SectionXY; X++)
	{
		for (int32 Y = 0; Y < SectionXY; Y++)
		{
			int32 i = X * SectionXY + Y;
			FVector2D Ratio = FVector2D(X, Y) / FVector2D(SectionXY - 1, SectionXY - 1);
			EVertPositionInsideSection VertPositionInsideSection = EVertPositionInsideSection::SB_NotOnBorder;

			// Is On Edge
			if (Ratio.Equals(FVector2D(0, 0))) { VertPositionInsideSection = EVertPositionInsideSection::SB_EdgeBottomLeft; }
			if (Ratio.Equals(FVector2D(1, 0))) { VertPositionInsideSection = EVertPositionInsideSection::SB_EdgeTopLeft; }
			if (Ratio.Equals(FVector2D(0, 1))) { VertPositionInsideSection = EVertPositionInsideSection::SB_EdgeBottomRight; }
			if (Ratio.Equals(FVector2D(1, 1))) { VertPositionInsideSection = EVertPositionInsideSection::SB_EdgeTopRight; }

			// Is On Border
			if (VertPositionInsideSection == EVertPositionInsideSection::SB_NotOnBorder)
			{
				if (FMath::IsNearlyEqual(Ratio.X, 0)) { VertPositionInsideSection = EVertPositionInsideSection::SB_BorderBottom; }
				if (FMath::IsNearlyEqual(Ratio.X, 1)) { VertPositionInsideSection = EVertPositionInsideSection::SB_BorderTop; }
				if (FMath::IsNearlyEqual(Ratio.Y, 0)) { VertPositionInsideSection = EVertPositionInsideSection::SB_BorderLeft; }
				if (FMath::IsNearlyEqual(Ratio.Y, 1)) { VertPositionInsideSection = EVertPositionInsideSection::SB_BorderRight; }
			}

			SectionProperties.PositionInsideSection[i] = VertPositionInsideSection;
			SectionProperties.VertexColors[i] = (VertPositionInsideSection == EVertPositionInsideSection::SB_NotOnBorder) ? (FColor(255, 255, 255, 0.0)) : (FColor(255, 0, 0, 1));		
		}
	}
}


void AProceduralMeshTerrain::FillGlobalProperties()
{
	// Get GlobalProperties Vertex & UV Coordinates
	for (int i = 0; i < GlobalProperties.Vertices.Num(); i++)
	{
		int32 VertsPerSide = (ComponentXY*SectionXY - (ComponentXY - 1));
		int32 X = i / VertsPerSide;
		int32 Y = i % VertsPerSide;

		// UV Coordinates
		FVector2D IterUV = FVector2D(X, Y);
		GlobalProperties.UV[i] = IterUV;

		// Vertex Coordinates & Normals
		FVector VertCoords = FVector(X, Y, 0) * QuadSize;
		CopyLandscapeHeightBelow(OUT VertCoords, OUT GlobalProperties.Normals[i]);
		GlobalProperties.Vertices[i] = VertCoords;
	}
}


void AProceduralMeshTerrain::CopyLandscapeHeightBelow(FVector &Coordinates, FVector& Normal)
{
	FHitResult Hit;
	TArray<AActor*> ToIgnore;
	//ToIgnore.Add(GetWorld()->GetFirstPlayerController()->GetPawn()); // Attention: causes crash when generated in constructin script

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), Cast<APawn>(GetWorld()->GetFirstPlayerController()->GetPawn())->GetClass(), OUT ToIgnore);

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
	Normal = (Hit.bBlockingHit) ? Normal = Hit.Normal : Normal = FVector(0, 0, -1);
}


void AProceduralMeshTerrain::SpawnSectionActors()
{
	if (!ClassToSpawnAsSection) { UE_LOG(LogTemp, Error, TEXT("ClassToSpawnAsSection Not Set in TerrainGenerator BP")); return; }

	// Iterate through amount of Components/Sections
	for (int32 X = 0; X < ComponentXY; X++)
	{
		for (int32 Y = 0; Y < ComponentXY; Y++)
		{
			// Spawn the SectionActor & Attach to this
			int32 SectionIndex = X * ComponentXY + Y;
			SectionActors[SectionIndex] = GetWorld()->SpawnActor<ARuntimeMeshSection>(
				ClassToSpawnAsSection,
				GetActorLocation(),
				GetActorRotation());
			SectionActors[SectionIndex]->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			SectionActors[SectionIndex]->InitializeOnSpawn(SectionIndex, FVector2D(X, Y), this);

			// Fill SectionProperties and create section inside SectionActor
			FillSectionVertStruct(SectionIndex);
			SectionActors[SectionIndex]->CreateSection();
		}
	}
}


void AProceduralMeshTerrain::FillSectionVertStruct(int32 SectionIndex)
{
	int32 IndexStart = SectionXY * SectionXY * SectionIndex;
	int32 IndexEnd = IndexStart + (SectionXY * SectionXY);
	for (int i = 0; i + IndexStart < IndexEnd; i++)
	{
		if (SectionProperties.Vertices.IsValidIndex(i))
		{
			int32 Index = IndexBuffer[i + IndexStart];
			SectionProperties.Vertices[i] = GlobalProperties.Vertices[Index];;
			SectionProperties.Normals[i] = GlobalProperties.Normals[Index];
			SectionProperties.UV[i] = GlobalProperties.UV[Index];
		}
	}
}


void AProceduralMeshTerrain::SectionRequestsUpdate(int32 SectionIndex, FVector HitLocation)
{
	MakeCrater(SectionIndex, HitLocation);
	//if (!SectionUpdateQueue.Contains(SectionIndex)) { SectionUpdateQueue.Add(SectionIndex); }		// TODO add neighbour section if border vert is hit
}

void AProceduralMeshTerrain::MakeCrater(int32 SectionIndex, FVector HitLocation)
{
	// Get Coordinates of hit Vertex
	// FVector RelativeHitLocation = (HitLocation - GetActorLocation());
	FVector CenterCoordinates = FVector(FMath::RoundToInt((HitLocation - GetActorLocation()).X / QuadSize), FMath::RoundToInt((HitLocation - GetActorLocation()).Y / QuadSize), 0);
	int32 VertsPerSide = (SectionXY - 1) * ComponentXY + 1; // (SectionXY * ComponentXY) - (ComponentXY - 1);
	int32 CenterIndex = CenterCoordinates.X * VertsPerSide + CenterCoordinates.Y;
	TArray<int32> AffectedSections;

	// Modify Verts around impact to make a crater
	for (int32 X = -HitRadius; X <= HitRadius; X++)
	{
		for (int32 Y = -HitRadius; Y <= HitRadius; Y++)
		{
			// Continue loop if Vert doesn't exist
			int32 CurrentIndex = CenterIndex + (X * VertsPerSide) + Y;
			if (!GlobalProperties.Vertices.IsValidIndex(CurrentIndex)) { continue; }

			// Makes crater round by sorting out verts that are to far away from center vert
			FVector CurrentVertCoords = FVector(FMath::RoundToInt(GlobalProperties.Vertices[CurrentIndex].X / QuadSize), FMath::RoundToInt(GlobalProperties.Vertices[CurrentIndex].Y / QuadSize), 0);
			float DistanceFromCenter = FVector::Dist(CenterCoordinates, CurrentVertCoords);
			if (DistanceFromCenter > HitRadius) { continue; }

			// update Vertex location and normal
			GlobalProperties.Vertices[CurrentIndex] -= FVector(0, 0, 100);
			GlobalProperties.Normals[CurrentIndex] = FVector(0, 0.5, 0.5);

			FVector SectionCoordinates = FVector(SectionIndex / (ComponentXY), SectionIndex % (ComponentXY), 0);
			FVector SectionVertCoords = CurrentVertCoords - (SectionCoordinates * SectionXY - SectionCoordinates);
			int32 SectionVertIndex = SectionVertCoords.X * SectionXY + SectionVertCoords.Y;

			//UE_LOG(LogTemp, Warning, TEXT("SectionCoords: %s  SectionIndex: %i  SectionVertIndex: %i  SectionVertCoords: %s"), *SectionCoordinates.ToString(), SectionIndex, SectionVertIndex, *SectionVertCoords.ToString());
			if (SectionVertCoords.X > SectionXY - 1 || SectionVertCoords.X < 0 || SectionVertCoords.Y > SectionXY - 1 || SectionVertCoords.Y < 0) { continue; }
			if (!SectionProperties.PositionInsideSection.IsValidIndex(SectionVertIndex)) { continue; }
			int32 NeighbourSection;
			switch (SectionProperties.PositionInsideSection[SectionVertIndex])
			{
			case EVertPositionInsideSection::SB_NotOnBorder:
			{
				if (!AffectedSections.Contains(SectionIndex)) { AffectedSections.Add(SectionIndex); }
				break;
			}
			case EVertPositionInsideSection::SB_BorderRight:
			{
				NeighbourSection = SectionIndex + 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_BorderLeft:
			{
				NeighbourSection = SectionIndex -1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_BorderTop:
			{
				NeighbourSection = SectionIndex + ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_BorderBottom:
			{
				NeighbourSection = SectionIndex - ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_EdgeBottomLeft:
			{
				NeighbourSection = SectionIndex - 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex - ComponentXY - 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex - ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_EdgeBottomRight:
			{
				NeighbourSection = SectionIndex + 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex - ComponentXY + 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex - ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_EdgeTopLeft:
			{
				NeighbourSection = SectionIndex - 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex + ComponentXY - 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex + ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			case EVertPositionInsideSection::SB_EdgeTopRight:
			{
				NeighbourSection = SectionIndex + 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex + ComponentXY + 1;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				NeighbourSection = SectionIndex + ComponentXY;
				if (!AffectedSections.Contains(NeighbourSection)) { AffectedSections.Add(NeighbourSection); }
				break;
			}
			}
		}
	}
	for (int32 Iter : AffectedSections)
	{
		if (SectionActors.IsValidIndex(Iter))
		{
			UE_LOG(LogTemp, Warning, TEXT("SECTION TO UPDATE: %i"), Iter);
		}
	}

	SectionUpdateQueue = AffectedSections;
}


void AProceduralMeshTerrain::SectionUpdateFinished()
{
	SectionUpdateQueue.RemoveAt(0);
	bAllowedToUpdateSection = true;
}


float AProceduralMeshTerrain::GetSizeOfGlobalProperties()
{
	return GlobalProperties.Vertices.GetAllocatedSize();
}


/*
// use section coordinates to access sectionproperties.positioninsidesection
if (!SectionProperties.PositionInsideSection.IsValidIndex(SectionVertIndex)) { UE_LOG(LogTemp, Error, TEXT("VertIndexNotValid"));  continue; }
switch (SectionProperties.PositionInsideSection[SectionVertIndex])
{
case EVertPositionInsideSection::SB_BorderRight:
{
UE_LOG(LogTemp, Warning, TEXT("SB_BorderRight"));
}
case EVertPositionInsideSection::SB_BorderLeft:
{
UE_LOG(LogTemp, Warning, TEXT("SB_BorderLeft"));
}
case EVertPositionInsideSection::SB_BorderTop:
{
UE_LOG(LogTemp, Warning, TEXT("SB_BorderTop"));
}
case EVertPositionInsideSection::SB_BorderBottom:
{
UE_LOG(LogTemp, Warning, TEXT("SB_BorderBottom"));
}
case EVertPositionInsideSection::SB_EdgeBottomLeft:
{
UE_LOG(LogTemp, Warning, TEXT("SB_EdgeBottomLeft"));
}
case EVertPositionInsideSection::SB_EdgeBottomRight:
{
UE_LOG(LogTemp, Warning, TEXT("SB_EdgeBottomRight"));
}
case EVertPositionInsideSection::SB_EdgeTopLeft:
{
UE_LOG(LogTemp, Warning, TEXT("SB_EdgeTopLeft"));
}
case EVertPositionInsideSection::SB_EdgeTopRight:
{
UE_LOG(LogTemp, Warning, TEXT("SB_EdgeTopRight"));
}

}

*************************************************



// Update Vert Normals & Visualize Updated Vert Locations
FVector IterationVertLocation = GlobalProperties.Vertices[CurrentIndex] + GetActorLocation();
FHitResult Hit;
TArray<AActor*> ToIgnore;
UKismetSystemLibrary::LineTraceSingle_NEW(
this,
IterationVertLocation,
IterationVertLocation + FVector(0, 0, 500),
UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
false,
ToIgnore,
EDrawDebugTrace::ForOneFrame,
OUT Hit,
true);
*/


/*
void AProceduralMeshTerrain::FillIndexBufferSection(int32 XComp, int32 YComp)
{
int32 ArraySizeGlobal = SectionXY * SectionXY * ComponentXY * ComponentXY;
IndexBuffer.SetNum(ArraySizeGlobal, true);
int32 Iterator = 0;

int32 GlobalXYVerts = (SectionXY - 1) * ComponentXY + 1;
int32 ToAddWhenIteratingComponentX = GlobalXYVerts * (SectionXY - 1);
int32 ToAddWhenIteratingComponentY = (SectionXY - 1);
int32 SectionRoot = (ToAddWhenIteratingComponentX * XComp) + (ToAddWhenIteratingComponentY * YComp);
for (int XSection = 0; XSection < SectionXY; XSection++)
{
for (int YSection = 0; YSection < SectionXY; YSection++)
{
int32 ToAddWhenIteratingSectionX = GlobalXYVerts;
int32 ToAddWhenIteratingSectionY = YSection;
int32 IndexToAdd = ToAddWhenIteratingSectionX * XSection + ToAddWhenIteratingSectionY;
int32 IndexTotal = SectionRoot + IndexToAdd;
IndexBuffer[Iterator] = IndexTotal;
Iterator++;
}
}
}
*/



/*void AProceduralMeshTerrain::UpdateMeshSection(int32 SectionIndex)
{
FillSectionVertStruct(SectionIndex);

RuntimeMeshComponent->UpdateMeshSection(
SectionIndex,
SectionProperties.Vertices,
SectionProperties.Normals,
SectionProperties.UV,
SectionProperties.VertexColors,
SectionProperties.Tangents);
bAllowedToUpdateSection = true;
}*/



/**********************************************************************************************************/
/** THE FOLLOWING CODE CREATES A NEW THREAD THAT CALCULATES PRIME NUMBERS WITHOUT FREEZING THE GAMETHEAD **/
/** CREDIT GOES TO: http://orfeasel.com/implementing-multithreading-in-ue4/ *******************************/
/** MIGHT BE USEFUL IN THE FUTURE TO CALCULATE SOME STUFF *************************************************/
/**********************************************************************************************************/

void AProceduralMeshTerrain::CalculatePrimeNumbers()
{
	//Performing the prime numbers calculations in the game thread...

	ThreadingTest::CalculatePrimeNumbers(MaxPrime);

	GLog->Log("--------------------------------------------------------------------");
	GLog->Log("End of prime numbers calculation on game thread");
	GLog->Log("--------------------------------------------------------------------");

}

void AProceduralMeshTerrain::CalculatePrimeNumbersAsync()
{
	/*Create a new Task and pass as a parameter our MaxPrime
	Then, tell that Task to execute in the background.

	The FAutoDeleteAsyncTask will make sure to delete the task when it's finished.

	Multithreading requires cautious handle of the available threads, in order to avoid
	race conditions and strange bugs that are not easy to solve

	Fortunately, UE4 contains a class (FAutoDeleteAsyncTask) which handles everything by itself
	and the programmer is able to perform async operations without any real effort.*/

	(new FAutoDeleteAsyncTask<PrimeCalculationAsyncTask>(MaxPrime))->StartBackgroundTask();
}
