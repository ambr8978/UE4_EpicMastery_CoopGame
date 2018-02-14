// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"

const int32 DEFAULT_NUM_TICKS = 0;
const int32 DEFAULT_TICKS_PROCESSED = 0;
const float DEFAULT_POWERUP_INTERVAL = 0.0f;

ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = DEFAULT_POWERUP_INTERVAL;
	TotalNumTicks = DEFAULT_NUM_TICKS;
	TicksProcessed = DEFAULT_TICKS_PROCESSED;
}

void ASPowerupActor::ActivatePowerup()
{
	if (PowerupInterval > 0.0f)
	{
		SetAndStartPowerupTimer();
	} 
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::OnTickPowerup()
{
	++TicksProcessed;
	OnPowerupTicked();

	if (TicksProcessed >= TotalNumTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::SetAndStartPowerupTimer()
{
	GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true, 0.0f);
}