// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TRInteractableActor.h"
#include "TRInteractableActor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Component/TREquipmentComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ATRInteractableActor::ATRInteractableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
    SetReplicateMovement(true);
	bNetUseOwnerRelevancy = true;   // 소유자의 relevancy를 따르게

    // cpp (생성자)
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Block);
    RootComponent = SphereCollision;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
    StaticMesh->SetupAttachment(SphereCollision);
}

// Called when the game starts or when spawned
void ATRInteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATRInteractableActor::Interact(AActor* Interactor)
{
    if (!HasAuthority()) return; // InteractComponent가 서버 RPC로 호출하므로 여기서 서버 보장됨.

    if (!Interactor) return;
    if (UTREquipmentComponent* Equip = Interactor->FindComponentByClass<UTREquipmentComponent>())
    {
        Equip->Server_Pickup(this);
    }
}

bool ATRInteractableActor::CanInteract()
{
	return true;
}

void ATRInteractableActor::Multicast_ApplyIdle_Implementation(AActor* Holder)
{
    if (!IsValid(this)) return;

    UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent());
    if (!RootPrim) return;

    if (IsValid(Holder))
        RootPrim->IgnoreActorWhenMoving(Holder, false);

    if (UProjectileMovementComponent* PM = FindComponentByClass<UProjectileMovementComponent>())
    {
        PM->StopMovementImmediately();
        PM->Deactivate();
    }

    RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RootPrim->SetCollisionResponseToAllChannels(ECR_Block);

    if (HasAuthority())
    {
        RootPrim->SetSimulatePhysics(true);
        RootPrim->SetEnableGravity(true);
        RootPrim->WakeAllRigidBodies();
    }
    else
    {
        RootPrim->SetSimulatePhysics(false);
        RootPrim->SetEnableGravity(false);
    }

    if (IsValid(StaticMesh))
    {
        StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StaticMesh->SetCollisionResponseToAllChannels(ECR_Block);
    }
}

void ATRInteractableActor::Multicast_ApplyHeld_Implementation(AActor* Holder)
{
   
    // 액터/컴포넌트 유효성 방어
    if (!IsValid(this)) return;

    UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent());
    if (!RootPrim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyHeld: RootPrim is null"), *GetName());
        return;
    }

    // 물리/충돌 비활성화
    UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyHeld: RootPrim is vaild"), *GetName());

    RootPrim->SetSimulatePhysics(false);
    RootPrim->SetEnableGravity(false);
    RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RootPrim->SetCollisionResponseToAllChannels(ECR_Ignore);

    // 들고 있는 캐릭터와의 충돌 무시 (Holder가 해당 클라에 비가시성이면 nullptr일 수 있음)
    if (IsValid(Holder))
    {
        RootPrim->IgnoreActorWhenMoving(Holder, true);
    }

    // (선택) StaticMesh가 따로 충돌을 갖는다면 방어 후 처리
    if (IsValid(StaticMesh))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyHeld: StaticMesh is vaild"), *GetName());

        StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    UE_LOG(LogTemp, Verbose, TEXT("[%s] Multicast_ApplyHeld (Authority=%d)"),
        *GetName(), HasAuthority());
    
}
