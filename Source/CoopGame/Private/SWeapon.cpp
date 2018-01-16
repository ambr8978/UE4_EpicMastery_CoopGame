// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"

ASWeapon::ASWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

