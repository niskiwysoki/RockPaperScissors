// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "OrbitingStaticMeshComponent.generated.h"

UENUM()
enum class ESphereType : uint8
{
	Rock,
	Paper,
	Scissors,
	MAX_NUMBER,
};

DECLARE_DELEGATE_OneParam(OrbitCollisionCompDelegate, int32)

UCLASS()
class ROCKPAPERSCISSORS_API UOrbitingStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UOrbitingStaticMeshComponent(const FObjectInitializer &ObjectInitializer);

	ESphereType GetType() const { return m_Type; }
	void SetType(ESphereType val) { m_Type = val; }

	UFUNCTION(Category = "Orbiting Actor")
	void OnOrbitingSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DestroySphere();

	OrbitCollisionCompDelegate DestroySphresDelegate;

	void SetId(int32 val) { m_Id = val; }
	int32 GetId() const { return m_Id; }

protected:
	UFUNCTION(NetMulticast, Reliable)
	//void NetMulticast_DestroyVisibleSpheres(AOrbitingActorCollisionState* collisionSpheresActor,int32 index);
	void NetMulticast_DestroyVisibleSpheres();

private:
	UPROPERTY(Replicated)
	ESphereType m_Type;

	UPROPERTY(Replicated)
	int32 m_Id;

};
