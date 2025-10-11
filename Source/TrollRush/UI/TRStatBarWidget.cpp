// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TRStatBarWidget.h"
#include "Components/ProgressBar.h"

void UTRStatBarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (CastingBar)
    {
        CastingBar->SetVisibility(ESlateVisibility::Collapsed);
        CastingBar->SetPercent(0.f);
    }
    if(HpBar)
    {
        HpBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void UTRStatBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (IsCasting)
    {
        UpdateCastingBarPercent_Internal(InDeltaTime);
    }
}

void UTRStatBarWidget::UpdateHpBarPercent(float InPercent)
{
    if (HpBar)
    {
        HpBar->SetPercent(FMath::Clamp(InPercent, 0.f, 1.f));
    }
}

void UTRStatBarWidget::UpdateCastingBarState(bool bIsCasting, float InCastingDuration)
{
    CastingDuration = InCastingDuration;
    IsCasting = bIsCasting;
    UE_LOG(LogTemp, Warning, TEXT("UpdateCastingBarState"));

    if (IsCasting)
    {

        CastingBar->SetVisibility(ESlateVisibility::Visible);
        CastingBar->SetPercent(0.f);
        CastingPercentDelta = 0.f;
    }
    else
    {
        CastingBar->SetVisibility(ESlateVisibility::Collapsed);
        CastingBar->SetPercent(0.f);
    }

}

void UTRStatBarWidget::OnCastingSucceeded()
{
    if (!CastingBar) return;

    IsCasting = false;
    CastingBar->SetVisibility(ESlateVisibility::Collapsed);
    CastingBar->SetPercent(0.f);
}

void UTRStatBarWidget::UpdateCastingBarPercent_Internal(float InDeltaTime)
{
    CastingPercentDelta = InDeltaTime + CastingPercentDelta;

    float LerpCastingPercentVal = FMath::Clamp(CastingPercentDelta / CastingDuration, 0.f, 1.f);

    CastingBar->SetPercent(LerpCastingPercentVal);
}
