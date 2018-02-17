// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPickupActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComponent;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerupClass;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CoolDownDuration;

	virtual void BeginPlay() override;
	void Respawn();

private:
	ASPowerupActor* PowerupInstance;
	FTimerHandle TimerHandle_RespawnTimer;

	void SetupSphereComponent();
	void SetupDecalComponent();
	void SetRootComponent();
};
