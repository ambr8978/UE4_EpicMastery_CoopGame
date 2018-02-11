// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

const float HEALTH_DEFAULT = 100;

USHealthComponent::USHealthComponent()
{
	DefaultHealth = HEALTH_DEFAULT;
	SetIsReplicated(true);
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		CreateTakeDamageHook();
	}

	Health = DefaultHealth;
}

void USHealthComponent::CreateTakeDamageHook()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::TakeAnyDamage);
	}
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
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

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent, Health);
}
