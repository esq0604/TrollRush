// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TREquipmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROLLRUSH_API UTREquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTREquipmentComponent();

	bool AttachToSocket(AActor* Item);

	void DetachFromSocket(AActor* Item);

	UFUNCTION(Server, Reliable)
	void Server_Pickup(AActor* Item);

	// ★ 던지기 서버 RPC
	UFUNCTION(Server, Reliable)
	void Server_Throw();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	AActor* HeldItem;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName AttachSocketName;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> LastAttachedItem; // 로컬 캐시
};
