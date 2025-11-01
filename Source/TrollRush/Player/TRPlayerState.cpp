// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/TRPlayerState.h"
#include "AbilitySystem/TRAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/TRAttributeSet.h"

ATRPlayerState::ATRPlayerState()
{
    ASC = CreateDefaultSubobject<UTRAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<UTRAttributeSet>(TEXT("AttributeSet"));

    ASC->AddAttributeSetSubobject(AttributeSet.Get());
}
