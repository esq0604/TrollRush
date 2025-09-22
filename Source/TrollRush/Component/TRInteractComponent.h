// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRInteractComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROLLRUSH_API UTRInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTRInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void TrySphereInteract();

private:
	bool ShpereScan(AActor*& Out);

	// 서버 RPC (확정 실행)
	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* Target);

private:
	UPROPERTY(EditDefaultsOnly)
	float SphereInteractRadius;

	UPROPERTY(EditDefaultsOnly)
	bool bDrawDebug = false;

	/** 어떤 채널로 검사할지 (기본 Visibility) */
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_Visibility;
};
