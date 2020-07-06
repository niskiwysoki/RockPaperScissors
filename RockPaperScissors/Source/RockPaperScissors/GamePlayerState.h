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

	int32 GetPlayerId() const { return m_PlayerId; }
	void SetPlayerId(int32 val) { m_PlayerId = val; }

protected:

	UPROPERTY(Replicated)
	int32 m_PlayerId;

private:


	

};
