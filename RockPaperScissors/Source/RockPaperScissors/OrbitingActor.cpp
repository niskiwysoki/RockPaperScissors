// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitingActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/KismetTextLibrary.h"
#include "Internationalization/Text.h"

// Sets default values
AOrbitingActor::AOrbitingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	m_RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = m_RootComp;
	
	m_RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	m_RotatingMovementComp->SetUpdatedComponent(RootComponent);

	m_RadiusOfRotation = 500.f;
	m_NumberOfSpheres = 3;
	

	//for (int32 i = 0; i < m_NumberOfSpheres; i++)
	//{
	//	FString IntAsString = FString::FromInt(i);
	//	UStaticMeshComponent* staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(*IntAsString);
	//	staticMesh->SetupAttachment(RootComponent);
	//	m_SpheresArray.Push(staticMesh);
	//}


	bReplicates = true;
}

// Called when the game starts or when spawned
void AOrbitingActor::BeginPlay()
{
	Super::BeginPlay();
	
	for (int32 i = 0; i < m_NumberOfSpheres; i++)
	{
		FString IntAsString = FString::FromInt(i);
		UStaticMeshComponent* staticMeshComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(),*IntAsString);
		if (staticMeshComp)
		{
			staticMeshComp->SetupAttachment(RootComponent);
			double radians = (2 * PI * i / m_NumberOfSpheres);
			FVector NewLocation = FVector(cos(radians)*m_RadiusOfRotation, sin(radians)*m_RadiusOfRotation, 0.f);
			staticMeshComp->SetWorldLocation(GetActorLocation() + NewLocation);
			staticMeshComp->SetStaticMesh(m_StaticMesh);
			staticMeshComp->RegisterComponent();
			staticMeshComp->SetMaterial(0, m_MaterialsArray[i % 3]);

			FStaticMeshCompStruct meshStruct;
			meshStruct.staticMeshComp = staticMeshComp;
			meshStruct.type = SphereType(i % 3);
			m_SpheresArray.Push(meshStruct);
		}
	}



}

// Called every frame
void AOrbitingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

