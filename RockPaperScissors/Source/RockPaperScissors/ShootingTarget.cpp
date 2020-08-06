// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingTarget.h"
#include "Components/PrimitiveComponent.h"
#include "GameCharacter.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AShootingTarget::AShootingTarget()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("SceneComp");
	RootComponent->SetAbsolute(false, true, false);
	RootComponent->SetIsReplicated(false);

	m_DistanceBetweenPoints = 100.f;


	bReplicates = false;
	SetReplicateMovement(false);

	m_CollisionTargetComp = nullptr;
	m_NewDestination = FVector(0.f, 0.f, 0.f);
	m_InterpSpeed = 50;
	m_bNewPositionSelected = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(AShootingTarget, m_CollisionTargetComp)
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::BeginPlay()
{
	Super::BeginPlay();
	SetCharacter();

}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::GenerateComponents()
{
	for (int32 z = -1; z <= 1; z++)
		for (int32 y = -1; y <= 1; y++)
			for (int32 x = -1; x <= 1; x++)
			{
				FVector location = FVector(x, y, z);
				FString name = FString("Point");
				name.Append(location.ToString());
				USceneComponent* comp = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), FName(*name));
				comp->SetRelativeLocation(location*m_DistanceBetweenPoints);
				m_CompArray.Push(comp);
				comp->SetupAttachment(RootComponent);
				comp->RegisterComponent();
					
				comp->SetIsReplicated(false);
			}

	m_CollisionTargetComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("Target"));
	if (m_CollisionTargetComp)
	{
		m_CollisionTargetComp->SetupAttachment(RootComponent);
		m_CollisionTargetComp->SetStaticMesh(m_Mesh);
		m_CollisionTargetComp->SetVisibility(true);
		m_CollisionTargetComp->SetRelativeLocation(GetActorLocation());
		m_CollisionTargetComp->SetCollisionProfileName("NoCollision");

		m_CollisionTargetComp->SetIsReplicated(false);
		m_CollisionTargetComp->RegisterComponent();
	}
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::SetCharacter()
{
	auto parent = GetAttachParentActor();
	if (parent)
		if (Cast<AGameCharacter>(parent))
			m_GameCharacter = Cast<AGameCharacter>(parent);
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SelectAndSetNewDestination(DeltaTime);
	for (auto& comp : m_CompArray)
	{
		DrawDebugSphere(GetWorld(), comp->GetComponentLocation(), 20.f, 10, FColor::Cyan, false, 0.f);
	}	
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::SelectAndSetNewDestination(float DeltaTime)
{
	if (m_bNewPositionSelected)
	{
		FVector NewLoc = FMath::VInterpConstantTo(m_CollisionTargetComp->GetRelativeLocation(), m_NewDestination, DeltaTime, m_InterpSpeed);
		DrawDebugSphere(GetWorld(), GetActorLocation() + m_NewDestination, 20.f, 16, FColor::Red, false, 0.f);
		m_CollisionTargetComp->SetRelativeLocation(NewLoc);

		if (m_CollisionTargetComp->GetRelativeLocation().Equals(m_NewDestination, 1.f))
		{
			m_bNewPositionSelected = false;
		}
	}
	else
	{
		AuthSelectNewDestination();
		m_bNewPositionSelected = true;
	}
	
		
	
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::AuthSelectNewDestination()
{
	if (HasAuthority() && GetNetMode() == ENetMode::NM_ListenServer) 
	{
		int32 randIndex = FMath::RandRange(0, m_CompArray.Num() - 1);
		NetMulticast_SelectDestination(randIndex);
	}
}
/******************************************************************************************************************************************************************************************************/
void AShootingTarget::NetMulticast_SelectDestination_Implementation(int randIndex)
{
	if (GetNetMode() == ENetMode::NM_Client)
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::FromInt(randIndex));
	m_NewDestination = m_CompArray[randIndex]->GetRelativeLocation();
}

