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


// C++ Equivalent of Construction Script
void AProceduralMeshTerrain::OnConstruction(const FTransform & Transform)
{
	auto ConstructTimeBegin = UKismetMathLibrary::UtcNow();
	UE_LOG(LogTemp, Warning, TEXT("OnConstruction() triggered in c++"));
	GenerateMesh(false);
	Super::OnConstruction(Transform);
	auto ConstructTimeEnd = UKismetMathLibrary::UtcNow();

	auto ConstructTime = UKismetMathLibrary::Subtract_DateTimeDateTime(ConstructTimeBegin, ConstructTimeBegin);
	UE_LOG(LogTemp, Warning, TEXT("ConstructionTime: %i s %i ms"), ConstructTime.GetSeconds(), ConstructTime.GetMilliseconds());

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
}


void AProceduralMeshTerrain::GenerateMesh(bool CalculateTangentsForMesh)
{
	InitializeProperties();

	// Get GlobalProperties Vertex & UV Coordinates
	for (int i = 0; i < GlobalProperties.Vertices.Num(); i++)
	{
		int32 VertsPerSide = (ComponentXY*SectionXY - (ComponentXY - 1));
		int32 X = i / VertsPerSide;
		int32 Y = i % VertsPerSide;
		
		// UV Coordinates
		FVector2D IterUV = FVector2D(X, Y);
		GlobalProperties.UV[i] = IterUV;

		// Vertex Coordinates
		FVector VertCoords = FVector(X, Y, 0) * QuadSize;
		CopyLandscapeHeightBelow(OUT VertCoords, OUT GlobalProperties.Normals[i]);
		GlobalProperties.Vertices[i] = VertCoords;
	}

	for (int i = RuntimeMeshComponent->GetNumSections(); i >= ComponentXY * ComponentXY; i--)
	{
		RuntimeMeshComponent->ClearMeshSection(i);
	}
	// Add Section Indices to Queue // TODO check if better performance with seperate RuntimeMeshComponents and single section
	for (int X = 0; X < ComponentXY; X++)
	{
		for (int Y = 0; Y < ComponentXY; Y++)
		{
			int32 SectionIndex = X * ComponentXY + Y;
			FillSectionVertStruct(SectionIndex);

			RuntimeMeshComponent->CreateMeshSection(
				SectionIndex,
				SectionProperties.Vertices,
				SectionProperties.Triangles,
				SectionProperties.Normals,
				SectionProperties.UV,
				SectionProperties.VertexColors,
				SectionProperties.Tangents,
				true);
		}
	}
}


void AProceduralMeshTerrain::InitializeProperties() 
{
	FillIndexBuffer();

	int32 MeshVertsPerSide = SectionXY * ComponentXY - (ComponentXY - 1);
	int32 ArraySizeGlobal = MeshVertsPerSide * MeshVertsPerSide;
	GlobalProperties.Normals.SetNum(ArraySizeGlobal, true);
	GlobalProperties.Tangents.SetNum(ArraySizeGlobal, true);
	GlobalProperties.UV.SetNum(ArraySizeGlobal, true);
	GlobalProperties.VertexColors.SetNum(ArraySizeGlobal, true);
	GlobalProperties.Vertices.SetNum(ArraySizeGlobal, true);

	int32 ArraySizeSection = SectionXY * SectionXY;
	SectionProperties.Normals.SetNum(ArraySizeSection, true);
	SectionProperties.Tangents.SetNum(ArraySizeSection, true);
	SectionProperties.UV.SetNum(ArraySizeSection, true);
	SectionProperties.VertexColors.SetNum(ArraySizeSection, true);
	SectionProperties.Vertices.SetNum(ArraySizeSection, true);

	// Is Vertex on Section Seam
	SectionProperties.IsOnBorder.SetNum(ArraySizeSection, true);
	SectionProperties.IsOnEdge.SetNum(ArraySizeSection, true);
	for (int X = 0; X < SectionXY; X++)
	{
		for (int Y = 0; Y < SectionXY; Y++)
		{
			int32 i = X * SectionXY + Y;

			// Is On Border TODO make enum
			bool bIsVertOnSectionBorder = (X == 0 || X == SectionXY - 1 || Y == 0 || Y == SectionXY - 1) ? true : false;
			SectionProperties.IsOnBorder[i] = bIsVertOnSectionBorder;
			
			// Is On Edge
			FVector2D Ratio = FVector2D(X, Y) / FVector2D(SectionXY - 1, SectionXY - 1); 
			bool bIsOnSectionEdge = false;
			if (Ratio.Equals(FVector2D(0, 0))) { bIsOnSectionEdge = true; }
			if (Ratio.Equals(FVector2D(1, 0))) { bIsOnSectionEdge = true; }
			if (Ratio.Equals(FVector2D(0, 1))) { bIsOnSectionEdge = true; }
			if (Ratio.Equals(FVector2D(1, 1))) { bIsOnSectionEdge = true; }
			SectionProperties.IsOnEdge[i] = bIsOnSectionEdge;
			//UE_LOG(LogTemp, Warning, TEXT("RemainderXY: %i, %i"), RemainderX, RemainderY);
		}
	}
	URuntimeMeshLibrary::CreateGridMeshTriangles(SectionXY, SectionXY, false, OUT SectionProperties.Triangles);
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
			SectionProperties.VertexColors[i]	= GlobalProperties.VertexColors[Index];
			SectionProperties.Vertices[i]		= GlobalProperties.Vertices[Index];;
			SectionProperties.Tangents[i]		= GlobalProperties.Tangents[Index];
			SectionProperties.Normals[i]		= GlobalProperties.Normals[Index];
			SectionProperties.UV[i]				= GlobalProperties.UV[Index];
		}
	}
}


void AProceduralMeshTerrain::CopyLandscapeHeightBelow(FVector &Coordinates, FVector& Normal)
{
	FHitResult Hit;
	TArray<AActor*> ToIgnore;
	//ToIgnore.Add(GetWorld()->GetFirstPlayerController()->GetPawn()); // attention causes crash when using in construction script 
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
	if (bCalculateTangents)
	{
		//Normal = Hit.ImpactNormal;
		Normal = Hit.Normal;
	}
	else
	{
		Normal = FVector(0, 0, 1);
	}
}


void AProceduralMeshTerrain::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	// Get Coordinates relative To Actor RootComponent
	FVector RelativeHitLocation = Hit.Location - GetActorLocation();
	int32 XCoordinate = FMath::RoundToInt(RelativeHitLocation.X / QuadSize);
	int32 YCoordinate = FMath::RoundToInt(RelativeHitLocation.Y / QuadSize);
	FVector2D CoordinatesRelativeToRoot = FVector2D(XCoordinate, YCoordinate);

	int32 MeshXY = SectionXY * ComponentXY;
	int32 HitIndex = XCoordinate * MeshXY + YCoordinate;

	if (GlobalProperties.Vertices.IsValidIndex(HitIndex))
	{
		GlobalProperties.Vertices[HitIndex];
		UpdateMeshSection(0);
	}

	UE_LOG(LogTemp, Warning, TEXT("HIT"));
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


void AProceduralMeshTerrain::UpdateMeshSection(int32 SectionIndex)
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
}

/////////////////////////////////////////


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