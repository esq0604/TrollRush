// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TRThrowWorldItem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ATRThrowWorldItem::ATRThrowWorldItem()
{
    
    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
    ProjectileMovementComp->bAutoActivate = false;
    ProjectileMovementComp->bRotationFollowsVelocity = true;
    ProjectileMovementComp->ProjectileGravityScale = 1.0f;


    bReplicates = true;
    SetReplicateMovement(true);
}

void ATRThrowWorldItem::Throw()
{
    if (!ProjectileMovementComp) return;
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    UE_LOG(LogTemp, Warning, TEXT("InitSpeed: %f"), ProjectileMovementComp->InitialSpeed);
    Server_Throw(OwnerActor->GetActorForwardVector() * ProjectileMovementComp->InitialSpeed);
}

void ATRThrowWorldItem::Server_Throw_Implementation(FVector_NetQuantize LaunchVel)
{
    if (!HasAuthority())
        return;
    UE_LOG(LogTemp, Warning, TEXT("[THROW][Server] LaunchVel=%.1f | Owner=%s"),
        LaunchVel.Size(), *GetNameSafe(GetOwner()));

    Multicast_ApplyIdle(GetOwner());
    Multicast_ApplyThrown(LaunchVel ,GetOwner());

    UE_LOG(LogTemp, Warning, TEXT("[THROWN][%s] Auth=%d | Vel=%.1f | PMActive=%d | PMUpdated=%d"),
        *GetName(), HasAuthority(), LaunchVel.Size(),
        ProjectileMovementComp ? ProjectileMovementComp->IsActive() : 0,
        ProjectileMovementComp && ProjectileMovementComp->UpdatedComponent ? 1 : 0);
    //충돌문제는 서버에서만 
    // 0.25초 후 Holder 무시 해제
    if (IsValid(GetOwner()) && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_ClearIgnoreOwner,
            FTimerDelegate::CreateUObject(this, &ThisClass::ClearIgnoreOwner, GetOwner()),
            0.25f, false
        );
    }

}

void ATRThrowWorldItem::Multicast_ApplyThrown_Implementation(FVector_NetQuantize LaunchVel, AActor* Holder)
{
    if (!IsValid(this)) return;

    UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent());
    if (!RootPrim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] ApplyThrown: RootPrim is null"), *GetName());
        return;
    }

    if (IsValid(Holder))
        RootPrim->IgnoreActorWhenMoving(Holder, true);

    // 0.25초 후 서로 무시 해제
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_ClearIgnoreOwner,
            FTimerDelegate::CreateWeakLambda(this, [this, Holder]()
        {
            if (UPrimitiveComponent* RP = Cast<UPrimitiveComponent>(GetRootComponent()))
                RP->IgnoreActorWhenMoving(Holder, false);

            if (IsValid(Holder))
                if (UPrimitiveComponent* HR = Cast<UPrimitiveComponent>(Holder->GetRootComponent()))
                    HR->IgnoreActorWhenMoving(this, false);
        }),
            0.25f, false
        );
    }

    // PMovement용 루트 설정: QueryOnly + Block, 물리 OFF
    RootPrim->SetSimulatePhysics(false);
    RootPrim->SetEnableGravity(false);
    RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RootPrim->SetCollisionResponseToAllChannels(ECR_Block);

    // ★ 소켓 분리 직후 겹침 방지: 살짝 전방으로 Nudge
    {
        const FVector Nudge = GetActorForwardVector() * 2.f;
        FHitResult Hit;
        RootPrim->MoveComponent(Nudge, GetActorRotation(), true, &Hit);
    }

    if (IsValid(StaticMesh))
    {
        StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        StaticMesh->SetCollisionResponseToAllChannels(ECR_Block);
    }

    if (ProjectileMovementComp)
    {
        if (ProjectileMovementComp->UpdatedComponent != RootPrim)
        {
            ProjectileMovementComp->SetUpdatedComponent(RootPrim);
        }

        UE_LOG(LogTemp, Warning, TEXT("[%s] THROWN | Auth=%d | Vel=%.1f | PMUpdated=%d"),
            *GetName(), HasAuthority(), LaunchVel.Size(),
            ProjectileMovementComp->UpdatedComponent ? 1 : 0);

        if (HasAuthority()) // 서버만 시뮬
        {
            ProjectileMovementComp->StopMovementImmediately();
            ProjectileMovementComp->Velocity = LaunchVel;
            ProjectileMovementComp->SetComponentTickEnabled(true);
            ProjectileMovementComp->Activate(true);
        }
        else
        {
            ProjectileMovementComp->Deactivate();
        }
    }
}

void ATRThrowWorldItem::BeginPlay()
{
    Super::BeginPlay();    
    
    if (ProjectileMovementComp && GetRootComponent())
    {
        ProjectileMovementComp->SetUpdatedComponent(GetRootComponent());
    }

    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        // Block 충돌 시 OnHit (권장)
        RootPrim->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);

    }
}

void ATRThrowWorldItem::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority()) return;
    if (!ProjectileMovementComp || !ProjectileMovementComp->IsActive()) return; 
    if (!IsValid(OtherActor) || OtherActor == this) return;

    // 소유자(투척자) 즉시 무시 (짧은 시간 ignore 타이머를 이미 쓰고 있어도 2중 방지)
    if (OtherActor == GetOwner()) return;

    UE_LOG(LogTemp, Warning, TEXT("ProjectileHit"));
    // 캐릭터/피격 가능 대상만 처리하고 싶다면 여기서 필터링 (예: APawn*)
    // if (!OtherActor->IsA(APawn::StaticClass())) return;

    //// 데미지 적용
    UGameplayStatics::ApplyDamage(
        OtherActor,
        Damage,
        nullptr,                 // InstigatorController (원하면 캐릭터 컨트롤러 넘겨도 됨)
        this,                    // DamageCauser
        UDamageType::StaticClass()
    );


    // 피격 시 관통하지 않고 멈추고 싶다면 PM 끄고 Idle로 전환/파괴
    if (ProjectileMovementComp)
        ProjectileMovementComp->Deactivate();

    // 선택 1) 즉시 제거
    // Destroy();

    // 선택 2) 바닥 상태로 전환해 땅에 떨어지게
    Multicast_ApplyIdle(GetOwner());
}

void ATRThrowWorldItem::ClearIgnoreOwner(AActor* Holder)
{
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        if (IsValid(Holder))
        {
            RootPrim->IgnoreActorWhenMoving(Holder, false);
        }
    }
}
