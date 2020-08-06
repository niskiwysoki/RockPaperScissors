// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingTarget.generated.h"

class ATargetPoint;
class AGameCharacter;
class UStaticMeshComponent;

UCLASS()
class ROCKPAPERSCISSORS_API AShootingTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShootingTarget();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SelectAndSetNewDestination(float DeltaTime);

	void GenerateComponents();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetCharacter();

private:
	void AuthSelectNewDestination();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SelectDestination(int randInex);         

	TArray<USceneComponent*> m_CompArray;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_DistanceBetweenPoints;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_InterpSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* m_Mesh;

	//UPROPERTY(Replicated)
	UStaticMeshComponent* m_CollisionTargetComp;

	AGameCharacter* m_GameCharacter;
	bool m_bNewPositionSelected;

	FVector m_NewDestination;

	

};
