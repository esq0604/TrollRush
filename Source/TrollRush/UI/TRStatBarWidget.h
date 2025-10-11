// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TRStatBarWidget.generated.h"

/**
 * 
 */
class UProgressBar;

UCLASS()
class TROLLRUSH_API UTRStatBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; 
	
	//체력 프로그레스바를 업데이트하는 함수
	void UpdateHpBarPercent(float InPercent);
	//캐스팅바 업데이트에 대한 상태를 조정하는 함수
	void UpdateCastingBarState(bool bIsCasting, float InCastingDuration);

	void OnCastingSucceeded();
protected:


private:
	//캐스팅바를 업데이트하는 함수(보간을 사용하여 점차 차오르도록 업데이트합니다)
	void UpdateCastingBarPercent_Internal(float InDeltaTime);

	UPROPERTY(meta = (BindWidget), EditDefaultsOnly)
	UProgressBar* HpBar;

	UPROPERTY(meta = (BindWidget), EditDefaultsOnly)
	UProgressBar* CastingBar;

	float CastingDuration;
	bool IsCasting;
	float CastingStartServerTime;
	float CastingPercentDelta = 0.f;
};
