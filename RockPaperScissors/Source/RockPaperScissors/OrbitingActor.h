// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbitingActor.generated.h"

class USphereComponent;
class URotatingMovementComponent;

UENUM()
enum SphereType
{
	Rock,
	Paper,
	Scissors,
	MAX_NUMBER
};

USTRUCT()
struct FStaticMeshCompStruct
{
	GENERATED_BODY()
	SphereType type = MAX_NUMBER;
	UStaticMeshComponent* staticMeshComp = nullptr;
};

UCLASS()
class ROCKPAPERSCISSORS_API AOrbitingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbitingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	URotatingMovementComponent* m_RotatingMovementComp;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* m_StaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	float m_RadiusOfRotation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	int32 m_NumberOfSpheres;

	TArray<FStaticMeshCompStruct> m_SpheresArray;

};
