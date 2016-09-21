// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"  // forward declaration not enought to access "FRuntimeMeshTangent"
#include "ProceduralMeshTerrain.generated.h"


USTRUCT(BlueprintType)
struct FVertexCombined
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<int32> Triangles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector2D> UV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Normals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FColor> VertexColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FRuntimeMeshTangent> Tangents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<bool> IsOnBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<bool> IsOnEdge;

	FVertexCombined()
	{
	}
};


UCLASS()
class BATTLETANK_API AProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralMeshTerrain();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);

	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshGeneration")
	void GenerateMesh(bool CalculateTangentsForMesh);

	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	int32 ComponentXY = 1;
	
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	int32 SectionXY = 20;
	
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float QuadSize = 100;
	
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float LineTraceLength = 10000;
	
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float LineTraceHeightOffset = 100;
	
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	bool bCalculateTangents = false;

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void CopyLandscapeHeightBelow(FVector& Coordinates, FVector& Normal);
	void FillSectionVertStruct(int32 SectionIndex);
	void UpdateMeshSection(int32 SectionIndex);
	void FillIndexBuffer();
	void InitializeProperties();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	FVertexCombined GlobalProperties;
	FVertexCombined SectionProperties;
	TArray<int32> IndexBuffer;
	TArray<int32> SectionUpdateQueue; // TODO replace with TQueue
	TArray<int32> SectionCreateQueue; // TODO replace with TQueue

	bool bAllowedToUpdateSection = true;

/////////////////////////////////

protected:

	/*Calculates prime numbers in the game thread*/
	UFUNCTION(BlueprintCallable, Category = MultiThreading)
		void CalculatePrimeNumbers();

	/*Calculates prime numbers in a background thread*/
	UFUNCTION(BlueprintCallable, Category = MultiThreading)
		void CalculatePrimeNumbersAsync();

	/*The max prime number*/
	UPROPERTY(EditAnywhere, Category = MultiThreading)
		int32 MaxPrime;

};
////////////////////////////////////////////////////////////////////////////////////////


namespace ThreadingTest
{
	static void CalculatePrimeNumbers(int32 UpperLimit)
	{
		//Calculating the prime numbers...
		for (int32 i = 1; i <= UpperLimit; i++)
		{
			bool isPrime = true;

			for (int32 j = 2; j <= i / 2; j++)
			{
				if (FMath::Fmod(i, j) == 0)
				{
					isPrime = false;
					break;
				}
			}

			if (isPrime) GLog->Log("Prime number #" + FString::FromInt(i) + ": " + FString::FromInt(i));
		}
	}
}

/*PrimeCalculateAsyncTask is the name of our task
FNonAbandonableTask is the name of the class I've located from the source code of the engine*/
class PrimeCalculationAsyncTask : public FNonAbandonableTask
{
	int32 MaxPrime;

public:
	/*Default constructor*/
	PrimeCalculationAsyncTask(int32 MaxPrime)
	{
		this->MaxPrime = MaxPrime;
	}

	/*This function is needed from the API of the engine.
	My guess is that it provides necessary information
	about the thread that we occupy and the progress of our task*/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(PrimeCalculationAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	/*This function is executed when we tell our task to execute*/
	void DoWork()
	{
		ThreadingTest::CalculatePrimeNumbers(MaxPrime);

		GLog->Log("--------------------------------------------------------------------");
		GLog->Log("End of prime numbers calculation on background thread");
		GLog->Log("--------------------------------------------------------------------");
	}
};


