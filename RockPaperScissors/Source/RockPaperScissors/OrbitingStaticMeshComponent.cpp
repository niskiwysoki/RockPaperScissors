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

void UOrbitingStaticMeshComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOrbitingStaticMeshComponent, m_Type);
	DOREPLIFETIME(UOrbitingStaticMeshComponent, m_Id);
}


void UOrbitingStaticMeshComponent::OnOrbitingSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UOrbitingStaticMeshComponent* OtherSpehereComp = Cast<UOrbitingStaticMeshComponent>(OtherComp);
	if (OtherSpehereComp)
	{
		if (OtherSpehereComp->GetType() == GetType())
		{
			/*DestroyOtherSpheres(OtherSpehereComp);
			DestroyOtherSpheres(this);*/

			OtherSpehereComp->DestroySphere();
			DestroySphere();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Orbs Deystoyd"));
		}
	}
}

void UOrbitingStaticMeshComponent::DestroySphere()
{
	if (AOrbitingActorCollisionState* collsionOrbitingActor = Cast<AOrbitingActorCollisionState>(GetOwner()))
		if (AGameCharacter* character = Cast<AGameCharacter>(collsionOrbitingActor->GetAttachParentActor()))
			if (AOrbitingActorCollisionState* collisionSpheresActor = Cast<AOrbitingActorCollisionState>(character->GetCollidingOrbitingSpheres()->GetChildActor()))
			{
				int32 index = collisionSpheresActor->GetSpheresArray().Find(this);
				//NetMulticast_DestroyVisibleSpheres(collisionSpheresActor, index);
				NetMulticast_DestroyVisibleSpheres();
				DestroyComponent();
				collisionSpheresActor->GetSpheresArray().RemoveAt(index);
			}
			else
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Failed cast to collisionSpheresActor"));
		else
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Failed cast to gameCharacter"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Failed cast to orbit actor"));
}

//void UOrbitingStaticMeshComponent::NetMulticast_DestroyVisibleSpheres_Implementation(AOrbitingActorCollisionState* collisionSpheresActor, int32 index)
//{
//	AGameCharacter* character = Cast<AGameCharacter>(collisionSpheresActor->GetAttachParentActor());
//	AOrbitingActor* visibleSpheresActor = Cast<AOrbitingActor>(character->GetVisibleOrbitingSpheres()->GetChildActor());
//	visibleSpheresActor->GetSpheresArray()[index]->DestroyComponent();
//	visibleSpheresActor->GetSpheresArray().RemoveAt(index);
//	DestroySphresDelegate.ExecuteIfBound(this);
//}

void UOrbitingStaticMeshComponent::NetMulticast_DestroyVisibleSpheres_Implementation()
{
	DestroySphresDelegate.ExecuteIfBound(m_Id);
}
