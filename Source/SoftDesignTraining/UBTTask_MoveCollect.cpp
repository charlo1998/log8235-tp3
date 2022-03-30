// Fill out your copyright notice in the Description page of Project Settings.


#include "UBTTask_MoveCollect.h"

#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UUBTTask_MoveCollect::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->MoveToRandomCollectible();
    }
    return EBTNodeResult::Succeeded;

}