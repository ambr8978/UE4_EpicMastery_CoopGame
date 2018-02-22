// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();
	PrepareForNextWave();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();
	--NumBotsToSpawnInCurrentWave;
	if (NumBotsToSpawnInCurrentWave <= 0)
	{
		EndWave();
	}
}

void ASGameMode::StartWave()
{
	++WaveCount;
	NumBotsToSpawnInCurrentWave = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::PrepareForNextWave()
{
	FTimerHandle TimerHandle_NextWaveStart;
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, 1.0f, false, 0.0f);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	PrepareForNextWave();
}
