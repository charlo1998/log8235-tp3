// Fill out your copyright notice in the Description page of Project Settings.


#include "UBTTask_MoveFlee.h"

#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UUBTTask_MoveFlee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->MoveToBestFleeLocation();
        //GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, "Fleeing");// / (double)aiCount));
        return EBTNodeResult::Succeeded;
    }
    return EBTNodeResult::Failed;

}