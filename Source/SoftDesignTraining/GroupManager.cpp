// Fill out your copyright notice in the Description page of Project Settings.


#include "GroupManager.h"
#include "DrawDebugHelpers.h"

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
	DrawDebug();
}

void AGroupManager::AddCharacterToGroup(AActor* character)
{
	m_pursuingCharacters.push_back(character);
}

void AGroupManager::RemoveCharacterFromGroup(AActor* character)
{
	m_pursuingCharacters.remove(character);
}

void AGroupManager::DrawDebug()
{
	for (std::list<AActor*>::iterator iter = m_pursuingCharacters.begin(), end = this->m_pursuingCharacters.end();
		iter != end;
		++iter)
	{
		FVector loc = (*iter)->GetActorLocation() + FVector(0.0f, 0.0f, 75.0f);
		DrawDebugSphere(GetWorld(), loc, 25.0f, 100, FColor::Purple);
	}
}