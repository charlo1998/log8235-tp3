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
double  ASDTAIController::detectionTime = 0.0;
double  ASDTAIController::collectibleTime = 0.0;

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

    //Find the groupManager at launch of the game
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
    ShowNavigationPath();
    PrintCPUTime();

}


void ASDTAIController::PrintCPUTime() {
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 500.f), FString::SanitizeFloat(collectibleTime * 1000000), GetPawn(), FColor::Green, 0.005f, false);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 300.f), FString::SanitizeFloat(detectionTime * 1000000), GetPawn(), FColor::Blue, 0.003f, false);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), FString::SanitizeFloat(chooseFleeTime * 1000000), GetPawn(), FColor::Purple, 0.001f, false);
    chooseFleeTime = 0.0;
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
    ShouldBeInChasingGroup();
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
    ShouldBeInChasingGroup();
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

        //retrieve position of agent in group and assign location in a circle. repeat if no places left.
        FVector offSet(0.f, 0.f, 0.f);
        int modulo = m_positionInGroup % 6;
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
    ShouldBeInChasingGroup();
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

    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        m_ReachedTarget = true;
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

}

//Find the groupManager at launch of the game
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

//Checks if the AI should be added or removed from the group
void ASDTAIController::ShouldBeInChasingGroup()
{
    //Checks if the state of the AI changed
    if (m_GroupManager && m_previousState != m_PlayerInteractionBehavior)
    {
        //Add if chasing
        if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Chase)
            dynamic_cast<AGroupManager*>(m_GroupManager)->AddCharacterToGroup(GetPawn());
        //remove otherwise
        else
            dynamic_cast<AGroupManager*>(m_GroupManager)->RemoveCharacterFromGroup(GetPawn());
        m_previousState = m_PlayerInteractionBehavior;
    }
}
