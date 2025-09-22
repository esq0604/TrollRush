// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TRInteractableActor.h"
#include "TRInteractableActor.h"
#include "Net/UnrealNetwork.h"
#include "Component/TREquipmentComponent.h"
// Sets default values
ATRInteractableActor::ATRInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	SetReplicateMovement(true);
	bNetUseOwnerRelevancy = true;   // ?? 추가: 소유자의 relevancy를 따르게
}

// Called when the game starts or when spawned
void ATRInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
}



void ATRInteractableActor::Interact(AActor* Interactor)
{
	
	if (UTREquipmentComponent* EquipComp = Interactor->FindComponentByClass<UTREquipmentComponent>())
	{
		EquipComp->TryPickupItem(this);
	}
	
}

bool ATRInteractableActor::CanInteract()
{
	return true;
}



