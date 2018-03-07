// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WorldCollision.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class USHealthComponent;
class UMaterialInstanceDynamic;
class UParticleSystem;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	ASTrackerBot();

	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	void OnPlayerOverlap();

protected:
	int32 PowerLevel;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComponent;

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

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;
	void DamageSelf();

	UMaterialInstanceDynamic* DynamicMaterialToPulseOnDamage;
	
	FVector NextPathPoint;
	FVector GetNextPathPoint();

	void OnCheckNearbyBots();
	FCollisionShape CreateNearbyBotsCollisionShape();
	FCollisionObjectQueryParams GetNearbyBotsCollisionQueryParams();
	TArray<FOverlapResult> GetOverlappingObjects(FCollisionObjectQueryParams QueryParams, FCollisionShape CollisionShape);
	int CalculateNumberOverlappingBots(TArray<FOverlapResult> OverlapResults);
	int CalculatePowerLevel(int NumOverlappingBots);

	void UpdateMaterialBasedOffOfPowerLevel();
	void InstantiatePowerLevelTimer();

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
	bool bStartedSelfDestruction;
	bool bExploded;
	FTimerHandle TimerHandle_RefreshPath;

	void SetupHealthComponent();
	void SetRootComponent();
	void SetupMeshComponent();
	void SetupSphereComponent();

	void MoveTowardsTarget();
	void KeepMovingTowardsTarget();
	void UpdateDynamicMaterial();
	void SelfDestruct();

	AActor* GetNearestTarget();
	void StartPathFindingRefreshTimer();
	void RefreshPath();
};
