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

	URotatingMovementComponent* GetRotatingMovementComp() const { return m_RotatingMovementComp; }

	TArray<UOrbitingStaticMeshComponent *> GetSpheresArray() const { return m_SpheresArray; }
	
	UFUNCTION()
	void DeleteSpheres(int32 id);

	void CreateSphere(int32 id, float angleInRadians, float radius);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URotatingMovementComponent* m_RotatingMovementComp;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_RootComp;

	TArray<UOrbitingStaticMeshComponent*> m_SpheresArray;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* m_StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Spheres", meta = (AllowPrivateAccess = "true"))
	TArray<UMaterial*> m_MaterialsArray;

};
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
DECLARE_DELEGATE_OneParam(VisbleOrbitActorDelegate, int32)
DECLARE_DELEGATE_ThreeParams(VisbleOrbitActorThreeParamDelegate, int32, float, float)		

USTRUCT()
struct FStoredGeneratesCallSturct
{
	GENERATED_BODY()

	UPROPERTY()
	int32 id = -1;
	UPROPERTY()
	float angleInRadians = -1;
	UPROPERTY()	
	float radius = -1;
};

UCLASS()
class ROCKPAPERSCISSORS_API AOrbitingActorCollisionState : public AActor
{
	GENERATED_BODY()

public:
	AOrbitingActorCollisionState();

	virtual void Tick(float DeltaTime) override;
	
	VisbleOrbitActorDelegate deleteVisibleSpheresDelegate;
	VisbleOrbitActorThreeParamDelegate createVisibleSphereDelegate;

	TArray<UOrbitingStaticMeshComponent *> GetSpheresArray() const { return m_SpheresArray; }

	void SetParentCharacter();

	URotatingMovementComponent* GetRotatingMovementComp() const { return m_RotatingMovementComp; }

	UFUNCTION()
	void DestroyVisibleSpheres(int32 id);

	void DestroySphere(int32 m_Id);

	void GenerateSpheres();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_SpheresArray();

	UPROPERTY(Replicated)
	URotatingMovementComponent* m_RotatingMovementComp;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
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

	int32 m_IdCounter;

	UPROPERTY(Replicated)
	TArray<FStoredGeneratesCallSturct> m_ArrayOfStoredGeneratesCallSturcts;

	TArray<int32> m_LocalVisSpheresIdsArray;

	bool m_bCreateVisSpheres;
};
