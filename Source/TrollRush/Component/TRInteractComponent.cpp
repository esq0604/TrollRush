// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/TRInteractComponent.h"
#include "TRInteractComponent.h"
#include "Interface/TRInteractable.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UTRInteractComponent::UTRInteractComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);                       

	// ...
}


// Called when the game starts
void UTRInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UTRInteractComponent::TrySphereInteract()
{
	AActor* Hit = nullptr;

    if (!ShpereScan(Hit) || !Hit) return;

    Server_Interact(Hit);
}

bool UTRInteractComponent::ShpereScan(AActor*& Out)
{
    Out = nullptr;

    const APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!PC) return false;

    FVector CamLoc; FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(Interact_Sphere), false, Pawn);

    const bool bAny = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Pawn->GetActorLocation(),
        FQuat::Identity,
        QueryChannel,
        FCollisionShape::MakeSphere(SphereInteractRadius),
        Params
    );

    if (!bAny)
    {
        if (bDrawDebug) DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), SphereInteractRadius, 16, FColor::Red, false, 1.5f);
        return false;
    }

    // 정면/가까움 우선 스코어로 최적 후보 선택
    const FVector Fwd = CamRot.Vector();
    float BestScore = -FLT_MAX;
    AActor* Best = nullptr;

    for (const auto& O : Overlaps)
    {
        AActor* A = O.GetActor();
        if (!A || !A->Implements<UTRInteractable>()) continue;

        const FVector To = (A->GetActorLocation() - CamLoc);
        const float   DirScore = FVector::DotProduct(Fwd, To.GetSafeNormal());            // 정면(1.0)일수록 높음
        const float   DistScore = 1.0f - FMath::Clamp(To.Size() / (SphereInteractRadius * 2.f), 0.f, 1.f);
        const float   Score = DirScore * 0.7f + DistScore * 0.3f;

        if (Score > BestScore) { BestScore = Score; Best = A; }
    }

    if (Best)
    {
        Out = Best;
        if (bDrawDebug) DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), SphereInteractRadius, 16, FColor::Green, false, 1.5f);
        return true;
    }

    if (bDrawDebug) DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), SphereInteractRadius, 16, FColor::Red, false, 1.5f);
    return false;
}


void UTRInteractComponent::Server_Interact_Implementation(AActor* Target)
{
    if (!Target || !Target->Implements<UTRInteractable>()) return;

    // 서버 재검증(치트/경쟁 방지)
    constexpr float MaxServerDistance = 500.f;
    if (FVector::Dist(Target->GetActorLocation(), Target->GetActorLocation()) > MaxServerDistance) return;

    if (ITRInteractable* Interactable = Cast<ITRInteractable>(Target))
    {
        if (Interactable->CanInteract())
        {
            Interactable->Interact(GetOwner());
        }
    }
}



