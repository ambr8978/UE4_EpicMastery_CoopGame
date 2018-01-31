// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

/*
Contains information of a single hitscan weapon linetrace
*/
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();

	void StartFire();
	void StopFire();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCameraShake;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	FTimerHandle TimerHandle_TimeBetweenShots;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	virtual void Fire();	

private:
	float TimeBetweenShots;
	float LastFireTime;

	FCollisionQueryParams GetLineTraceCollisionQueryParams(AActor* OwnerActor);
	void ProcessLineTrace(AActor* OwnerActor);
	void ProcessDamage(FHitResult HitResult, FVector ShotDirection, AController* InstigatorController);
	float GetDamageBasedOffHitSurface(EPhysicalSurface HitSurface);
	void UpdateHitScanTrace(FVector TraceEndPoint, EPhysicalSurface SurfaceHit);

	void SpawnShotEffects(FVector TraceEnd);
	void SpawnHitEffects(EPhysicalSurface SurfaceHit, FVector ImpactPoint);
	UParticleSystem* GetHitSurfaceParticleEffect(EPhysicalSurface SurfaceType);
	EPhysicalSurface GetHitSurfaceType(FHitResult HitResult);
	void SpawnMuzzleEffect();
	void SpawnTraceEffect(FVector TraceEndPoint);
	void PlayWeaponShakeAnimation();
};
