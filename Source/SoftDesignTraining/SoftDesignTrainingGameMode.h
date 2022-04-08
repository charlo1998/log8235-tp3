// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include <queue> 
#include "SDTAIController.h"
#include "SoftDesignTrainingGameMode.generated.h"

UCLASS(minimalapi)
class ASoftDesignTrainingGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASoftDesignTrainingGameMode();

	virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
	std::queue<ASDTAIController*> AIactors;
	const int timeBudget = 40; // in microseconds
	long long elapsedTime;
};



