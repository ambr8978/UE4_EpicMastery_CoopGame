// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void StartPlay() override;
	ASGameMode();

protected:
	int32 WaveCount;
	int32 NumBotsToSpawnInCurrentWave;
	FTimerHandle TimerHandle_BotSpawner;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	/*
	Hook for BP to spawn a single bot
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	/*
	Start spawning bots
	*/
	void StartWave();

	/*
	Stop spawning bots
	*/
	void EndWave();

	/*
	Set timer for next startwave
	*/
	void PrepareForNextWave();
	
};
