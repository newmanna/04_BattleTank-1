// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"  // forward declaration not enought to access "FRuntimeMeshTangent"
#include "ProceduralMeshTerrain.generated.h"


class ARuntimeMeshSection;


UENUM(BlueprintType)
enum class EVertPositionInsideSection : uint8
{
	SB_NotOnBorder		UMETA(DisplayName = "NotOnBorder"),
	SB_BorderLeft 		UMETA(DisplayName = "BorderLeft"),
	SB_BorderRight		UMETA(DisplayName = "BorderRight"),
	SB_BorderTop 		UMETA(DisplayName = "BorderTop"),
	SB_BorderBottom 	UMETA(DisplayName = "BorderBottom"),
	SB_EdgeTopLeft 		UMETA(DisplayName = "EdgeTopLeft"),
	SB_EdgeTopRight 	UMETA(DisplayName = "EdgeTopRight"),
	SB_EdgeBottomLeft 	UMETA(DisplayName = "EdgeBottomLeft"),
	SB_EdgeBottomRight 	UMETA(DisplayName = "EdgeBottomRight")
};


// Struct that constains global Vertex information
USTRUCT(BlueprintType)
struct FGlobalProperties
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector2D> UV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<FVector> Normals;

	FGlobalProperties()
	{
	}
};


// Struct that constains necessary Vertex information of a single section
USTRUCT(BlueprintType)
struct FSectionProperties
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TArray<EVertPositionInsideSection> PositionInsideSection;

	FSectionProperties()
	{
	}
};


UCLASS()
class BATTLETANK_API AProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()

public:
	// Main function to generate Mesh
	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshGeneration")
	void GenerateMesh(bool CalculateTangentsForMesh);

	FSectionProperties SectionProperties;


	// Getters 
	UFUNCTION(BlueprintPure, Category = "ProceduralMeshGeneration")
	int32 GetSectionXY() const { return SectionXY; }
	UFUNCTION(BlueprintPure, Category = "ProceduralMeshGeneration")
	float GetQuadSize() const { return QuadSize; }
	UFUNCTION(BlueprintPure, Category = "ProceduralMeshGeneration")
	int32 GetComponentXY() const { return ComponentXY; }
	UFUNCTION(BlueprintPure, Category = "ProceduralMeshGeneration")
	float GetSizeOfGlobalProperties();
	FSectionProperties GetSectionProperties() const { return SectionProperties; }

	// called from section actor on projectile hit
	void SectionRequestsUpdate(int32 SectionIndex, FVector HitLocation);
	void SectionUpdateFinished();

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
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	float SectionVisibilityRange = 80000;
	UPROPERTY(EditAnywhere, Category = "ProceduralMeshGeneration")
	int32 HitRadius = 2;

	// class that acts as a mesh section
	UPROPERTY(EditDefaultsOnly, Category = "ProceduralMeshGeneration")
	TSubclassOf<ARuntimeMeshSection> ClassToSpawnAsSection;

private:
	AProceduralMeshTerrain();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	void InitializeProperties();
	void FillIndexBuffer();
	void AddBorderVerticesToSectionProperties();

	void FillGlobalProperties();
	void CopyLandscapeHeightBelow(FVector& Coordinates, FVector& Normal);

	void SpawnSectionActors();
	void FillSectionVertStruct(int32 SectionIndex);
	//void FillIndexBufferSection(int32 XComp, int32 YComp);

	void MakeCrater(int32 SectionIndex, FVector HitLocation);


	 
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URuntimeMeshComponent* RuntimeMeshComponent = nullptr;

	FGlobalProperties GlobalProperties;
	TArray<int32> IndexBuffer;

	TArray<int32> SectionUpdateQueue; 
	bool bAllowedToUpdateSection = true;
	TArray<int32> SectionCreateQueue;
	TArray<ARuntimeMeshSection*> SectionActors;

	

	/**********************************************************************************************************/
	/** THE FOLLOWING CODE CREATES A NEW THREAD THAT CALCULATES PRIME NUMBERS WITHOUT FREEZING THE GAMETHEAD **/
	/** CREDIT GOES TO: http://orfeasel.com/implementing-multithreading-in-ue4/ *******************************/
	/** MIGHT BE USEFUL IN THE FUTURE *************************************************************************/
	/**********************************************************************************************************/
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
