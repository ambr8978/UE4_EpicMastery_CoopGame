// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationPath.h"
#include "SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"

const float MOVEMENT_FORCE_DEFAULT = 1000;
const float REQUIRED_DISTANCE_DEFAULT = 100;

const float SELF_DESTRUCT_RADIAL_DAMAGE = 100;
const float SELF_DESTRUCT_RADIUS = 200;
const float SELF_DAMAGE_AMOUNT = 20;

const float TIMER_RATE_DAMAGE_SELF = 0.5f;
const float TIMER_DELAY_DAMAGE_SELF = 0.0f;

const float SPHERE_COMPONENT_RADIUS = 100;

ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupMeshComponent();
	SetRootComponent();
	SetupHealthComponent();
	SetupSphereComponent();

	DynamicMaterialToPulseOnDamage = nullptr;
	bUseVelocityChange = false;
	MovementForce = MOVEMENT_FORCE_DEFAULT;
	RequiredDistanceToTarget = REQUIRED_DISTANCE_DEFAULT;
	bExploded = false;
	bStartedSelfDestruction = false;
	ExplosionDamage = SELF_DESTRUCT_RADIAL_DAMAGE;
	ExplosionRadius = SELF_DESTRUCT_RADIUS;
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
	UpdateDynamicMaterial();
	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (bStartedSelfDestruction)
	{
		return;
	}

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn)
	{
		OnPlayerOverlap();
	}
}

void ASTrackerBot::OnPlayerOverlap()
{
	GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, TIMER_RATE_DAMAGE_SELF, true, TIMER_DELAY_DAMAGE_SELF);
	bStartedSelfDestruction = true;
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, SELF_DAMAGE_AMOUNT, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::UpdateDynamicMaterial()
{
	if (DynamicMaterialToPulseOnDamage == nullptr)
	{
		DynamicMaterialToPulseOnDamage = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComponent->GetMaterial(0));
	}

	if (DynamicMaterialToPulseOnDamage)
	{
		DynamicMaterialToPulseOnDamage->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}
	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(
		this, 
		ExplosionDamage, 
		GetActorLocation(), 
		ExplosionRadius , 
		nullptr, 
		IgnoredActors, 
		this, 
		GetInstigatorController(), 
		true);

	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

	Destroy();
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

void ASTrackerBot::SetupSphereComponent()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(SPHERE_COMPONENT_RADIUS);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	/*
	We set up the coliision this way so that we don't register for any collision events that we don't need.
	This makes it cheaper for the physics engine to deal with and we get less events that we need to handle.
	*/
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetupAttachment(RootComponent);
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