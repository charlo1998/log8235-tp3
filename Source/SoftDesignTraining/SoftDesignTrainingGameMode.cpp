// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingGameMode.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingPlayerController.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTAIController.h"
#include <chrono>



ASoftDesignTrainingGameMode::ASoftDesignTrainingGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASoftDesignTrainingPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprint/BP_SDTMainCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ASoftDesignTrainingGameMode::StartPlay()
{
    Super::StartPlay();
	UWorld* world = GetWorld();
	world->Exec(world, TEXT("stat fps"));

	PrimaryActorTick.bCanEverTick = true;


	TArray<AActor*> actors = world->GetCurrentLevel()->Actors;
	for (AActor* actor : actors) {
		if (actor != NULL) {
			APawn* Pawn = Cast<APawn>(actor);
			if (actor->GetFName().ToString().Find("BP_SDTAICharacter") != std::string::npos) {
				AIactors.push(Cast<ASDTAIController>(Pawn->GetController()));
			}
		}
	}

}


void ASoftDesignTrainingGameMode::Tick(float DeltaSeconds) {

	elapsedTime = 0;
	std::queue<ASDTAIController*> tempQueue;
	while (!AIactors.empty() && elapsedTime< timeBudget)
	{
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		ASDTAIController* controller = AIactors.front();
		AIactors.pop();

		//GEngine->AddOnScreenDebugMessage(i, 50, FColor::Cyan, controller->GetFName().ToString());
		//i++;

		controller->StartTree();
		tempQueue.push(controller);

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		elapsedTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	}
	UE_LOG(LogTemp, Warning, TEXT("%d"),tempQueue.size());
	while (!tempQueue.empty()) {
		ASDTAIController* controller = tempQueue.front();
		tempQueue.pop();
		AIactors.push(controller);
	}
}