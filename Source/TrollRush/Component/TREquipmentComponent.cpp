// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TREquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Actor/TRInteractableActor.h"
#include "Actor/TRThrowWorldItem.h"
#include "Interface/TRThrowable.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTREquipmentComponent::UTREquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); 
}

void UTREquipmentComponent::Server_Throw_Implementation()
{
    if (!HeldItem) return;

    // 1) 먼저 부착 해제 (부착 관계는 복제되어 클라에도 반영)
    DetachFromSocket(HeldItem);

    

    if (ATRThrowWorldItem* W = Cast<ATRThrowWorldItem>(HeldItem))
    {
        W->Server_Throw(GetOwner()->GetActorForwardVector() * 2000); // Holder는 아이템 내부에서 GetOwner() 사용 중이면 생략 가능
    }
    else if (ITRThrowable* IThrowable = Cast<ITRThrowable>(HeldItem))
    {
        IThrowable->Throw(); // 인터페이스 경로 (내부에서 서버우회 구현해두면 됨)
    }

    // 4) 참조 정리
    // HeldItem->SetOwner(nullptr); // 필요 시 소유권 해제
    HeldItem = nullptr;
    LastAttachedItem = nullptr;
}

// Called when the game starts
void UTREquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UTREquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

bool UTREquipmentComponent::AttachToSocket(AActor* Item)
{
    if (!Item) return false;
 
    USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

    const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
    Item->AttachToComponent(Mesh, Rules, AttachSocketName);
    return true;
}

void UTREquipmentComponent::DetachFromSocket(AActor* Item)
{
    if (!Item) return;
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void UTREquipmentComponent::Server_Pickup_Implementation(AActor* Item)
{
    if (!Item || HeldItem) return;

    // 1) 아이템이 모든 클라에서 물리/콜리전 OFF
    if (ATRInteractableActor* IA = Cast<ATRInteractableActor>(Item))
        IA->Multicast_ApplyHeld(GetOwner());

    // 2) 서버에서만 Attach (→ 자동 복제)
    if (!AttachToSocket(Item))
        return;

    Item->SetOwner(GetOwner()); // 선택
    HeldItem = Item;
}
