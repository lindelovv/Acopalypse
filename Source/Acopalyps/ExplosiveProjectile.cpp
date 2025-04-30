// Fill out your copyright notice in the Description page of Project Settings.
/**
 @author: Gin Lindelöw
*/

#include "ExplosiveProjectile.h"
#include "ExplosiveDamageType.h" // grayed by rider out but needed
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	//Projectile values overriden from the base Projectile class.
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
}

//Overriden OnHit class
void AExplosiveProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	HitTriggerEvent(Hit);
	TArray<FOverlapResult> Overlaps;
	if(HitActor != nullptr && ExplosionTrace(Overlaps) )
	{
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(),
			80.f,
			20.f,
			Hit.Location,
			ExplosionRadius / 2,
			ExplosionRadius,
			1.f,
			ExplosiveDamageType,
			{},
			this,
			GetWorld()->GetFirstPlayerController(),
			ECC_Visibility
			);
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(),
			1.f,
			1.f,
			Hit.Location,
			GlassSmashExplosionRadius / 2,
			GlassSmashExplosionRadius,
			1.f,
			ExplosiveDamageType,
			{},
			this,
			GetWorld()->GetFirstPlayerController(),
			ECC_Visibility
			);
		for( FOverlapResult Overlap : Overlaps )
		{
			
			if(Overlap.GetActor() != nullptr && Overlap.GetComponent() != nullptr && Overlap.GetComponent()->IsSimulatingPhysics() )
			{
				Overlap.GetComponent()->AddImpulseAtLocation(GetActorLocation() + GetVelocity().Size() * 20.f, GetActorLocation());
				ExplostionOverlapActorEvent(Overlap.GetActor());
			}
		}
	}
	DestructionDelegate.BindLambda([this]{ if( this->IsValidLowLevel() ) Destroy(); });
	SetActorEnableCollision(false);
	GetWorldTimerManager().SetTimer(DestructionTimer, DestructionDelegate, .1f, false);
}

bool AExplosiveProjectile::ExplosionTrace(TArray<FOverlapResult>& Overlaps)
{
	AController* OwnerController = GetWorld()->GetFirstPlayerController();
	if(OwnerController == nullptr)
	{
		return false;
	}
	
	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);
	FVector End = Location + Rotation.Vector() * 240;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerController);

	if( bDrawDebugSphere )
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 10, FColor::Red, true, 5);
	}
	if( bDrawDebugSphere )
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), GlassSmashExplosionRadius, 10, FColor::Cyan, true, 5);
	}
	return GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params
		);
}

USphereComponent* AExplosiveProjectile::GetCollisionComp() const
{
	return CollisionComp;
}

UProjectileMovementComponent* AExplosiveProjectile::GetProjectileMovement() const
{
	return ProjectileMovement;
}
