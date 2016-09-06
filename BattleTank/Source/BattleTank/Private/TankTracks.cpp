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



//custom
bool UTankTracks::HoverSuspension()
{
	bool bHoveringDistanceOK = false;
	TArray<FName> HoverSocketNames = GetAllSocketNames();
	for (int32 i = 0; i < HoverSocketNames.Num(); i++) // int32 working?
	{
		FName SocketName = HoverSocketNames[i];
		FTransform SocketTransform = GetSocketTransform(SocketName);
		SocketTransform;

		//LineTrace
		float DesiredTraceLength = 75;
		float TrackTotalHoverForce = -950000;

		FVector TraceStart = SocketTransform.GetTranslation();
		FVector TraceEnd = TraceStart + SocketTransform.GetRotation().GetForwardVector() * DesiredTraceLength;
		FHitResult HitResult;

		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_WorldStatic);

		float TraceLength = (HitResult.Location - TraceStart).Size();

		if (TraceLength < DesiredTraceLength)
		{
			FVector SocketForward = SocketTransform.GetRotation().GetForwardVector();
			float PerSocketForceMax = (TrackTotalHoverForce / HoverSocketNames.Num()) / GetWorld()->DeltaTimeSeconds; // TODO check if want to apply less force if going uppwards

																													  // reduce total force if track is moving up
			bool bTrackMovingUp = (GetComponentVelocity().Size() > 0) ? (true) : (false);
			float PerSocketForceReduced = (bTrackMovingUp) ? (PerSocketForceMax * 0.75) : (PerSocketForceMax);

			FVector ForceToApply = SocketForward * PerSocketForceReduced;

			// Apply more or less force depending on distance 
			float DistanceLerpAlpha = TraceLength / DesiredTraceLength;
			FVector PerSocketForceLerped = FMath::LerpStable(ForceToApply, FVector(0, 0, 0), DistanceLerpAlpha);

			// apply force at socket location
			auto TankRoot = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent());
			TankRoot->AddForceAtLocation(PerSocketForceLerped, TraceStart);

			bHoveringDistanceOK = true;
		}
		else
		{
			bHoveringDistanceOK = false;
		}

		DrawDebugLine(
			GetWorld(),
			TraceStart,
			HitResult.Location,
			FColor(255, 0, 0),
			false, -1, 0,
			12.333);
	}
	return bHoveringDistanceOK;
}



/*


auto LineTraceFrontStart = GetSocketLocation("SuspensionFront");
auto LineTraceFrontEnd = LineTraceFrontStart + GetSocketRotation("SuspensionFront").Vector() * 20;
auto ForceFront = GetSocketRotation("SuspensionFront").Vector() * -2500000;


auto LineTraceBackStart = GetSocketLocation("SuspensionBack");
auto LineTraceBackEnd = LineTraceBackStart + GetSocketRotation("SuspensionBack").Vector() * 20;
auto ForceBack = GetSocketRotation("SuspensionBack").Vector() * -2500000;


//LineTrace
FHitResult HitResult;
GetWorld()->LineTraceSingleByChannel(
HitResult,
LineTraceFrontStart,
LineTraceFrontEnd,
ECollisionChannel::ECC_WorldStatic);

DrawDebugLine(
GetWorld(),
LineTraceFrontStart,		// TraceStart
HitResult.Location,			// TraceEnd
FColor(255, 0, 0),			// Red Green Blue
false, -1, 0,				// Ispersistent, lifetime, DepthPriority
12.333						// thickness
);

//LineTrace 2
FHitResult HitResult2;
GetWorld()->LineTraceSingleByChannel(
HitResult2,
LineTraceBackStart,
LineTraceBackEnd,
ECollisionChannel::ECC_WorldStatic);

DrawDebugLine(
GetWorld(),
LineTraceBackStart,			// TraceStart
HitResult2.Location,		// TraceEnd
FColor(255, 0, 0),			// Red Green Blue
false, -1, 0,				// Ispersistent, lifetime, DepthPriority
12.333						// thickness
);

auto TraceLengthFront = (LineTraceFrontStart - HitResult.Location).Size();
auto TraceLengthBack = (LineTraceBackStart - HitResult2.Location).Size();


auto ForceFrontFinal = ForceFront * (10 / TraceLengthFront);
auto ForceBackFinal = ForceBack * (10 / TraceLengthBack);



TankRoot->AddForceAtLocation(ForceFrontFinal, LineTraceFrontStart);
TankRoot->AddForceAtLocation(ForceBackFinal, LineTraceBackStart);

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