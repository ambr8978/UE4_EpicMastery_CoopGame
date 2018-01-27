// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USHealthComponent;
class ASWeapon;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	ASWeapon* CurrentWeapon;
	bool bWantsToZoom;
	float DefaultFOV;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player ")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;
	 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCameraComponent* CameraComponent;

	USHealthComponent* HealthComponent;

	virtual void BeginPlay() override;

	virtual FVector GetPawnViewLocation() const override;

	void StartFire();
	void StopFire();

	void MoveForward(float MovementValue);
	void MoveRight(float MovementValue);

	void BeginCrouch();
	void EndCrouch();

	void EnableTicking();
	void SetupSpringArmComponent();
	void EnableCrouching();
	void SetupCameraComponent();
	void SetupHealthComponent();
	void SetupCapsuleComponentCollision();

	void InitCurrentFOV();
	void SetCurrentFOV(float DeltaTime);

	void BeginZoom();
	void EndZoom();

	void SpawnDefaultWeapon();

	UFUNCTION()
	void OnHealthChanged(
		USHealthComponent* HealthComponent,
		float Health,
		float  HealthDelta,
		const class UDamageType* DamageType,
		class AController* DamageInstigator,
		AActor* DamageCauser);

	void Die();
};
