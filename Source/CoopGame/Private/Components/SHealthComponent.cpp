// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"

const float HEALTH_DEFAULT = 100;
const uint8 TEAM_NUM_DEFAULT = 255;

USHealthComponent::USHealthComponent()
{
	DefaultHealth = HEALTH_DEFAULT;
	TeamNum = TEAM_NUM_DEFAULT;
	bIsDead = false;
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

void USHealthComponent::Heal(float HealAmount)
{
	if ((HealAmount <= 0.0f) || (Health <= 0.0f))
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
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
	if (Damage <= 0.0f || bIsDead)
	{
		return;
	}

	if ((DamageCauser != DamagedActor) && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	ApplyDamage(Damage);
	BroadcastHealthChange(Damage, DamageType, DamageInstigator, DamageCauser);
}

void USHealthComponent::ApplyDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
	bIsDead = (Health <= 0.0f);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s, is dead:%d"), *FString::SanitizeFloat(Health), bIsDead);
}

void USHealthComponent::BroadcastHealthChange(
	float Damage,
	const UDamageType* DamageType,
	AController* DamageInstigator,
	AActor* DamageCauser)
{
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, DamageInstigator, DamageCauser);

	if (bIsDead)
	{
		BroadcastDeath(DamageCauser, DamageInstigator);
	}
}

void USHealthComponent::BroadcastDeath(AActor* DamageCauser, AController* DamageInstigator)
{
	ASGameMode* GameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser, DamageInstigator);
	}
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent, Health);
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

bool USHealthComponent::IsFriendly(AActor* FirstActor, AActor* SecondActor)
{
	if (FirstActor == nullptr || SecondActor == nullptr)
	{
		/*	
		Assume friendly
		*/
		return true;
	}

	USHealthComponent* FirstHealthComponent = Cast<USHealthComponent>(FirstActor->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* SecondHealthComponent = Cast<USHealthComponent>(SecondActor->GetComponentByClass(USHealthComponent::StaticClass()));
	if (FirstHealthComponent == nullptr || SecondHealthComponent == nullptr)
	{
		/*
		Assume that if thge other actor does not have a health component,
		then he must not explicitly	be an enemy.
		*/
		return true;
	}

	return (FirstHealthComponent->TeamNum == SecondHealthComponent->TeamNum);
}