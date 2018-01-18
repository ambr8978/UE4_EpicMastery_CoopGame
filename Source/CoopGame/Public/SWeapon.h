// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ImpactEffect;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();	

	virtual void BeginPlay() override;
private:

	FCollisionQueryParams GetLineTraceCollisionQueryParams(AActor* OwnerActor);
	void ProcessLineTrace(AActor* OwnerActor);
	void ProcessDamage(FHitResult HitResult, FVector ShotDirection, AController* InstigatorController);

	void SpawnShotEffects(FVector EyeLocation, FVector TraceEnd);
	void SpawnHitEffects(FHitResult HitResult);
};
