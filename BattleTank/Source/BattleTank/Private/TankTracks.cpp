// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleTank.h"
#include "TankTracks.h"


UTankTracks::UTankTracks()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTankTracks::BeginPlay()
{
	OnComponentHit.AddDynamic(this, &UTankTracks::OnHit);	
}


/*
 void UTankTracks::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (HoverSuspension())
	{
		ApplySidewaysForce();
		DriveTrack();
		CurrentThrottle = 0;
	}
}
*/

void UTankTracks::OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	// UE_LOG(LogTemp, Warning, TEXT("Im Hit"))
	ApplySidewaysForce();
	DriveTrack();
	CurrentThrottle = 0;
}


void UTankTracks::ApplySidewaysForce()
{
	auto SlippageSpeed = FVector::DotProduct(GetRightVector(), GetComponentVelocity().GetSafeNormal());
	auto DeltaTime = GetWorld()->DeltaTimeSeconds;
	auto CorrectionAcceleration = -SlippageSpeed / DeltaTime * GetRightVector();
	auto TankRoot = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent());
	auto CorrectionForce = (TankRoot->GetMass() * CorrectionAcceleration) / 2;  // because two tracks diveded by 2

	TankRoot->AddForce(CorrectionForce);
	//UE_LOG(LogTemp, Warning, TEXT("SlippageSpeed: %f"), SlippageSpeed)
}


void UTankTracks::SetThrottle(float Throttle)
{
	CurrentThrottle = FMath::Clamp<float>(CurrentThrottle + Throttle, -1, 1);
}


void UTankTracks::DriveTrack()
{
	auto ForceApplied = GetForwardVector() * CurrentThrottle * TrackMaxDrivingForce;
	auto ForceLocation = GetComponentLocation();
	auto TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()); // cast is done to get a specific parent class that can be used to apply physics
	TankRoot->AddForceAtLocation(ForceApplied, ForceLocation);
	//UE_LOG(LogTemp, Warning, TEXT("%s Throttle: %f"), *Name, Throttle);
}

