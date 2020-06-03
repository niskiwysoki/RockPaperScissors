// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GamePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ROCKPAPERSCISSORS_API AGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AGamePlayerState(const FObjectInitializer &ObjectInitialize);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerInfo")
	int32 m_PlayerId;

private:


	

};
