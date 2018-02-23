// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

enum class EWaveState : uint8;

UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	int32 WaveCount;
	int32 NumBotsToSpawnInCurrentWave;
	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;

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
	void GameOver();

	/*
	Set timer for next startwave
	*/
	void PrepareForNextWave();

	void CheckWaveState();
	
private:
	bool IsBotAlive(APawn* Bot);
	bool IsAnyBotAlive();
	bool IsAnyPlayerAlive();

	bool BotsStillNeedToBeSpawned();
	bool NextWavePreparationInProgress();

	void SetWaveState(EWaveState NewState);
};
