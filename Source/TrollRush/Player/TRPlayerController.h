// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "TRPlayerController.generated.h"

/**
 * 
 */
class UTRAbilitySystemComponent;
class UTRInputConfig;

UCLASS()
class TROLLRUSH_API ATRPlayerController : public APlayerController
{
	GENERATED_BODY()
	

private:
	UTRAbilitySystemComponent* GetASC();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UTRInputConfig> InputConfig;
};
