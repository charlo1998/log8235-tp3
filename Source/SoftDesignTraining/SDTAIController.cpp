// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "GroupManager.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"


int ASDTAIController::aiCount = 0;
int ASDTAIController::counter = 0;
int ASDTAIController::lastUpdated = 0;

double  ASDTAIController::chooseFleeTime = 0.0;
double  ASDTAIController::updateTime = 0.0;
double  ASDTAIController::detectionTime = 0.0;
double  ASDTAIController::collectibleTime = 0.0;

//double  ASDTAIController::elapsedTime = 0.0;

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
    m_previousState = m_PlayerInteractionBehavior;
    m_behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    m_blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ASDTAIController::BeginPlay()
{
    Super::BeginPlay();

    m_blackboardComponent->InitializeBlackboard(*behaviorTree->BlackboardAsset);
    //this line runs the agents for one tick

    StartTree();

    FindGroupManager();


    // Get the number of AI
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetPawn()->GetClass(), FoundActors);
    aiCount = FoundActors.Num();

    skippedDeltaTime = 0.0;
}

void ASDTAIController::StartTree() {
    m_behaviorTreeComponent->StartTree(*behaviorTree, EBTExecutionMode::SingleRun); //par d�fault en looped chang� pour singleRun, et il faut caller le behavior tree des agents selon le budget
}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    PrintCPUTime();

}

//void ASDTAIController::PrintCPUTime(bool resetValue)
//{
//    GEngine->AddOnScreenDebugMessage(20, 50, FColor::Cyan, "Elasped time  : " + FString::SanitizeFloat(elapsedTime));// / (double)aiCount));
//    GEngine->AddOnScreenDebugMessage(4, -1, FColor::Green, "CPU time for choosing fleeing position : " + FString::SanitizeFloat(chooseFleeTime * 1000000));// / (double)aiCount));
//    GEngine->AddOnScreenDebugMessage(3, -1, FColor::Purple, "CPU time for detection                 : " + FString::SanitizeFloat(detectionTime * 1000000));// / (double)aiCount));
//    GEngine->AddOnScreenDebugMessage(2, -1, FColor::Yellow, "CPU time for choosing the collectible  : " + FString::SanitizeFloat(collectibleTime * 1000000));// / (double)aiCount));
//    GEngine->AddOnScreenDebugMessage(1, -1, FColor::Red, "CPU time for update                    : " + FString::SanitizeFloat(updateTime * 1000000));// / (double)aiCount));
//
//    if (updateTime != 0.0) GEngine->AddOnScreenDebugMessage(5, 60, FColor::Red, "Last non null CPU time for update                    : " + FString::SanitizeFloat(updateTime * 1000000));
//    if (detectionTime != 0.0) GEngine->AddOnScreenDebugMessage(6, 60, FColor::Purple, "Last non null CPU time for detection                 : " + FString::SanitizeFloat(detectionTime * 1000000));
//    if (chooseFleeTime != 0.0) GEngine->AddOnScreenDebugMessage(7, 60, FColor::Green, "Last non null CPU time for choosing fleeing position : " + FString::SanitizeFloat(chooseFleeTime * 1000000));
//    if (collectibleTime != 0.0) GEngine->AddOnScreenDebugMessage(8, 60, FColor::Yellow, "Last non null CPU time for choosing the collectible  : " + FString::SanitizeFloat(collectibleTime * 1000000));
//
//    if (resetValue)
//    {
//        chooseFleeTime = 0.0;
//        updateTime = 0.0;
//        detectionTime = 0.0;
//        collectibleTime = 0.0;
//    }
//}

void ASDTAIController::PrintCPUTime() {
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 700.f), FString::SanitizeFloat(updateTime * 1000000), GetPawn(), FColor::Yellow, 0.01f, false);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 500.f), FString::SanitizeFloat(collectibleTime * 1000000), GetPawn(), FColor::Green, 0.005f, false);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 300.f), FString::SanitizeFloat(detectionTime * 1000000), GetPawn(), FColor::Blue, 0.003f, false);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), FString::SanitizeFloat(chooseFleeTime * 1000000), GetPawn(), FColor::Purple, 0.001f, false);
    chooseFleeTime = 0.0;
    updateTime = 0.0;
    detectionTime = 0.0;
    collectibleTime = 0.0;
}


void ASDTAIController::GoToBestTarget(float deltaTime)
{

    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Collect:

        MoveToRandomCollectible();

        break;

    case PlayerInteractionBehavior_Chase:

        MoveToPlayer();

        break;

    case PlayerInteractionBehavior_Flee:

        MoveToBestFleeLocation();

        break;
    }
}

void ASDTAIController::MoveToRandomCollectible()
{
    double startCollectible = FPlatformTime::Seconds();

    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
    if (!m_ReachedTarget) {
        double endCollectible = FPlatformTime::Seconds();
        collectibleTime += endCollectible - startCollectible;
        return;
    }

    float closestSqrCollectibleDistance = 18446744073709551610.f;
    ASDTCollectible* closestCollectible = nullptr;

    TArray<AActor*> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    while (foundCollectibles.Num() != 0)
    {
        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

        ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
        if (!collectibleActor) {
            double endCollectible = FPlatformTime::Seconds();
            collectibleTime += endCollectible - startCollectible;
            return;
        }

        if (!collectibleActor->IsOnCooldown())
        {
            MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, NULL, false);
            OnMoveToTarget();
            double endCollectible = FPlatformTime::Seconds();
            collectibleTime += endCollectible - startCollectible;
            return;
        }
        else
        {
            foundCollectibles.RemoveAt(index);
        }
    }
}

