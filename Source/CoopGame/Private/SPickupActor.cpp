// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerupActor.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FRotator DEFAULT_ROTATION_DECAL_COMPONENT = FRotator(90, 0.0f, 0.0f);
const float DEFAULT_RADIUS_SPHERE_COMPONENT = 75.0f;
const FVector DEFAULT_SIZE_DECAL_COMPONENT = FVector(64, 75, 75);
const float DEFAULT_COOLDOWN_DURATION = 0.0f;

ASPickupActor::ASPickupActor()
{
	SetupSphereComponent();
	SetupDecalComponent();
}

void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();
	Respawn();
}

void ASPickupActor::Respawn()
{
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Powerup class is nullptr in %s.  Please update your BP"), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

void ASPickupActor::SetupSphereComponent()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(DEFAULT_RADIUS_SPHERE_COMPONENT);
}

void ASPickupActor::SetupDecalComponent()
{
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetRelativeRotation(DEFAULT_ROTATION_DECAL_COMPONENT);
	DecalComponent->DecalSize = DEFAULT_SIZE_DECAL_COMPONENT;
	DecalComponent->SetupAttachment(RootComponent);
}

void ASPickupActor::SetRootComponent()
{
	RootComponent = SphereComponent;
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (PowerupInstance)
	{
		PowerupInstance->ActivatePowerup();
		PowerupInstance = nullptr;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickupActor::Respawn, CoolDownDuration);
	}
}