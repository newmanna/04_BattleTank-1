// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "RuntimeMeshComponent.h"
#include "ProceduralMeshTerrain.h"
#include "RuntimeMeshSection.h"


ARuntimeMeshSection::ARuntimeMeshSection()
{
	PrimaryActorTick.bCanEverTick = false;
	//SceneRoot = CreateDefaultSubobject<USphereComponent>(TEXT("SceneRoot")); 
	//RootComponent = SceneRoot;
	
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent")); // Testing
	RootComponent = RuntimeMeshComponent;
}


void ARuntimeMeshSection::InitializeOnSpawn(int32 SectionIndex, AProceduralMeshTerrain* Terrain)
{
	OwningTerrain = Terrain;
	SectionIndexLocal = SectionIndex;
	SectionCoordinates = SectionCoordinates;

	UE_LOG(LogTemp, Warning, TEXT("SectionIndex: %i Owner: %s "), SectionIndexLocal, *OwningTerrain->GetName());
}


void ARuntimeMeshSection::BeginPlay()
{
	Super::BeginPlay();	
	RuntimeMeshComponent->OnComponentHit.AddDynamic(this, &ARuntimeMeshSection::OnHit);

}


void ARuntimeMeshSection::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}


void ARuntimeMeshSection::CreateSection()
{
	auto SectionProperties = OwningTerrain->GetSectionProperties();
	RuntimeMeshComponent->CreateMeshSection(
		0,
		SectionProperties.Vertices,
		SectionProperties.Triangles,
		SectionProperties.Normals,
		SectionProperties.UV,
		SectionProperties.VertexColors,
		SectionProperties.Tangents,
		true);
}


void ARuntimeMeshSection::UpdateSection()
{
	auto SectionProperties = OwningTerrain->GetSectionProperties();
	RuntimeMeshComponent->UpdateMeshSection(
		0,
		SectionProperties.Vertices,
		SectionProperties.Normals,
		SectionProperties.UV,
		SectionProperties.VertexColors,
		SectionProperties.Tangents);

	OwningTerrain->SectionUpdateFinished();

	UE_LOG(LogTemp, Warning, TEXT("Updated Section: %i"), SectionIndexLocal);

}


void ARuntimeMeshSection::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	OwningTerrain->SectionRequestsUpdate(SectionIndexLocal, Hit.Location);
	
	//FVector HitVectorRelativeLocation =  Hit.Location - GetActorLocation();
	

	UE_LOG(LogTemp, Warning, TEXT("Hit Section: %i NormalImpulse: %s"), SectionIndexLocal, *Hit.Normal.ToString());
}