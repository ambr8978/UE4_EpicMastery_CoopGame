// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	ASExplosiveBarrel();

protected:
	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComponent;

	UFUNCTION()
	void OnHealthChanged(
		USHealthComponent* HealthComp,
		float Health,
		float  HealthDelta,
		const class UDamageType* DamageType,
		class AController* DamageInstigator,
		AActor* DamageCauser);
	
	/*
	Impulse applied to the barrel mesh when it explodes to boost it up a little
	*/
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	/*
	Particle to play when health reached zero	
	*/
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	/*
	The material to replace the original on the mesh once eplxoded (a blackened version)
	*/
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

private:
	void SetupHealthComponent();
	void SetupStaticMeshComponent();
	void SetupRadialForceComponent();
	void SetupReplication();

	void BoostBarrelUpwards();
	void PlayFX();
	void SetMaterialToExplodedMaterial();
	void ApplyRadialForce();
};
