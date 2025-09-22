// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/TRInteractable.h"
#include "TRInteractableActor.generated.h"

UCLASS()
class TROLLRUSH_API ATRInteractableActor : public AActor, public ITRInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATRInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	virtual void Interact(AActor* Interactor) override;

	virtual bool CanInteract() override;

};
