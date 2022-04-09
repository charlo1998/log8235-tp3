// Fill out your copyright notice in the Description page of Project Settings.


#include "GroupManager.h"
#include "DrawDebugHelpers.h"
#include "SDTAIController.h"

// Sets default values
AGroupManager::AGroupManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGroupManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGroupManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//send their position in the pursuing group to the AIs
	for (int i = 0; i < m_pursuingCharacters.Num(); i++)
	{
		//Draw purple sphere on top of the AIs in the group
		FVector loc = m_pursuingCharacters[i]->GetActorLocation() + purpleMarkerLocation;
		DrawDebugSphere(GetWorld(), loc, 20.0f, 5, FColor::Purple);

		//Assign position to each AI to circle the player
		APawn* Pawn = Cast<APawn>(m_pursuingCharacters[i]);
		if (ASDTAIController* Controller = Cast<ASDTAIController>(Pawn->GetController()))
		{
			Controller->m_positionInGroup = i;
		}
	}
}

void AGroupManager::AddCharacterToGroup(AActor* character)
{
	m_pursuingCharacters.Add(character);
}

void AGroupManager::RemoveCharacterFromGroup(AActor* character)
{
	for (int i = 0; i < m_pursuingCharacters.Num(); i++)
	{
		if (character == m_pursuingCharacters[i])
		{
			m_pursuingCharacters.RemoveAt(i);
			break;
		}
	}
}