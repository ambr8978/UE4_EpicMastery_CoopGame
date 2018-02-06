// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationPath.h"

ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupMeshComponent();
	SetRootComponent();
}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASTrackerBot::SetupMeshComponent()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCanEverAffectNavigation(false);
}

void ASTrackerBot::SetRootComponent()
{
	RootComponent = MeshComponent;
}

FVector ASTrackerBot::GetNextPathPoint()
{
	//Hack to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), (AActor*) PlayerPawn);
	
	if (NavPath->PathPoints.Num() > 1)
	{
		//Return next point in path
		return NavPath->PathPoints[1];
	}

	//Failed to find path
	return GetActorLocation();
}