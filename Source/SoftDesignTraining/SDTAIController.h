// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
    GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        float m_DetectionCapsuleHalfLength = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        float m_DetectionCapsuleRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        UCurveFloat* JumpCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        float JumpSpeed = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
        bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
        bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
        bool Landing = false;

    UPROPERTY(EditAnywhere, Category = AI)
        UBehaviorTree* behaviorTree;
    UBehaviorTreeComponent* m_behaviorTreeComponent;
    UBlackboardComponent* m_blackboardComponent;

protected:

    enum PlayerInteractionBehavior
    {
        PlayerInteractionBehavior_Collect,
        PlayerInteractionBehavior_Chase,
        PlayerInteractionBehavior_Flee
    };

    void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
    void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime);
    //PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit);
    bool HasLoSOnHit(const FHitResult& hit);
    void PlayerInteractionLoSUpdate();
    void OnPlayerInteractionNoLosDone();
    void OnMoveToTarget();

public:
    virtual void BeginPlay() override;
    //behaviour tree tasks interface
    void MoveToPlayer();
    void MoveToRandomCollectible();
    void MoveToBestFleeLocation();
    void StartTree();

    //behaviour tree service interface
    bool playerPoweredUp();
    bool HasLos();

    virtual void Tick(float deltaTime) override;
    void PrintCPUTime();

    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void RotateTowards(const FVector& targetLocation);
    void SetActorLocation(const FVector& targetLocation);
    void AIStateInterrupted();
    int m_positionInGroup=0;
    float skippedDeltaTime;

private:
    virtual void GoToBestTarget(float deltaTime) override;
    virtual void UpdatePlayerInteraction(float deltaTime) override;
    virtual void ShowNavigationPath() override;
    void FindGroupManager();
    void ShouldBeInChasingGroup();
    static int aiCount;
    static int counter;
    static int lastUpdated;

    static double chooseFleeTime;
    //static double updateTime;
    static double detectionTime;
    static double collectibleTime;

protected:
    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;
    PlayerInteractionBehavior m_PlayerInteractionBehavior;
    AActor* m_GroupManager;
    PlayerInteractionBehavior m_previousState;
};