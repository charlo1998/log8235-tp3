// Fill out your copyright notice in the Description page of Project Settings.


#include "UBTTask_IsTargetSeen.h"

#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UUBTTask_IsTargetSeen::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (aiController->HasLos())
            return EBTNodeResult::Succeeded;

    }

    return EBTNodeResult::Failed;

    //if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>("TargetIsSeen"))
    //{
    //    return EBTNodeResult::Succeeded;
    //}

    //return EBTNodeResult::Failed;
}
