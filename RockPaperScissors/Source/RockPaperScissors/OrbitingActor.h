// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbitingActor.generated.h"

class USphereComponent;
class URotatingMovementComponent;
class UOrbitingStaticMeshComponent;
class AGameCharacter;

UCLASS()
class ROCKPAPERSCISSORS_API AOrbitingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbitingActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	URotatingMovementComponent* GetRotatingMovementComp() const { return m_RotatingMovementComp; }

	void SetParentCharacter();

	TArray<UOrbitingStaticMeshComponent *> GetSpheresArray() const { return m_SpheresArray; }
	
	UFUNCTION()
	void DeleteSpheres(int32 index);

protected:
	virtual void BeginPlay() override;

	virtual void GenerateSpheres();

	UFUNCTION()
	void OnRep_SpheresArray();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URotatingMovementComponent* m_RotatingMovementComp;

	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_RootComp;

	UPROPERTY(ReplicatedUsing = OnRep_SpheresArray)
	TArray<UOrbitingStaticMeshComponent*> m_SpheresArray;

	UPROPERTY(EditDefaultsOnly, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	int32 m_NumberOfSpheres;

	UPROPERTY(EditDefaultsOnly, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	float m_RadiusOfRotation;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* m_StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	TArray<UMaterial*> m_MaterialsArray;

	AGameCharacter* m_GameCharacter;

	FRotator m_CurrentRoation;

	int32 m_IdCounter;
};
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
DECLARE_DELEGATE_OneParam(OrbitActorDelegate, int32)

UCLASS()
class ROCKPAPERSCISSORS_API AOrbitingActorCollisionState : public AOrbitingActor
{
	GENERATED_BODY()

public:
	AOrbitingActorCollisionState();

	virtual void BeginPlay() override;
	
	OrbitActorDelegate deleteSpheresDelegate;

protected:
	virtual void GenerateSpheres() override;

	UFUNCTION()
	void DestroySphres(int32 index);

private:

};
