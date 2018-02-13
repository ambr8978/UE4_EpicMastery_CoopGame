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
#include "Sound/SoundCue.h"

const float MOVEMENT_FORCE_DEFAULT = 1000;
const float REQUIRED_DISTANCE_DEFAULT = 100;

const float SELF_DESTRUCT_RADIAL_DAMAGE = 100;
const float SELF_DESTRUCT_RADIUS = 200;
const float SELF_DAMAGE_AMOUNT = 20;

const float TIMER_RATE_DAMAGE_SELF = 0.5f;
const float TIMER_DELAY_DAMAGE_SELF = 0.0f;

const float SPHERE_COMPONENT_RADIUS = 100;

const float NEARBY_BOTS_RADIUS = 600;
const int32 MAX_POWER_LEVEL = 4;
const int32 UPDATE_POWER_LEVEL_INTERVAL_SEC = 1.0f;

ASTrackerBot::ASTrackerBot()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupMeshComponent();
	SetRootComponent();
	SetupHealthComponent();
	SetupSphereComponent();

	PowerLevel = 0;
	DynamicMaterialToPulseOnDamage = nullptr;
	bUseVelocityChange = false;
	MovementForce = MOVEMENT_FORCE_DEFAULT;
	RequiredDistanceToTarget = REQUIRED_DISTANCE_DEFAULT;
	bExploded = false;
	bStartedSelfDestruction = false;
	ExplosionDamage = SELF_DESTRUCT_RADIAL_DAMAGE;
	ExplosionRadius = SELF_DESTRUCT_RADIUS;
	SelfDamageInterval = TIMER_RATE_DAMAGE_SELF;
}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
		InstantiatePowerLevelTimer();
	}
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Role == ROLE_Authority && !bExploded)
	{
		MoveTowardsTarget();
	}
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
	if (bStartedSelfDestruction || bExploded)
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
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle_SelfDamage,
			this,
			&ASTrackerBot::DamageSelf,
			SelfDamageInterval,
			true,
			TIMER_DELAY_DAMAGE_SELF);
	}

	bStartedSelfDestruction = true;
	UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
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
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	/*
	We are hiding the mesh so that the trackerbot still appears to be destroyed, even though
	it will still be present for a few seconds (see SetLifeSpan call below).
	*/
	MeshComponent->SetVisibility(false, true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		float ActualDamage = (ExplosionDamage + (ExplosionDamage * PowerLevel));

		UGameplayStatics::ApplyRadialDamage(
			this,
			ActualDamage,
			GetActorLocation(),
			ExplosionRadius,
			nullptr,
			IgnoredActors,
			this,
			GetInstigatorController(),
			true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		/*
		We set a short life span instead of out right destroying the actor so as to allow the client's
		SpawnEmitterAtLocation to fire before the actor is destroyed.  Otherwise, there is a race condition
		between the client receivng and processing the health updates (including spawning an emitter at the
		actor location) and the server destroying the actor (which is replicated on both server and client)
		*/
		SetLifeSpan(2.0f);
	}

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

	if (NavPath && (NavPath->PathPoints.Num() > 1))
	{
		//Return next point in path
		return NavPath->PathPoints[1];
	}

	//Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::OnCheckNearbyBots()
{
	FCollisionShape CollisionShape = CreateNearbyBotsCollisionShape();
	FCollisionObjectQueryParams QueryParams = GetNearbyBotsCollisionQueryParams();
	TArray<FOverlapResult> OverlapResults = GetOverlappingObjects(QueryParams, CollisionShape);
	DrawDebugSphere(GetWorld(), GetActorLocation(), NEARBY_BOTS_RADIUS, 12, FColor::White, false, 1.0f);

	int NumberOfOverlappingBots = CalculateNumberOverlappingBots(OverlapResults);
	PowerLevel = CalculatePowerLevel(NumberOfOverlappingBots);
	UpdateMaterialBasedOffOfPowerLevel();
}

FCollisionShape ASTrackerBot::CreateNearbyBotsCollisionShape()
{
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(NEARBY_BOTS_RADIUS);
	return CollisionShape;
}

FCollisionObjectQueryParams ASTrackerBot::GetNearbyBotsCollisionQueryParams()
{
	FCollisionObjectQueryParams QueryParams;
	/*
	Our tracker bot's mesh component is set to Physics Body in BP (defailt profile of physics simulated actors)
	*/
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	return QueryParams;
}

TArray<FOverlapResult> ASTrackerBot::GetOverlappingObjects(
	FCollisionObjectQueryParams QueryParams,
	FCollisionShape CollisionShape)
{
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		QueryParams,
		CollisionShape);

	return Overlaps;
}

int ASTrackerBot::CalculateNumberOverlappingBots(TArray<FOverlapResult> OverlapResults)
{
	int NumBots = 0;
	for (FOverlapResult Result : OverlapResults)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		if (Bot && (Bot != this))
		{
			++NumBots;
		}
	}
	return NumBots;
}

int ASTrackerBot::CalculatePowerLevel(int NumOverlappingBots)
{
	return FMath::Clamp(NumOverlappingBots, 0, MAX_POWER_LEVEL);
}

void ASTrackerBot::UpdateMaterialBasedOffOfPowerLevel()
{
	if (DynamicMaterialToPulseOnDamage == nullptr)
	{
		DynamicMaterialToPulseOnDamage = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComponent->GetMaterial(0));
	}

	if (DynamicMaterialToPulseOnDamage)
	{
		/*
		Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know
		the max power level, which can be tweaked many times by gameplay decisions (would mean we need to keep 2 places up to date).
		*/
		float Alpha = PowerLevel / (float)MAX_POWER_LEVEL;
		DynamicMaterialToPulseOnDamage->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
}

void ASTrackerBot::InstantiatePowerLevelTimer()
{
	FTimerHandle TimerHandle_CheckPowerLevel;
	GetWorldTimerManager().SetTimer(
		TimerHandle_CheckPowerLevel,
		this,
		&ASTrackerBot::OnCheckNearbyBots,
		UPDATE_POWER_LEVEL_INTERVAL_SEC,
		true);
}