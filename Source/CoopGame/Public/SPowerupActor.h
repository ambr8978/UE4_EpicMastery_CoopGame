// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPowerupActor();

	void ActivatePowerup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnExpired();
protected:
	int32 TicksProcessed;
	FTimerHandle TimerHandle_PowerupTick;

	/*
	Time between powerup ticks
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Powerup")
	float PowerupInterval;

	/*
	Total times we apply the powerup effect
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Powerup")
	int32 TotalNumTicks;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTickPowerup();

private:
	void SetAndStartPowerupTimer();
};
