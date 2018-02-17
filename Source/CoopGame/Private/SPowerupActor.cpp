// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

const int32 DEFAULT_NUM_TICKS = 0;
const int32 DEFAULT_TICKS_PROCESSED = 0;
const float DEFAULT_POWERUP_INTERVAL = 0.0f;

ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = DEFAULT_POWERUP_INTERVAL;
	TotalNumTicks = DEFAULT_NUM_TICKS;
	TicksProcessed = DEFAULT_TICKS_PROCESSED;
	bIsPowerupActive = false;

	SetReplicates(true);
}

void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		SetAndStartPowerupTimer();
	} 
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::OnTickPowerup()
{
	++TicksProcessed;
	OnPowerupTicked();

	if (TicksProcessed >= TotalNumTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::SetAndStartPowerupTimer()
{
	GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}