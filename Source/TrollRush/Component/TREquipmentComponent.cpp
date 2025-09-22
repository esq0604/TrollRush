// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TREquipmentComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTREquipmentComponent::UTREquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); 
	// ...
}

void UTREquipmentComponent::TryPickupItem(AActor* Item)
{
    if (!Item || HeldItem) return;

    Server_Pickup(Item);
}

void UTREquipmentComponent::TryDropItem()
{
    if (!HeldItem) return;
    Server_Drop();

    HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
    {
        RootComp->SetSimulatePhysics(true);
        RootComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    HeldItem = nullptr;
}

void UTREquipmentComponent::TryThrowItem()
{
    if (!HeldItem) return;
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (!OwnerChar)
        return;
    // 발사 속도 계산(카메라/컨트롤러 방향 권장)
    const FVector Forward = OwnerChar->GetControlRotation().Vector();
    Server_Throw(Forward);
}


// Called when the game starts
void UTREquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UTREquipmentComponent::AttachToHand(AActor* Item)
{
    //ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    //if (!OwnerChar) return;

    //HeldItem = Item;

    //if (USkeletalMeshComponent* MeshComp = OwnerChar->GetMesh())
    //{
    //    FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
    //    HeldItem->AttachToComponent(MeshComp, Rules, FName("hand_rSocket")); //Item이나 다른곳에서 소켓정보를 들고있도록 수정
    //}

    //// 물리 끄기
    //if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
    //{
    //}
    if (!IsValid(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachToHand: Item is invalid"));
        return false;
    }

    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachToHand: Owner is not ACharacter"));
        return false;
    }

    USkeletalMeshComponent* Mesh = Char->GetMesh();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachToHand: Character Mesh is null"));
        return false;
    }

    if (!Mesh->DoesSocketExist(HandSocketName))
    {
        UE_LOG(LogTemp, Error, TEXT("AttachToHand: Socket '%s' does not exist"),
            *HandSocketName.ToString());
        return false;
    }

    USceneComponent* ItemRoot = Item->GetRootComponent();
    if (!ItemRoot)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachToHand: Item has no RootComponent"));
        return false;
    }

    // 부착 가능한 상태로 정리
    if (auto* Prim = Cast<UPrimitiveComponent>(ItemRoot))
    {
        
        Prim->SetSimulatePhysics(false);
        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    const FAttachmentTransformRules Rules(EAttachmentRule::KeepWorld, true);
    Item->AttachToComponent(Mesh, Rules, HandSocketName);

    return true;
}

void UTREquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UTREquipmentComponent, HeldItem);
}

void UTREquipmentComponent::OnRep_HeldItem()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_HeldItem on %s, Held=%s"),
        *GetNameSafe(GetOwner()), *GetNameSafe(HeldItem));
    // 클라에서 복제로 바뀌었을 때 부착/해제 상태 맞춰주기
    if (HeldItem)
    {
        AttachToHand(HeldItem);
    }
    else
    {
        // 비어졌다면 아무 것도 안 함(던지기/드랍 시점에는 아이템 쪽에서 월드 상태로 바뀌었음)
    }
}

void UTREquipmentComponent::Server_Throw_Implementation(FVector_NetQuantize LaunchVelocity)
{
    if (!HeldItem) return;

    AActor* Item = HeldItem;
    
    HeldItem = nullptr;

    // 월드로 떼고
    Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // ProjectileMovement 우선 사용
    if (UProjectileMovementComponent* Proj = Item->FindComponentByClass<UProjectileMovementComponent>())
    {
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RootPrim->SetSimulatePhysics(false); // Projectile이 이동 담당
        }

        Proj->Velocity = LaunchVelocity;
        Proj->Activate(true);
    }
    else
    {
        // ProjectileMovement가 없다면 물리 임펄스로 처리
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RootPrim->SetSimulatePhysics(true);
            RootPrim->AddImpulse(LaunchVelocity * RootPrim->GetMass());
        }
    }


    // 아이템 Holder 해제 등 처리
}

void UTREquipmentComponent::Server_Drop_Implementation()
{

}

void UTREquipmentComponent::Server_Pickup_Implementation(AActor* Item)
{
    //AActor* Owner= GetOwner();
    //if (!Owner || !Item) return;

    //// 이미 누가 들고 있는지(Holder 등) 체크는 대상 액터 쪽에서 CanInteract로 처리했다고 가정
    //AttachToHand(HeldItem);

    //Item->SetReplicateMovement(false);   // 들고 있는 동안은 이동 복제 불필요(부착 복제로 충분)
    //Item->SetOwner(GetOwner());           
    //Item->ForceNetUpdate();

    //UE_LOG(LogTemp, Warning, TEXT("Server_Pickup: Owner=%s Role=%d"),
    //    *GetNameSafe(GetOwner()), (int32)GetOwner()->GetLocalRole()); 

    if (GetOwnerRole() != ROLE_Authority) 
        return;

    if (HeldItem || !IsValid(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_Pickup: invalid or already holding"));
        return;
    }

    // 클라가 보낸 포인터가 서버에서도 유효/복제중인지 확인
    if (!Item->GetIsReplicated())
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_Pickup: Item is not replicated"));
        // (옵션) 서버에서 직접 라인트레이스로 상호작용 대상 재탐색/검증
        return;
    }

    // 소유자/리레번시 설정(필요시)
    Item->SetOwner(GetOwner());
    Item->SetReplicates(true);

    // 먼저 상태 확정
    HeldItem = Item; // Replicated 변수여야 함 (이전 메시지 참고)

    // 연출/부착 시도 (방탄 유틸 사용)
    if (!AttachToHand(Item))
    {
        UE_LOG(LogTemp, Error, TEXT("Server_Pickup: AttachToHand failed"));
        HeldItem = nullptr; // 되돌리기
        return;
    }

    // (선택) 위치/회전 정리 or 소켓 기준 오프셋 적용
}


