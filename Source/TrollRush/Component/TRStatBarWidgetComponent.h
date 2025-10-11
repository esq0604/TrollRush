// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "TRStatBarWidgetComponent.generated.h"

/**
 * 
 */
class UTRStatComponent;
class UTRStatBarWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TROLLRUSH_API UTRStatBarWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UTRStatBarWidgetComponent();
protected:
	virtual void BeginPlay() override;

	TWeakObjectPtr<UTRStatComponent> StatComp;

private:
	UPROPERTY() // GC ¾ÈÀü
	UTRStatBarWidget* CachedWidget = nullptr;

	UFUNCTION()
	void HandleHpChanged(float CurrentHp, float MaxHp);

	UFUNCTION()
	void HandleCastingChanged(float InCastingDuration, float InCastingStartServerTime, bool bIsCasting);

	UFUNCTION()
	void HandleCastingSucceeded();
	void CacheWidget();

};
