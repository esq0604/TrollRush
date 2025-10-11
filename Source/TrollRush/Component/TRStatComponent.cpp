// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TRStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UTRStatComponent::UTRStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UTRStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OnHealthChangeDelegate.Broadcast(CurrentHp, MaxHp);
}


// Called every frame
void UTRStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTRStatComponent::InitStats(float InCurrentHp, float InMaxHp)
{
	if (GetOwnerRole() != ROLE_Authority) return; // 서버에서만

	MaxHp = FMath::Max(1.f, InMaxHp);
	CurrentHp = FMath::Clamp(InCurrentHp, 0.f, MaxHp);

	// 서버 로컬 즉시 UI 갱신(리슨 서버)
	OnHealthChangeDelegate.Broadcast(CurrentHp, MaxHp);
}

void UTRStatComponent::Server_ApplyDamage_Implementation(float InDamage)
{
	if (GetOwnerRole() != ROLE_Authority) return; 

	float PrevDamage = CurrentHp;

	const float ApplyDamage = FMath::Clamp(InDamage, 0.f, CurrentHp);

	CurrentHp = (PrevDamage - ApplyDamage);

	OnHealthChangeDelegate.Broadcast(CurrentHp, MaxHp);

	if (CurrentHp <= 0)
	{
		OnHpZeroDelegate.Broadcast();
	}
}

void UTRStatComponent::Server_CancleCasting_Implementation()
{
	IsCasting = false;
	if (CastingTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CastingTimerHandle);
		OnCastingStateChangeDelegate.Broadcast(CastingDuration, CastingServerTime, IsCasting);
	}
}

void UTRStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTRStatComponent, CurrentHp);
	DOREPLIFETIME(UTRStatComponent, MaxHp);
	DOREPLIFETIME(UTRStatComponent, IsCasting);

	DOREPLIFETIME(UTRStatComponent, CastingDuration);
	DOREPLIFETIME(UTRStatComponent, CastingServerTime);

}

void UTRStatComponent::Server_BeginCasting_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server_BeginCasting_Implementation"));
	IsCasting = true;
	CastingServerTime = UGameplayStatics::GetGameState(GetWorld())->GetServerWorldTimeSeconds();

	if (CastingTimerHandle.IsValid())
	{
		//기존타이머가 있다면 제거 
		GetWorld()->GetTimerManager().ClearTimer(CastingTimerHandle);
	}

	OnCastingStateChangeDelegate.Broadcast(CastingDuration, CastingServerTime, IsCasting);

	//5초 뒤 캐스팅 성공시 
	GetWorld()->GetTimerManager().SetTimer(CastingTimerHandle, [this]()
	{
		IsCasting = false;
		GetWorld()->GetTimerManager().ClearTimer(CastingTimerHandle);
		OnCastingStateChangeDelegate.Broadcast(CastingDuration, CastingServerTime, IsCasting); //캐스팅 종료 델리게이트

		OnCastingSuccedDelegate.Broadcast(); //캐스팅 성공 델리게이트 

	}, CastingDuration, false);


}

void UTRStatComponent::OnRep_CurrentHp()
{
	OnHealthChangeDelegate.Broadcast(CurrentHp, MaxHp);

}

void UTRStatComponent::OnRep_MaxHp()
{
	OnHealthChangeDelegate.Broadcast(CurrentHp, MaxHp);

}

void UTRStatComponent::OnRep_IsCasting()
{
	OnCastingStateChangeDelegate.Broadcast(CastingDuration, CastingServerTime, IsCasting);
}

