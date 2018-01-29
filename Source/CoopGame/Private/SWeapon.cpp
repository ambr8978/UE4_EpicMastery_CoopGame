// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "CoopGame.h"

const bool TIMER_SHOULD_LOOP = true;

const int LINE_TRACE_LENGTH = 10000;
const FColor LINE_TRACE_COLOR = FColor::White;
const float LINE_TRACE_LIFETIME_SEC = 1.0f;
const bool LINE_TRACE_PERSISTENT = false;
const int LINE_TRACE_DEPTH_PRIORITY = 0;
const float LINE_TRACE_THICKNESS = 1.0f;

const float DAMAGE_HEADSHOT_MULTIPLIER = 4;


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines For Weapons"),
	ECVF_Cheat);

ASWeapon::ASWeapon()
{
	SetReplicates(true);
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	RateOfFire = 600.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
	LastFireTime = -TimeBetweenShots;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f); 

	/*
	Call Fire() every TIMER_RATE sec
	*/
	GetWorldTimerManager().SetTimer(
		TimerHandle_TimeBetweenShots, 
		this, 
		&ASWeapon::Fire, 
		TimeBetweenShots, 
		TIMER_SHOULD_LOOP,
		FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

/*
There is a lot of object creation and calculation that takes place in this function.
Seems like this function is an obvious candidate for optimization.
*/
void ASWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		ProcessLineTrace(MyOwner);
	}
}

FCollisionQueryParams ASWeapon::GetLineTraceCollisionQueryParams(AActor* OwnerActor)
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.AddIgnoredActor(this);
	/*
	Setting this to true gives us the exact location the line trace hit.
	If this were set to false, this might give us a more simple collision, like whether it hit the collision box, etc.
	I would imagine that this would be necessary if we wanted to differentiate between head/non-head hits.
	*/
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	return QueryParams;
}

void ASWeapon::ProcessLineTrace(AActor* OwnerActor)
{
	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (ShotDirection * LINE_TRACE_LENGTH);
	FVector TraceEndPoint = TraceEnd;

	FHitResult HitResult;
	bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		EyeLocation, 
		TraceEnd, 
		COLLISION_WEAPON, 
		GetLineTraceCollisionQueryParams(OwnerActor));

	if (bBlockingHit)
	{
		TraceEndPoint = HitResult.ImpactPoint;
		ProcessDamage(HitResult, ShotDirection, OwnerActor->GetInstigatorController());
		SpawnHitEffects(HitResult);
	}

	SpawnShotEffects(EyeLocation, TraceEndPoint);
	PlayWeaponShakeAnimation();
	LastFireTime = GetWorld()->TimeSeconds;
}

void ASWeapon::ProcessDamage(FHitResult HitResult, FVector ShotDirection, AController* InstigatorController)
{
	AActor* HitActor = HitResult.GetActor();
	float DamageDealt = GetDamageBasedOffHitSurface(GetHitSurfaceType(HitResult));

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		DamageDealt,
		ShotDirection,
		HitResult,
		InstigatorController,
		this,
		DamageType);
}

float ASWeapon::GetDamageBasedOffHitSurface(EPhysicalSurface HitSurface)
{
	float DamageDealt = BaseDamage;
	if (HitSurface == SURFACE_FLESHVULNERABLE)
	{
		DamageDealt *= DAMAGE_HEADSHOT_MULTIPLIER;
	}

	return DamageDealt;
}

void ASWeapon::SpawnShotEffects(FVector EyeLocation, FVector TraceEndPoint)
{
	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(
			GetWorld(),
			EyeLocation,
			TraceEndPoint,
			LINE_TRACE_COLOR,
			LINE_TRACE_PERSISTENT,
			LINE_TRACE_LIFETIME_SEC,
			LINE_TRACE_DEPTH_PRIORITY,
			LINE_TRACE_THICKNESS);
	}

	SpawnMuzzleEffect();
	SpawnTraceEffect(TraceEndPoint);	
	PlayWeaponShakeAnimation();
}

void ASWeapon::SpawnMuzzleEffect()
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}
}

void ASWeapon::SpawnTraceEffect(FVector TraceEndPoint)
{
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComponent)
		{
			/*
			This VectorParameter and it's name are coming from the P_SmokeTrail particle asset's Target section
			and it's respective Distribution.Parameter Name
			*/
			TracerComponent->SetVectorParameter("Target", TraceEndPoint);
		}
	}
}

void ASWeapon::SpawnHitEffects(FHitResult HitResult)
{

	UParticleSystem* HitSurfaceParticleEffect = GetHitSurfaceParticleEffect(GetHitSurfaceType(HitResult));
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitSurfaceParticleEffect,
		HitResult.ImpactPoint,
		HitResult.ImpactNormal.Rotation()
	);
}

UParticleSystem* ASWeapon::GetHitSurfaceParticleEffect(EPhysicalSurface SurfaceType)
{	
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
	}

	return SelectedEffect;
}

EPhysicalSurface ASWeapon::GetHitSurfaceType(FHitResult HitResult)
{
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void ASWeapon::PlayWeaponShakeAnimation()
{
	APawn* OwnerActor = Cast<APawn>(GetOwner());
	if (OwnerActor)
	{
		APlayerController* PlayerController = Cast<APlayerController>(OwnerActor->GetController());
		if (PlayerController)
		{
			PlayerController->ClientPlayCameraShake(FireCameraShake);
		}
	}
}