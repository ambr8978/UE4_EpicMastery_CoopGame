// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();	

private:

	FCollisionQueryParams GetLineTraceCollisionQueryParams(AActor* OwnerActor);
	void LineTraceAndProcessDamage(AActor* OwnerActor);
	void ProcessDamage(FHitResult HitResult, FVector ShotDirection, AController* InstigatorController);
};
