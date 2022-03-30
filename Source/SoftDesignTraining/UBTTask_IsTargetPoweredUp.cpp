// Fill out your copyright notice in the Description page of Project Settings.


#include "UBTTask_IsTargetPoweredUp.h"

#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UUBTTask_IsTargetPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>("TargetPoweredUp"))
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}