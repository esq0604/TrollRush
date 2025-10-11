// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/TRInteractableActor.h"
#include "Interface/TRThrowable.h"
#include "TRThrowWorldItem.generated.h"

/**
 * 
 */
class UProjectileMovementComponent;

UCLASS()
class TROLLRUSH_API ATRThrowWorldItem : public ATRInteractableActor, public ITRThrowable
{
	GENERATED_BODY()
	
public:
	ATRThrowWorldItem();

	virtual void Throw() override;

	UFUNCTION(Server, Reliable) void Server_Throw(FVector_NetQuantize LaunchVel);

	// 모든 클라에서 동일하게 Thrown 전환(시각/물리 상태)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyThrown(FVector_NetQuantize LaunchVel, AActor* Holder);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovementComp;

	// 던진 직후 소유자와 잠시 충돌 무시를 풀어주기 위한 타이머
	FTimerHandle TimerHandle_ClearIgnoreOwner;

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse,const FHitResult& Hit);

	UFUNCTION()
	void ClearIgnoreOwner(AActor* Holder);

	UPROPERTY(EditDefaultsOnly)
	float Damage;
};
