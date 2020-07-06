// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerState.h"
#include "Net/UnrealNetwork.h"

AGamePlayerState::AGamePlayerState(const FObjectInitializer &ObjectInitialize) : APlayerState(ObjectInitialize)
{
	m_PlayerId = 0;
}


void AGamePlayerState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGamePlayerState, m_PlayerId);
}