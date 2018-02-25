// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "SHealthComponent.h"
#include "SGameState.h"
#include "SPlayerState.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	WaveCount = 0;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();

	if (!IsAnyPlayerAlive())
	{
		GameOver();
	}
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
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, 1.0f, false, 0.0f);
	SetWaveState(EWaveState::WaitingToStartWave);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	SetWaveState(EWaveState::WaitingToCompleteWave);
}

void ASGameMode::CheckWaveState()
{
	if (NextWavePreparationInProgress() || BotsStillNeedToBeSpawned() || IsAnyBotAlive())
	{
		return;
	}

	SetWaveState(EWaveState::WaveComplete);
	PrepareForNextWave();
}

bool ASGameMode::NextWavePreparationInProgress()
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
}

bool ASGameMode::BotsStillNeedToBeSpawned()
{
	return (NumBotsToSpawnInCurrentWave > 0);
}

bool ASGameMode::IsAnyBotAlive()
{
	for (FConstPawnIterator Itr = GetWorld()->GetPawnIterator(); Itr; ++Itr)
	{
		APawn* TestPawn = Itr->Get();
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		if (IsBotAlive(TestPawn))
		{
			return true;
		}
	}

	return false;;
}

bool ASGameMode::IsBotAlive(APawn* Bot)
{
	USHealthComponent* HealthComponent = Cast<USHealthComponent>(Bot->GetComponentByClass(USHealthComponent::StaticClass()));
	if (HealthComponent && (HealthComponent->GetHealth() > 0.0f))
	{
		return true;
	}

	return false;
}

bool ASGameMode::IsAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator Itr = GetWorld()->GetPlayerControllerIterator(); Itr; ++Itr)
	{
		APlayerController* PlayerController = Itr->Get();
		if (PlayerController && PlayerController->GetPawn())
		{
			APawn* MyPawn = PlayerController->GetPawn();
			USHealthComponent* HealthComponent = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (ensure(HealthComponent) && (HealthComponent->GetHealth() > 0.0f))
			{
				return true;
			}
		}
	}
	
	return false;
}

void ASGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
	UE_LOG(LogTemp, Log, TEXT("GAME OVER, PLAYERS DIED!"));
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GameState = GetGameState<ASGameState>();
	if (ensureAlways(GameState))
	{
		GameState->SetWaveState(NewState);
	}
}