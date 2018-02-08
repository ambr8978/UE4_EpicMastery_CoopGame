// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class UMaterialInstanceDynamic;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	ASTrackerBot();

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;
	 
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;
	
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	UMaterialInstanceDynamic* DynamicMaterialToPulseOnDamage;
	
	FVector NextPathPoint;
	FVector GetNextPathPoint();

	virtual void BeginPlay() override;
	UFUNCTION()
	void HandleTakeDamage(
		USHealthComponent* HealthComp,
		float Health,
		float HealthDelta,
		const class UDamageType* DamageType,
		class AController* InstigatedBy,
		AActor* DamageCauser);
private:
	bool bExploded;

	void SetupHealthComponent();
	void SetRootComponent();
	void SetupMeshComponent();

	void MoveTowardsTarget();
	void KeepMovingTowardsTarget();
	void UpdateDynamicMaterial();
	void SelfDestruct();
};