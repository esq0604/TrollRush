// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TRStatBarWidgetComponent.h"
#include "Component/TRStatComponent.h"
#include "UI/TRStatBarWidget.h"
UTRStatBarWidgetComponent::UTRStatBarWidgetComponent()
{

}

void UTRStatBarWidgetComponent::BeginPlay()
{
    Super::BeginPlay();

    CacheWidget();

    //위젯컴포넌트만 사용시 서버쪽에서 클라이언트들의 Widgetcomp가 안보여, ChildActorComp를 사용해, Actor지정 후 해당 Actor에서 WidgetComp를 사용한 방식
    AActor* SuperOwner = GetOwner()->GetAttachParentActor();

    if (SuperOwner)
    {
            UE_LOG(LogTemp, Warning, TEXT("SuperOwner : %s"), *SuperOwner->GetName());

            StatComp = SuperOwner->FindComponentByClass<UTRStatComponent>();
            if (StatComp.IsValid())
            {
                UE_LOG(LogTemp, Warning, TEXT("Parent StatComp Is Vaild"));
                // 델리게이트 바인딩
                StatComp->OnHealthChangeDelegate.AddDynamic(this, &ThisClass::HandleHpChanged);
                StatComp->OnCastingStateChangeDelegate.AddDynamic(this, &ThisClass::HandleCastingChanged);
                StatComp->OnCastingSuccedDelegate.AddDynamic(this, &ThisClass::HandleCastingSucceeded);
                // 현재값으로 초기 동기화
                HandleHpChanged(StatComp->GetCurrentHp(), StatComp->GetMaxHp());
            }
    }
 
}

void UTRStatBarWidgetComponent::HandleHpChanged(float CurrentHp, float MaxHp)
{
    if (!CachedWidget)
    {
        CacheWidget();
    }
    if (CachedWidget)
    {
        const float Percent = (MaxHp > 0.f) ? (CurrentHp / MaxHp) : 0.f;
        CachedWidget->UpdateHpBarPercent(Percent);

    }
}

void UTRStatBarWidgetComponent::HandleCastingChanged(float InCastingDuration, float InCastingStartServerTime, bool bIsCasting)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleCastingChange"));

    if (CachedWidget)
    {
        CachedWidget->UpdateCastingBarState(bIsCasting, InCastingDuration);
    }
}

void UTRStatBarWidgetComponent::HandleCastingSucceeded()
{
    if (CachedWidget)
    {
        CachedWidget->OnCastingSucceeded();
    }
}

void UTRStatBarWidgetComponent::CacheWidget()
{
    if (!CachedWidget)
    {
        if (UUserWidget* UW = GetUserWidgetObject())
        {
            CachedWidget = Cast<UTRStatBarWidget>(UW);
        }
    }
}
