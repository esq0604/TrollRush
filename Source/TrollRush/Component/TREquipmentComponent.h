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

	void TryPickupItem(AActor* Item);
	void TryDropItem();
	void TryThrowItem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool AttachToHand(AActor* Item);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_HeldItem();

	// 서버 권위 RPC
	UFUNCTION(Server, Reliable) void Server_Pickup(AActor* Item);
	UFUNCTION(Server, Reliable) void Server_Drop();
	UFUNCTION(Server, Reliable) void Server_Throw(FVector_NetQuantize LaunchVelocity);
private:
	UPROPERTY(Replicated)
	AActor* HeldItem;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	FName HandSocketName = TEXT("hand_r"); 
		
};
