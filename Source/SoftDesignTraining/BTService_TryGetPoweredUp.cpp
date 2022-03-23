// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryGetPoweredUp.h"
#include "SDTAIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_TryGetPoweredUp::UBTService_TryGetPoweredUp()
{
    bCreateNodeInstance = true;
}

void UBTService_TryGetPoweredUp::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (aiController->playerPoweredUp())
        {
            //write to bb that the player is seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(OwnerComp.GetBlackboardComponent()->GetKeyID("TargetPoweredUp"), true);
        }
        else {
            //write to bb that the player is not seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(OwnerComp.GetBlackboardComponent()->GetKeyID("TargetPoweredUp"), false);
        }

    }
}




