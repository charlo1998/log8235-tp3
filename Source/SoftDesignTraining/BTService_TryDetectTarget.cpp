// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryDetectTarget.h"
#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_TryDetectTarget::UBTService_TryDetectTarget()
{
    bCreateNodeInstance = true;
}

void UBTService_TryDetectTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (aiController->HasLos())
        {
            //write to bb that the player is seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(OwnerComp.GetBlackboardComponent()->GetKeyID("TargetIsSeen"), true);
            }
        else {
            //write to bb that the player is not seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(OwnerComp.GetBlackboardComponent()->GetKeyID("TargetIsSeen"), false);
        }

    }
}




