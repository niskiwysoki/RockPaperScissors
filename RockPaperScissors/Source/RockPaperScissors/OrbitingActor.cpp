// Fill out your copyright notice in the Description page of Project Settings.

#include "OrbitingActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/KismetTextLibrary.h"
#include "Internationalization/Text.h"
#include "OrbitingStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GamePC.h"
#include "GameFramework/Pawn.h"
#include "GameCharacter.h"

// Sets default values
AOrbitingActor::AOrbitingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = m_RootComp;
	RootComponent->SetIsReplicated(false);
	
	m_RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	m_RotatingMovementComp->SetUpdatedComponent(RootComponent);
	m_RotatingMovementComp->SetIsReplicated(false);
	m_RotatingMovementComp->RotationRate.SetComponentForAxis(EAxis::Z, 80.f);

	m_RadiusOfRotation = 500.f;
	m_NumberOfSpheres = 3;
	m_IdCounter = 0;
	m_CurrentRoation = FRotator::ZeroRotator;

	m_GameCharacter = nullptr;

	//for (int32 i = 0; i < m_NumberOfSpheres; i++)
	//{
	//	FString IntAsString = FString::FromInt(i);
	//	UStaticMeshComponent* staticMesh = CreateDefaultSubobject<UStaticMeshComponent>(*IntAsString);
	//	staticMesh->SetupAttachment(RootComponent);
	//	m_SpheresArray.Push(staticMesh);
	//}
	
	bReplicates = false;
	SetReplicateMovement(false);
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::BeginPlay()
{
	Super::BeginPlay();
	SetParentCharacter();
	GenerateSpheres();

}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::GenerateSpheres() 
{
	if (m_GameCharacter)
		m_RotatingMovementComp->RotationRate.SetComponentForAxis(EAxis::Z, m_GameCharacter->GetBaseRotationRate());

	for (int32 i = 0; i < m_NumberOfSpheres; i++)
	{
		FString IntAsString = TEXT("Sphere") + FString::FromInt(i);
		UOrbitingStaticMeshComponent* sphereMeshComp = NewObject<UOrbitingStaticMeshComponent>(this, UOrbitingStaticMeshComponent::StaticClass(), *IntAsString);
		if (sphereMeshComp)
		{
			sphereMeshComp->SetupAttachment(RootComponent);
			double radians = (2 * PI * i / m_NumberOfSpheres);
			FVector NewLocation = FVector(cos(radians)*m_RadiusOfRotation, sin(radians)*m_RadiusOfRotation, 0.f);
			sphereMeshComp->SetVisibility(true);
			sphereMeshComp->SetType(ESphereType(i % 3));
			sphereMeshComp->SetWorldLocation(GetActorLocation() + NewLocation);
			sphereMeshComp->SetStaticMesh(m_StaticMesh);
			sphereMeshComp->SetMaterial(0, m_MaterialsArray[int32(sphereMeshComp->GetType()) % 3]);
			m_SpheresArray.Push(sphereMeshComp);
			sphereMeshComp->SetId(m_IdCounter);
			sphereMeshComp->RegisterComponent();
			m_IdCounter++;
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::OnRep_SpheresArray()
{
	for (auto& mesh : m_SpheresArray)
	{
		if (mesh)
		{
			mesh->SetMaterial(0, m_MaterialsArray[int32(mesh->GetType()) % 3]);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("mesh is nullptr"));
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOrbitingActor, m_SpheresArray);
}
/******************************************************************************************************************************************************************************************************/
// Called every frame
void AOrbitingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	m_CurrentRoation = GetActorRotation();
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::SetParentCharacter()
{
	auto parent = GetAttachParentActor();
	if (parent)
	{
		if (Cast<AGameCharacter>(parent))
			m_GameCharacter = Cast<AGameCharacter>(parent);
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Failed to cast to GameCharacter"));
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Orbiting actor has not parent"));
}

void AOrbitingActor::DeleteSpheres(int32 index)
{
	for (UOrbitingStaticMeshComponent* Comp : m_SpheresArray)
	{
		// Find the component with given ID
		if (Comp->GetId() == index)
		{
			// Destroy and remove it from the array
			Comp->DestroyComponent();
			m_SpheresArray.Remove(Comp);
			break;
		}
	}
}
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
AOrbitingActorCollisionState::AOrbitingActorCollisionState()
{
	m_RootComp->SetIsReplicated(false);
	RootComponent = m_RootComp;

	bReplicates = true;
	m_RotatingMovementComp->SetIsReplicated(false);
	SetReplicateMovement(true);
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::GenerateSpheres()
{
	if (HasAuthority())
	{
		if (m_GameCharacter)
			m_RotatingMovementComp->RotationRate.SetComponentForAxis(EAxis::Z, m_GameCharacter->GetBaseRotationRate());
		for (int32 i = 0; i < m_NumberOfSpheres; i++)
		{
			FString IntAsString = TEXT("Sphere") + FString::FromInt(i);
			UOrbitingStaticMeshComponent* sphereMeshComp = NewObject<UOrbitingStaticMeshComponent>(this, UOrbitingStaticMeshComponent::StaticClass(), *IntAsString);
			if (sphereMeshComp)
			{
				sphereMeshComp->SetupAttachment(RootComponent);
				double radians = (2 * PI * i / m_NumberOfSpheres);
				FVector NewLocation = FVector(cos(radians)*m_RadiusOfRotation, sin(radians)*m_RadiusOfRotation, 0.f);
				sphereMeshComp->SetType(ESphereType(i % 3));
				sphereMeshComp->SetVisibility(false);
				sphereMeshComp->SetGenerateOverlapEvents(true);
				sphereMeshComp->SetCollisionProfileName("OverlapAll");
				sphereMeshComp->SetWorldLocation(GetActorLocation() + NewLocation);
				sphereMeshComp->OnComponentBeginOverlap.AddUniqueDynamic(sphereMeshComp, &UOrbitingStaticMeshComponent::OnOrbitingSphereOverlap);
				sphereMeshComp->SetStaticMesh(m_StaticMesh);
				sphereMeshComp->SetMaterial(0, m_MaterialsArray[int32(sphereMeshComp->GetType()) % 3]);
				sphereMeshComp->DestroySphresDelegate.BindUObject(this, &AOrbitingActorCollisionState::DestroySphres);
				sphereMeshComp->SetId(m_IdCounter);
				m_SpheresArray.Push(sphereMeshComp);
				sphereMeshComp->RegisterComponent();
				m_IdCounter++;
			}
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::DestroySphres(int32 index)
{
	deleteSpheresDelegate.Execute(index);
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::BeginPlay()
{
	Super::BeginPlay();

	for (UActorComponent* Comp : GetComponents())
	{
		if (UOrbitingStaticMeshComponent* MeshComp = Cast<UOrbitingStaticMeshComponent>(Comp))
		{
			MeshComp->DestroySphresDelegate.BindUObject(this, &AOrbitingActorCollisionState::DestroySphres);
		}
	}
}
