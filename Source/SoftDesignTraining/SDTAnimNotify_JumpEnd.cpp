// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpEnd.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

void USDTAnimNotify_JumpEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    //Notify that our NPC has landed
    Cast<ASDTAIController>(Cast<APawn>(MeshComp->GetOwner())->Controller)->AiJumpProgress = 0.0f;//come back to walk/run/idle animation
}
