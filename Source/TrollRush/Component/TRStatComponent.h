// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRStatComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCastingStateChangeSignature, float, CastingDuration, float, CastStartServerTime, bool, IsCasting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangeSignature, float, CurrentHp, float, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHpZeroSignatrue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCastingSuccedSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROLLRUSH_API UTRStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTRStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitStats(float InCurrentHp, float InMaxHp);
	

	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(float InDamage);

	float GetCurrentHp() { return CurrentHp; }
	float GetMaxHp() { return MaxHp; }

	UFUNCTION(Server, Reliable)
	void Server_BeginCasting();

	UFUNCTION(Server, Reliable)
	void Server_CancleCasting();

	FOnHealthChangeSignature OnHealthChangeDelegate;
	FOnHpZeroSignatrue OnHpZeroDelegate;
	FOnCastingStateChangeSignature OnCastingStateChangeDelegate;
	FOnCastingSuccedSignature OnCastingSuccedDelegate;
private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_CurrentHp();
		
	UFUNCTION()
	void OnRep_MaxHp();

	UFUNCTION()
	void OnRep_IsCasting();
private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditDefaultsOnly)
	float CurrentHp =100.f;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_MaxHp)
	float MaxHp =100.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsCasting)
	bool IsCasting;

	UPROPERTY(EditDefaultsOnly,Replicated)
	float CastingDuration =3.f;

	UPROPERTY(Replicated)
	float CastingServerTime;

	FTimerHandle CastingTimerHandle;

};