/*
//custom
bool UTankTracks::HoverSuspension()
{
	bool bInHoverDistance = false;
	TArray<FName> AllSocketNames = GetAllSocketNames();
	for(FName CurrentSocket : AllSocketNames)
	{
		// get current socket transform
		FTransform SocketWorldTransform = GetSocketTransform(CurrentSocket);

		//LineTrace parameters
		FVector TraceStart = SocketWorldTransform.GetTranslation();
		FVector TraceEnd = TraceStart + SocketWorldTransform.GetRotation().GetForwardVector() * DesiredDistanceAboveGround;
		FHitResult HitResult;

		//Get DistanceAboveGround
		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility);
		float DistanceAboveGround = (HitResult.Location - TraceStart).Size();

		// only add force if inside DesiredDistanceAboveGround
		{
			bInHoverDistance = true;
			// Apply more or less force depending on distance 
			float PerSocketForceMax = (TrackTotalHoverForce / AllSocketNames.Num()) / GetWorld()->DeltaTimeSeconds;
			FVector SocketForward = SocketWorldTransform.GetRotation().GetForwardVector();
			FVector ForceToApply = SocketForward * PerSocketForceMax;

			float DistanceLerpAlpha = DistanceAboveGround / DesiredDistanceAboveGround;
			
			
			auto bFlipForce = (DistanceAboveGround < DesiredDistanceAboveGround) ? true : false;
			auto alphaTest = (DistanceAboveGround < DesiredDistanceAboveGround) ? (DistanceAboveGround / DesiredDistanceAboveGround) : (DesiredDistanceAboveGround / DistanceAboveGround);
			auto ForceTest = bFlipForce ? ForceToApply : ForceToApply * -0.5;

			
			ForceInterped = FMath::VInterpTo(ForceInterped, ForceTest, GetWorld()->DeltaTimeSeconds, 1);


			FVector PerSocketForceLerped = FMath::LerpStable(ForceTest, FVector(0, 0, 0), alphaTest);
			auto TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
			TankRoot->AddForceAtLocation(PerSocketForceLerped, TraceStart);


			DrawDebugLine(
				GetWorld(),
				TraceStart,
				PerSocketForceLerped + TraceStart,
				FColor(255, 0, 0),
				false, -1, 0,
				12.333);
		}
	}
	//ApplyDownwardsForce(FVector(0,0, -15000000));
	return bInHoverDistance;
}


void UTankTracks::ApplyDownwardsForce(FVector Force)
{
	if (CurrentThrottle > 0.2 || CurrentThrottle < -0.2)
	{
		auto HalfTankLength = 350;
		auto TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
		auto Displacement = TankRoot->GetForwardVector() * HalfTankLength * CurrentThrottle;
		auto ForceLocation = TankRoot->GetComponentLocation() + Displacement;

		TankRoot->AddForceAtLocation(Force, ForceLocation);

		DrawDebugLine(
			GetWorld(),
			ForceLocation,
			ForceLocation + Force,
			FColor(255, 0, 0),
			false, -1, 0,
			12.333);
	}

}



/*

// reduce total force if track is moving up
bool bTrackMovingUp = (GetComponentVelocity().Size() > 0) ? (true) : (false);
float PerSocketForceReduced = (bTrackMovingUp) ? (PerSocketForceMax * 0.75) : (PerSocketForceMax);

auto DebugEnd = (HitResult.Location.Equals(FVector(0,0,0), 0.1)) ? (TraceEnd) : (HitResult.Location);
DrawDebugLine(
GetWorld(),
TraceStart,
DebugEnd,
FColor(255, 0, 0),
false, -1, 0,
12.333);



//custom - saved working kinda
bool UTankTracks::HoverSuspension()
{
bool bInHoverDistance = false;
TArray<FName> AllSocketNames = GetAllSocketNames();
for(FName CurrentSocket : AllSocketNames)
{
// get current socket transform
FTransform SocketWorldTransform = GetSocketTransform(CurrentSocket);

//LineTrace parameters
FVector TraceStart = SocketWorldTransform.GetTranslation();
FVector TraceEnd = TraceStart + SocketWorldTransform.GetRotation().GetForwardVector() * DesiredDistanceAboveGround;
FHitResult HitResult;

//Get DistanceAboveGround
GetWorld()->LineTraceSingleByChannel(
HitResult,
TraceStart,
TraceEnd,
ECollisionChannel::ECC_Visibility);
float DistanceAboveGround = (HitResult.Location - TraceStart).Size();

// only add force if inside DesiredDistanceAboveGround
if (DistanceAboveGround < DesiredDistanceAboveGround)
{
bInHoverDistance = true;
// Apply more or less force depending on distance
float PerSocketForceMax = (TrackTotalHoverForce / AllSocketNames.Num()) / GetWorld()->DeltaTimeSeconds;
FVector SocketForward = SocketWorldTransform.GetRotation().GetForwardVector();
FVector ForceToApply = SocketForward * PerSocketForceMax;

float DistanceLerpAlpha = DistanceAboveGround / DesiredDistanceAboveGround;
FVector PerSocketForceLerped = FMath::LerpStable(ForceToApply, FVector(0, 0, 0), DistanceLerpAlpha);
auto TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
TankRoot->AddForceAtLocation(PerSocketForceLerped, TraceStart);
}
}
//ApplyDownwardsForce(FVector(0,0, -15000000));
return bInHoverDistance;
// TODO apply downwards force on the nose when moving forward and on the back when moving back
}

*/




/*

OVERLAP EVENTS - won't work on landscape for some reason - known engine bug apparently

// on begin play
OnComponentBeginOverlap.AddDynamic(this, &UTankTracks::TriggerEnter);
OnComponentEndOverlap.AddDynamic(this, &UTankTracks::TriggerExit);


//definition
void UTankTracks::TriggerEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
UE_LOG(LogTemp, Warning, TEXT("Overlap begin"))
}

void UTankTracks::TriggerExit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
UE_LOG(LogTemp, Warning, TEXT("Overlap end"))
}


//declaration
UFUNCTION()
void TriggerEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
UFUNCTION()
void TriggerExit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

*/