void ASDTAIController::MoveToPlayer()
{
    double startDetection = FPlatformTime::Seconds();

    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Chase;
    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter) {
        double endDetection = FPlatformTime::Seconds();
        detectionTime += endDetection - startDetection;
        return;
    }

    FVector selfPosition = GetPawn()->GetActorLocation();
    FVector playerPosition = playerCharacter->GetActorLocation();

    if (m_GroupManager && (selfPosition - playerPosition).Size() >= 150.f) //far from player
    {
        //instead of moving straigth into the player, assign a position around him as a function of the position of the AI in the pursuit group
        TArray<AActor*> group = dynamic_cast<AGroupManager*>(m_GroupManager)->GetPursuingCharacters();

        for (size_t i = 0; i < group.Num(); i++) //idea: instead of looping here each tick, use the group manager to loop each tick and write the position for each agent to its created member variable
        {
            if (GetPawn() == group[i]) //found my index
            {
                //retrieve position of agent in group and assign location in a circle. repeat if no places left.
                FVector offSet(0.f, 0.f, 0.f);
                int modulo = i % 6;
                switch (modulo)
                {
                case 0:
                    offSet = FVector(1.f, 0.f, 0.f);
                    break;
                case 1:
                    offSet = FVector(0.5f, 0.866f, 0.f);
                    break;
                case 2:
                    offSet = FVector(-0.5f, 0.866f, 0.f);
                    break;
                case 3:
                    offSet = FVector(-1.f, 0.f, 0.f);
                    break;
                case 4:
                    offSet = FVector(-0.5f, -0.866f, 0.f);
                    break;
                case 5:
                    offSet = FVector(0.5f, -0.866f, 0.f);
                    break;
                }
                MoveToLocation(playerPosition + offSet * 145.f, 0.5f, false, true, true, NULL, false);
                OnMoveToTarget();
                break;
            }

        }
    }
    else //close enough to player, go straight to him
    {
        MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
        OnMoveToTarget();
    }

    double endDetection = FPlatformTime::Seconds();
    detectionTime += endDetection - startDetection;
}

void ASDTAIController::PlayerInteractionLoSUpdate()
{

}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{

}

bool ASDTAIController::playerPoweredUp()
{
    return SDTUtils::IsPlayerPoweredUp(GetWorld());
}

bool ASDTAIController::HasLos()
{
    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    bool hasLosOnPlayer = false;

    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            return true;
        }
    }
    return false;
}

void ASDTAIController::MoveToBestFleeLocation()
{
    double startFleeChoice = FPlatformTime::Seconds();

    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Flee;
    float bestLocationScore = 0.f;
    ASDTFleeLocation* bestFleeLocation = nullptr;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter) {
        double endFleeChoice = FPlatformTime::Seconds();
        chooseFleeTime += endFleeChoice - startFleeChoice;
        return;
    }

    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
    {
        ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
        if (fleeLocation)
        {
            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

            FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToPlayer.Normalize();

            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToFleeLocation.Normalize();

            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

            if (locationScore > bestLocationScore)
            {
                bestLocationScore = locationScore;
                bestFleeLocation = fleeLocation;
            }

            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
        }
    }

    if (bestFleeLocation)
    {
        MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, NULL, false);
        OnMoveToTarget();
    }
    double endFleeChoice = FPlatformTime::Seconds();
    chooseFleeTime += endFleeChoice - startFleeChoice;
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    double startUpdate = FPlatformTime::Seconds();

    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;
    /*
    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    UpdatePlayerInteractionBehavior(detectionHit, deltaTime);
    */
    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        m_ReachedTarget = true;
    }

    if (m_GroupManager && m_previousState != m_PlayerInteractionBehavior)
    {
        if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Chase)
            dynamic_cast<AGroupManager*>(m_GroupManager)->AddCharacterToGroup(GetPawn());
        else
            dynamic_cast<AGroupManager*>(m_GroupManager)->RemoveCharacterFromGroup(GetPawn());
        m_previousState = m_PlayerInteractionBehavior;
    }

    FString debugString = "";

    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Chase:
        debugString = "Chase";
        break;
    case PlayerInteractionBehavior_Flee:
        debugString = "Flee";
        break;
    case PlayerInteractionBehavior_Collect:
        debugString = "Collect";
        break;
    }

    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, GetPawn(), FColor::Orange, 0.f, false);
    /*
    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
    */

    double endUpdate = FPlatformTime::Seconds();
    updateTime += endUpdate - startUpdate;
}

bool ASDTAIController::HasLoSOnHit(const FHitResult& hit)
{
    if (!hit.GetComponent())
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
    hitDirection.Normalize();

    FHitResult losHit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(hit.GetActor());

    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

    return losHit.GetActor() == nullptr;
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}
/*
ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
{

    if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Collect)
    {
        if (!hit.GetComponent())
            return PlayerInteractionBehavior_Collect;

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))
            return PlayerInteractionBehavior_Collect;

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else
    {
        PlayerInteractionLoSUpdate();

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }

}
*/
void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime)
{
    /*
    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit);


    if (currentBehavior != m_PlayerInteractionBehavior)
    {
        m_PlayerInteractionBehavior = currentBehavior;
        AIStateInterrupted();
    }
    */
}

void ASDTAIController::FindGroupManager()
{
    for (TActorIterator<AActor> actor(GetWorld()); actor; ++actor)
    {
        if (AGroupManager* groupManager = dynamic_cast<AGroupManager*>(*actor))
        {
            m_GroupManager = groupManager;
        }
    }
}
