// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"

const FRotator DEFAULT_ROTATION_DECAL_COMPONENT = FRotator(90, 0.0f, 0.0f);
const float DEFAULT_RADIUS_SPHERE_COMPONENT = 75.0f;
const FVector DEFAULT_SIZE_DECAL_COMPONENT = FVector(64, 75, 75);

ASPickupActor::ASPickupActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupSphereComponent();
	SetupDecalComponent();
}

void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

}

void ASPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}