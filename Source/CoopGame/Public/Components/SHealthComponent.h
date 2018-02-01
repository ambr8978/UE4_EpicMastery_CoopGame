// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

class AController;

/*
OnHealthChangedEvent
FOnHealthChangedSignature is name of event type, rest are parameters
1) HealthComponent that triggered event
2) Health
3) Health Delta
...
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
FOnHealthChangedSignature,
USHealthComponent*, HealthComponent,
float, Health,
float, HealthDelta,
const class UDamageType*, DamageType,
class AController*, DamageInstigator,
AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "HealthComponent")
	float Health;
		
	virtual void BeginPlay() override;
	void CreateTakeDamageHook();

	UFUNCTION()
	virtual void TakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const class UDamageType* DamageType,
		class AController* DamageInstigator,
		AActor* DamageCauser);
	
	void ApplyDamage(
		float Damage,
		const UDamageType* DamageType,
		AController* DamageInstigator,
		AActor* DamageCauser);

};
