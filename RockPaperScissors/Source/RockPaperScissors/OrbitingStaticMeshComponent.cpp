// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitingStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "OrbitingActor.h"
#include "GameCharacter.h"

UOrbitingStaticMeshComponent::UOrbitingStaticMeshComponent(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
	SetType(ESphereType::MAX_NUMBER);
	m_Id = -1;
	SetCollisionProfileName("NoCollision");
	
	SetIsReplicatedByDefault(true);
	SetGenerateOverlapEvents(false);
}
/******************************************************************************************************************************************************************************************************/
void UOrbitingStaticMeshComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOrbitingStaticMeshComponent, m_Type);
	DOREPLIFETIME(UOrbitingStaticMeshComponent, m_Id);
}
/******************************************************************************************************************************************************************************************************/
void UOrbitingStaticMeshComponent::OnOrbitingSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UOrbitingStaticMeshComponent* OtherSpehereComp = Cast<UOrbitingStaticMeshComponent>(OtherComp);
	if (OtherSpehereComp)
	{
		if (OtherSpehereComp->GetType() == GetType())
		{
			OtherSpehereComp->DestroySphere();
			DestroySphere();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Orbs Deystoyd"));
		}
	}
}
/******************************************************************************************************************************************************************************************************/
void UOrbitingStaticMeshComponent::DestroySphere()
{
	if (AOrbitingActorCollisionState* collsionOrbitingActor = Cast<AOrbitingActorCollisionState>(GetOwner()))
	{
		NetMulticast_DestroyVisibleSpheres();
		DestroyComponent();
		collsionOrbitingActor->DestroySphere(m_Id);
	}	
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Failed cast to collision orbit actor"));
}
/******************************************************************************************************************************************************************************************************/
void UOrbitingStaticMeshComponent::NetMulticast_DestroyVisibleSpheres_Implementation()
{
	destroyVisibleSpheresDelegate.ExecuteIfBound(m_Id);
}
