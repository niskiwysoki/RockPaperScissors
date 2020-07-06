// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGameMode.h"
#include "GamePC.h"
#include "GamePlayerState.h"

AGameGameMode::AGameGameMode(const FObjectInitializer &ObjectInitialize) : AGameMode(ObjectInitialize)
{
	
}

void AGameGameMode::PostLogin(APlayerController * newPlayer)
{
	Super::PostLogin(newPlayer);
	SetIdsToPlayers(newPlayer);

}

void AGameGameMode::SetIdsToPlayers(APlayerController * newPlayer)
{
	if (s_PlayerIdCounter >= 4)
		s_PlayerIdCounter = 0;
	if (newPlayer)
	{
		AGamePC* playerControlled = Cast<AGamePC>(newPlayer);
		if (playerControlled)
			if (AGamePlayerState* playerState = Cast<AGamePlayerState>(playerControlled->PlayerState))
			{
				playerState->SetPlayerId(s_PlayerIdCounter);
				s_PlayerIdCounter++;
			}
	}
}

int32 AGameGameMode::s_PlayerIdCounter = 0;;
