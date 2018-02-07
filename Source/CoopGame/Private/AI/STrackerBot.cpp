// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationPath.h"
#include "SHealthComponent.h"

const float MOVEMENT_FORCE_DEFAULT = 1000;
const float REQUIRED_DISTANCE_DEFAULT = 100;

ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupMeshComponent();
	SetRootComponent();
	SetupHealthComponent();

	bUseVelocityChange = false;
	MovementForce = MOVEMENT_FORCE_DEFAULT;
	RequiredDistanceToTarget = REQUIRED_DISTANCE_DEFAULT;
}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	NextPathPoint = GetNextPathPoint();
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MoveTowardsTarget();
}

void ASTrackerBot::HandleTakeDamage(
	USHealthComponent* HealthComp, 
	float Health, float HealthDelta, 
	const class UDamageType* DamageType, 
	class AController* InstigatedBy, 
	AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

void ASTrackerBot::MoveTowardsTarget()
{
	float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();
	if (DistanceToTarget <= RequiredDistanceToTarget)
	{
		NextPathPoint = GetNextPathPoint();
	}
	else
	{
		KeepMovingTowardsTarget();
	}
}

void ASTrackerBot::KeepMovingTowardsTarget()
{
	FVector ForceDirection = NextPathPoint - GetActorLocation();
	ForceDirection.Normalize();
	ForceDirection *= MovementForce;
	MeshComponent->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
}

void ASTrackerBot::SetupMeshComponent()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCanEverAffectNavigation(false);
	MeshComponent->SetSimulatePhysics(true);
}

void ASTrackerBot::SetRootComponent()
{
	RootComponent = MeshComponent;
}

void ASTrackerBot::SetupHealthComponent()
{
	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);
}

FVector ASTrackerBot::GetNextPathPoint()
{
	//Hack to get player location
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), (AActor*)PlayerPawn);

	if (NavPath->PathPoints.Num() > 1)
	{
		//Return next point in path
		return NavPath->PathPoints[1];
	}

	//Failed to find path
	return GetActorLocation();
}