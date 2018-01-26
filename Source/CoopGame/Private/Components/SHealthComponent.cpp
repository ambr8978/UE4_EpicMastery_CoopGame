// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Controller.h"


const float HEALTH_DEFAULT = 100;

USHealthComponent::USHealthComponent()
{
	DefaultHealth = HEALTH_DEFAULT;
}


void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::TakeAnyDamage);
	}

	Health = DefaultHealth;
}

void USHealthComponent::TakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const class UDamageType* DamageType,
	class AController* DamageInstigator,
	AActor* DamageCauser)
{
	if (Damage <= 0.0f)
	{
		return;
	}

	ApplyDamage(Damage, DamageType, DamageInstigator, DamageCauser);
}

void USHealthComponent::ApplyDamage(
	float Damage, 
	const UDamageType* DamageType, 
	AController* DamageInstigator, 
	AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, DamageInstigator, DamageCauser);
}
