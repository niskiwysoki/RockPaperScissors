// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitingActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

// Sets default values
AOrbitingActor::AOrbitingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	m_RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = m_RootComp;
	
	m_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	m_StaticMesh->SetupAttachment(RootComponent);

	m_RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	m_RotatingMovementComp->SetUpdatedComponent(RootComponent);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AOrbitingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrbitingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

