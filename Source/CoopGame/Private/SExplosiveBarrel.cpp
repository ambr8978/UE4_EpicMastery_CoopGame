// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"

const int RADIAL_FORCE_COMPONENT_RADIUS = 250;
const int EXPLOSION_IMPULSE_DEFAULT = 400;

ASExplosiveBarrel::ASExplosiveBarrel()
{
	SetupHealthComponent();
	SetUpStaticMeshComponent();
	RootComponent = MeshComponent;
	SetUpRadialForceComponent();
}

void ASExplosiveBarrel::SetupHealthComponent()
{
	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
}

void ASExplosiveBarrel::SetUpStaticMeshComponent()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	/*
	Set to physics body to let radial component affect us (i.e. when a nearby barrel explodes)
	*/
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
}

void ASExplosiveBarrel::SetUpRadialForceComponent()
{
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(MeshComponent);
	RadialForceComponent->Radius = RADIAL_FORCE_COMPONENT_RADIUS;
	RadialForceComponent->bImpulseVelChange = true;
	/*
	Prevent component from ticking, and only use FireImpule() instead
	*/
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->bIgnoreOwningActor = true;

	ExplosionImpulse = EXPLOSION_IMPULSE_DEFAULT;
}

void ASExplosiveBarrel::OnHealthChanged(
	USHealthComponent* HealthComp,
	float Health,
	float  HealthDelta,
	const class UDamageType* DamageType,
	class AController* DamageInstigator,
	AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health <= 0.0f)
	{
		bExploded = true;

		BoostBarrelUpwards();
		PlayFX();
		SetMaterialToExplodedMaterial();
		ApplyRadialForce();
	}
}

void ASExplosiveBarrel::BoostBarrelUpwards()
{
	FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
	MeshComponent->AddImpulse(BoostIntensity, NAME_None, true);
}

void ASExplosiveBarrel::PlayFX()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
}

void ASExplosiveBarrel::SetMaterialToExplodedMaterial()
{
	MeshComponent->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::ApplyRadialForce()
{
	RadialForceComponent->FireImpulse();
}