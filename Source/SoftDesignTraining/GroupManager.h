// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroupManager.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API AGroupManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGroupManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int AddCharacterToGroup(AActor* character);
	void RemoveCharacterFromGroup(AActor* character);
	std::list<AActor*> GetPursuingCharacters() { return m_pursuingCharacters; };

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	std::list<AActor*> m_pursuingCharacters;
	void DrawDebug();
};
