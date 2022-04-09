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

	void AddCharacterToGroup(AActor* character);
	void RemoveCharacterFromGroup(AActor* character);
	TArray<AActor*> GetPursuingCharacters() { return m_pursuingCharacters; };

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TArray<AActor*> m_pursuingCharacters;
	const FVector purpleMarkerLocation = FVector(0.0f, 0.0f, 75.0f);
};
