// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"

const float HEALTH_DEFAULT = 100;

USHealthComponent::USHealthComponent()
{
	Health = HEALTH_DEFAULT;
}


void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}
