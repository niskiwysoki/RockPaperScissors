// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ROCKPAPERSCISSORS_API AGameGameMode : public AGameMode
{
	GENERATED_BODY()
	
	AGameGameMode(const FObjectInitializer &ObjectInitialize);

	virtual void PostLogin(APlayerController * NewPlayer) override;

	void SetIdsToPlayers(APlayerController * newPlayer);

private:
	static int32 s_PlayerIdCounter;
};
