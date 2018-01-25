// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;

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
	virtual void Fire();	

private:
	//Derived from RateOfFire
	float TimeBetweenShots;
	float LastFireTime;

	FCollisionQueryParams GetLineTraceCollisionQueryParams(AActor* OwnerActor);
	void ProcessLineTrace(AActor* OwnerActor);
	void ProcessDamage(FHitResult HitResult, FVector ShotDirection, AController* InstigatorController);
	float GetDamageBasedOffHitSurface(EPhysicalSurface HitSurface);

	void SpawnShotEffects(FVector EyeLocation, FVector TraceEnd);
	void SpawnHitEffects(FHitResult HitResult);
	UParticleSystem* GetHitSurfaceParticleEffect(EPhysicalSurface SurfaceType);
	EPhysicalSurface GetHitSurfaceType(FHitResult HitResult);
	void SpawnMuzzleEffect();
	void SpawnTraceEffect(FVector TraceEndPoint);
	void PlayWeaponShakeAnimation();
};
