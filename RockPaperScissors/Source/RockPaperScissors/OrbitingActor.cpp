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

	m_RotatingMovementComp->Deactivate();
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::OnRep_SpheresArray()
{
	for (auto& mesh : m_SpheresArray)
	{
		if (mesh)
			mesh->SetMaterial(0, m_MaterialsArray[int32(mesh->GetType()) % 3]);
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("mesh is nullptr"));
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
void AOrbitingActor::DeleteSpheres(int32 id)
{
	for (UOrbitingStaticMeshComponent* Comp : m_SpheresArray)
	{
		// Find the component with given ID
		if (Comp->GetId() == id)
		{
			// Destroy and remove it from the array
			Comp->DestroyComponent();
			m_SpheresArray.Remove(Comp);
			break;
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActor::CreateSphere(int32 id, float angleInRadians, float radius)
{
	FString IntAsString = TEXT("Sphere") + FString::FromInt(id);
	UOrbitingStaticMeshComponent* sphereMeshComp = NewObject<UOrbitingStaticMeshComponent>(this, UOrbitingStaticMeshComponent::StaticClass(), *IntAsString);
	if (sphereMeshComp)
	{
		angleInRadians += FMath::DegreesToRadians(RootComponent->GetRelativeRotation().Yaw);
		sphereMeshComp->SetupAttachment(RootComponent);
		FVector NewLocation = FVector(FMath::Cos(angleInRadians)*radius, FMath::Sin(angleInRadians)*radius, 0.f);
		sphereMeshComp->SetVisibility(true);
		sphereMeshComp->SetType(ESphereType(id % 3));
		sphereMeshComp->SetRelativeLocation(NewLocation);
		sphereMeshComp->SetStaticMesh(m_StaticMesh);
		sphereMeshComp->SetMaterial(0, m_MaterialsArray[int32(sphereMeshComp->GetType()) % 3]);
		m_SpheresArray.Push(sphereMeshComp);
		sphereMeshComp->SetId(id);
		sphereMeshComp->RegisterComponent();
	}
}
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
/******************************************************************************************************************************************************************************************************/
AOrbitingActorCollisionState::AOrbitingActorCollisionState()
{
	m_RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	m_RootComp->SetIsReplicated(false);
	RootComponent = m_RootComp;
	

	m_RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	m_RotatingMovementComp->SetUpdatedComponent(RootComponent);
	m_RotatingMovementComp->SetIsReplicated(false);
	m_RotatingMovementComp->RotationRate = FRotator::ZeroRotator;

	m_RadiusOfRotation = 500.f;
	m_NumberOfSpheres = 3;
	m_IdCounter = 0;

	m_GameCharacter = nullptr;

	bReplicates = true;
	SetReplicateMovement(true);

	m_bCreateVisSpheres = false;
	PrimaryActorTick.bCanEverTick = true;
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOrbitingActorCollisionState, m_SpheresArray);
	DOREPLIFETIME(AOrbitingActorCollisionState, m_ArrayOfStoredGeneratesCallSturcts);
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::BeginPlay()
{
	Super::BeginPlay();
	SetParentCharacter();

	m_RotatingMovementComp->Activate();
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
				float radians = (2 * PI * i / m_NumberOfSpheres);
				FVector NewLocation = FVector(FMath::Cos(radians)*m_RadiusOfRotation, FMath::Sin(radians)*m_RadiusOfRotation, 0.f);
				sphereMeshComp->SetType(ESphereType(i % 3));
				sphereMeshComp->SetVisibility(false);
				sphereMeshComp->SetGenerateOverlapEvents(true);
				sphereMeshComp->SetCollisionProfileName("OverlapAll");
				sphereMeshComp->SetWorldLocation(GetActorLocation() + NewLocation);
				sphereMeshComp->OnComponentBeginOverlap.AddUniqueDynamic(sphereMeshComp, &UOrbitingStaticMeshComponent::OnOrbitingSphereOverlap);
				sphereMeshComp->SetStaticMesh(m_StaticMesh);
				sphereMeshComp->SetMaterial(0, m_MaterialsArray[int32(sphereMeshComp->GetType()) % 3]);
				sphereMeshComp->SetId(m_IdCounter);

				m_SpheresArray.Push(sphereMeshComp);
				m_ArrayOfStoredGeneratesCallSturcts.Push(FStoredGeneratesCallSturct{m_IdCounter, radians, m_RadiusOfRotation});
				m_bCreateVisSpheres = true;

				sphereMeshComp->RegisterComponent();
				m_IdCounter++;
			}
		}
	}

	for (UActorComponent* Comp : GetComponents())
	{
		if (UOrbitingStaticMeshComponent* MeshComp = Cast<UOrbitingStaticMeshComponent>(Comp))
		{
			MeshComp->destroyVisibleSpheresDelegate.BindUObject(this, &AOrbitingActorCollisionState::DestroyVisibleSpheres);
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::DestroyVisibleSpheres(int32 id)
{
	deleteVisibleSpheresDelegate.ExecuteIfBound(id);
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::DestroySphere(int32 id)
{
	for (UOrbitingStaticMeshComponent* Comp : m_SpheresArray)
	{
		// Find the component with given ID
		if (Comp->GetId() == id)
		{
			// Destroy and remove it from the array
			Comp->DestroyComponent();
			m_SpheresArray.Remove(Comp);
			break;
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::OnRep_SpheresArray()
{
	for (auto& mesh : m_SpheresArray)
	{
		if (mesh)
			mesh->SetMaterial(0, m_MaterialsArray[int32(mesh->GetType()) % 3]);
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("mesh is nullptr"));
	}
	m_bCreateVisSpheres = true;
}
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::SetParentCharacter()
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
/******************************************************************************************************************************************************************************************************/
void AOrbitingActorCollisionState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bCreateVisSpheres)
	{
		if (createVisibleSphereDelegate.IsBound())
		{
			if (m_ArrayOfStoredGeneratesCallSturcts.Num()>0)
			{
				for (const auto& storedGeneratesCallSturct : m_ArrayOfStoredGeneratesCallSturcts)
				{
					if (m_LocalVisSpheresIdsArray.Find(storedGeneratesCallSturct.id) == INDEX_NONE)
					{
						m_LocalVisSpheresIdsArray.Push(storedGeneratesCallSturct.id);
						
						createVisibleSphereDelegate.ExecuteIfBound(storedGeneratesCallSturct.id, storedGeneratesCallSturct.angleInRadians, storedGeneratesCallSturct.radius);
					}
				}
				if (!HasAuthority())
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("VisSpheres updated"));
			}
			m_bCreateVisSpheres = false;
		}
	}
	

}