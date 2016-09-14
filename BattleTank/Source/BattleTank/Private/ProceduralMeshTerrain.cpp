// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
//#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "RuntimeMeshComponent.h" 
#include "RuntimeMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h" // contains the line trace that is used in blueprint 
#include "ProceduralMeshTerrain.h"


AProceduralMeshTerrain::AProceduralMeshTerrain()
{
	PrimaryActorTick.bCanEverTick = false;
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent")); // Testing
	RootComponent = RuntimeMeshComponent;
}


void AProceduralMeshTerrain::BeginPlay()
{
	Super::BeginPlay();
	RuntimeMeshComponent->OnComponentHit.AddDynamic(this, &AProceduralMeshTerrain::OnHit);
	GenerateMesh(false);
}


void AProceduralMeshTerrain::GenerateMesh(bool CalculateTangentsForMesh) // parameter temporarly replaced with "bCalculateTangents"
{
	int32 ArraySize = (SectionXY) * (SectionXY);
	Vertices.SetNum(ArraySize, true);
	UV.SetNum(ArraySize, true);
	int32 StructSize = ComponentXY*ComponentXY;
	URuntimeMeshLibrary::CreateGridMeshTriangles(SectionXY, SectionXY, false, OUT Triangles);
	SectionPropertiesStruct.SetNum(ComponentXY * ComponentXY, true);

	for (int32 i = 0; i < ComponentXY*ComponentXY; i++)
	{
		auto ComponentOffsetX = i / ComponentXY;
		auto ComponentOffsetY = i % ComponentXY;
		FillVerticesArray(ComponentOffsetX, ComponentOffsetY);
		
		TArray<FProcMeshTangent> TangentsProcMesh;
		if (bCalculateTangents)
		{
			UKismetProceduralMeshLibrary::CalculateTangentsForMesh( // TODO replace this once v.2.0 of RuntimeMesh comes out
				Vertices, Triangles, 
				UV, 
				OUT Normals, 
				OUT TangentsProcMesh); 
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

		SectionPropertiesStruct[i].Vertices = Vertices;
		SectionPropertiesStruct[i].Triangles = Triangles;
		SectionPropertiesStruct[i].UV = UV;
		SectionPropertiesStruct[i].Normals = Normals;
		SectionPropertiesStruct[i].VertexColors = VertexColors;
		SectionPropertiesStruct[i].Tangents = Tangents;
	}


	//SectionPropertiesStruct.SetNum(10, true);
	UE_LOG(LogTemp, Warning, TEXT("SectionPropertiesStruct length: %i"), SectionPropertiesStruct.Num());
	UE_LOG(LogTemp, Warning, TEXT("VerticesInSaveStruct: %i"), SectionPropertiesStruct[0].Vertices.Num());
}


void AProceduralMeshTerrain::FillVerticesArray(float OffsetX, float OffsetY)
{
	FVector ComponentOffset = FVector(OffsetX, OffsetY, 0) * (SectionXY-1);
	auto RootOffset = 0; //SectionXY / 2; // note: SectionXY/2 is done so the procmesh middle is at 0,0 of the root
	for (int32 j = 0; j < Vertices.Num(); j++)
	{
		auto LoopX = (j / SectionXY) + ComponentOffset.X - RootOffset;
		auto LoopY = (j % SectionXY) + ComponentOffset.Y - RootOffset;
		
		FVector Coordinates = FVector(LoopX, LoopY, 0) * QuadSize;
		CopyLandscapeHeightBelow(OUT Coordinates); // TODO figuere out how to use heightmap instead or use noise, and as a bonus figure out how to export heightmap for save/load
		
		Vertices[j] = Coordinates;
		UV[j] = FVector2D(LoopX, LoopY);
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


void AProceduralMeshTerrain::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	FVector2D LocalCoordinates;
	int32 SectionIndex = 0;
	GetCoordinates(Hit.Location, OUT LocalCoordinates, OUT SectionIndex);
	int32 HitVertex = LocalCoordinates.X * SectionXY + LocalCoordinates.Y;
	
	//int32 HitVertexLocal = (LocalCoordinates.X - (SectionIndex * SectionXY)) + (LocalCoordinates.Y - (SectionIndex * SectionXY));
	auto HitVertexLocalX = LocalCoordinates.X % SectionXY; // TODO fix modulo and get right vector 
	auto HitVertexLocalY = LocalCoordinates.Y % SectionXY;
	int32 HitVertexLocal = HitVertexLocalX * SectionXY + HitVertexLocalY;

	UE_LOG(LogTemp, Warning, TEXT("VertexIndexLocal: %i"), HitVertexLocal);


	if (!SectionPropertiesStruct.IsValidIndex(SectionIndex)) { return; }
	if (!SectionPropertiesStruct[SectionIndex].Vertices.IsValidIndex(HitVertexLocal)) { return; }

	auto Test = SectionPropertiesStruct[SectionIndex].Vertices[HitVertexLocal];
	SectionPropertiesStruct[SectionIndex].Vertices[HitVertexLocal] = Test + FVector(0, 0, -100);


	RuntimeMeshComponent->UpdateMeshSection(
		SectionIndex,
		SectionPropertiesStruct[SectionIndex].Vertices,
		SectionPropertiesStruct[SectionIndex].Normals,
		SectionPropertiesStruct[SectionIndex].UV,
		SectionPropertiesStruct[SectionIndex].VertexColors,
		SectionPropertiesStruct[SectionIndex].Tangents
		);
		



	UE_LOG(LogTemp, Warning, TEXT("Updated mesh section: %i"), SectionIndex);
	UE_LOG(LogTemp, Warning, TEXT("VertexIndex: %i"), HitVertex);

}


void AProceduralMeshTerrain::GetCoordinates(FVector Location, FVector2D& LocalCoordinates, int32& SectionIndex)
{
	auto HitLocationLocal = Location - GetActorLocation();
	auto ComponentSize = (SectionXY - 1) * QuadSize;

	auto XHit = HitLocationLocal.X / QuadSize;
	auto YHit = HitLocationLocal.Y / QuadSize;
	LocalCoordinates = FVector2D(FMath::RoundToInt(XHit), FMath::RoundToInt(YHit)); // TODO right now returns global coordinates, make it local by subtracting

	int32 XCompHit = HitLocationLocal.X / ComponentSize;
	int32 YCompHit = HitLocationLocal.Y / ComponentSize;
	FVector2D ComponentCoordinates = FVector2D(XCompHit, YCompHit);

	SectionIndex = ComponentXY * XCompHit + YCompHit;
	UE_LOG(LogTemp, Warning, TEXT("SectionCoordinates: %s ComponentCoordinates: %s SectionIndex: %i"), *LocalCoordinates.ToString(), *ComponentCoordinates.ToString(), SectionIndex);
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