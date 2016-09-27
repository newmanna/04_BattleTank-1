// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "RuntimeMeshComponent.h"
#include "ProceduralMeshTerrain.h"
#include "RuntimeMeshSection.h"


ARuntimeMeshSection::ARuntimeMeshSection()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a RuntimeMeshComponent and make it the root
	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent"));
	RootComponent = RuntimeMeshComponent;
}


void ARuntimeMeshSection::BeginPlay()
{
	Super::BeginPlay();
	RuntimeMeshComponent->OnComponentHit.AddDynamic(this, &ARuntimeMeshSection::OnHit);

}


// References the owning Terrain Generator and initializes some variables
void ARuntimeMeshSection::InitializeOnSpawn(int32 SectionIndex, FVector2D ComponentCoordinates, AProceduralMeshTerrain* Terrain)
{
	OwningTerrain = Terrain;
	SectionIndexLocal = SectionIndex;
	SectionCoordinates = ComponentCoordinates;

	auto SectionSideInCM = OwningTerrain->GetSectionXY() * OwningTerrain->GetQuadSize();
	SectionCenterWorldLocation2D = FVector2D(ComponentCoordinates.X * SectionSideInCM, ComponentCoordinates.Y * SectionSideInCM) + FVector2D(GetActorLocation().X, GetActorLocation().Y);

	PlayerControllerReference = GetWorld()->GetFirstPlayerController();
}


void ARuntimeMeshSection::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (!ensure(PlayerControllerReference)) { return; }

	auto PlayerPawnWorldLocation = PlayerControllerReference->GetPawn()->GetActorLocation();
	FVector2D PlayerPawnWorldLocation2D = FVector2D(PlayerPawnWorldLocation.X, PlayerPawnWorldLocation.Y);
	float DistanceToPlayerPawn2D = FVector2D::Distance(PlayerPawnWorldLocation2D, SectionCenterWorldLocation2D);
	
	if (DistanceToPlayerPawn2D > OwningTerrain->SectionVisibilityRange && RuntimeMeshComponent->IsVisible())
	{
		RuntimeMeshComponent->SetVisibility(false);
	}
	else if (DistanceToPlayerPawn2D < OwningTerrain->SectionVisibilityRange && !RuntimeMeshComponent->IsVisible()) 
	{
		RuntimeMeshComponent->SetVisibility(true);
		RuntimeMeshComponent->SetMeshSectionCollisionEnabled(0, true);

	}
}


// Called from Terrain Generator on spawn
void ARuntimeMeshSection::CreateSection()
{
	FSectionProperties* SectionPropertiesPtr = &OwningTerrain->SectionProperties;
	RuntimeMeshComponent->CreateMeshSection(
		0,
		SectionPropertiesPtr->Vertices,
		SectionPropertiesPtr->Triangles,
		SectionPropertiesPtr->Normals,
		SectionPropertiesPtr->UV,
		SectionPropertiesPtr->VertexColors,
		SectionPropertiesPtr->Tangents,
		true);
}


// Send request to Terrain Generator whenever a projectile hits the RuntimeMeshComponent of this section
void ARuntimeMeshSection::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	OwningTerrain->SectionRequestsUpdate(SectionIndexLocal, Hit.Location);
}


// Called from Terrain Generator after receiving update request
void ARuntimeMeshSection::UpdateSection()
{
	//auto SectionProperties = OwningTerrain->GetSectionProperties();
	FSectionProperties* SectionPropertiesPtr = &OwningTerrain->SectionProperties;
	RuntimeMeshComponent->UpdateMeshSection(
		0,
		SectionPropertiesPtr->Vertices,
		SectionPropertiesPtr->Normals,
		SectionPropertiesPtr->UV,
		SectionPropertiesPtr->VertexColors,
		SectionPropertiesPtr->Tangents);

	OwningTerrain->SectionUpdateFinished();
}