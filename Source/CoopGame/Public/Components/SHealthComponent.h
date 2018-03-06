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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	uint8 TeamNum;

	/*
	BlueprintPure allows us to not have to pass the white execution pin into
	this function call when using it in BP which makes it a little easier to use.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="HealthComponent")
	static bool IsFriendly(AActor* FirstActor, AActor* SecondActor);

	float GetHealth() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float HealAmount);

protected:
	bool bIsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;
		
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	virtual void BeginPlay() override;
	void CreateTakeDamageHook();

	UFUNCTION()
	virtual void TakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const class UDamageType* DamageType,
		class AController* DamageInstigator,
		AActor* DamageCauser);
	
	void ApplyDamage(float Damage);

	void BroadcastHealthChange(
		float Damage,
		const UDamageType* DamageType,
		AController* DamageInstigator,
		AActor* DamageCauser);

	void BroadcastDeath(AActor* DamageCauser, AController* DamageInstigator);

};
