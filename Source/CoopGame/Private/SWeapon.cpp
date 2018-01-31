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
#include "Net/UnrealNetwork.h"

const bool TIMER_SHOULD_LOOP = true;

const int LINE_TRACE_LENGTH = 10000;
const FColor LINE_TRACE_COLOR = FColor::White;
const float LINE_TRACE_LIFETIME_SEC = 1.0f;
const bool LINE_TRACE_PERSISTENT = false;
const int LINE_TRACE_DEPTH_PRIORITY = 0;
const float LINE_TRACE_THICKNESS = 1.0f;

const float DAMAGE_HEADSHOT_MULTIPLIER = 4;

/*
Check out Lecture 88, around the 8:30 mark for more of a breakdown
*/
const float NET_UPDATE_FREQ_DEFAULT_TICS_PER_SEC = 66.0f;
const float MIN_NET_UPDATE_FREQ_TICS_PER_SEC = 33.0f;

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

	NetUpdateFrequency = NET_UPDATE_FREQ_DEFAULT_TICS_PER_SEC;
	MinNetUpdateFrequency = MIN_NET_UPDATE_FREQ_TICS_PER_SEC;
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

void ASWeapon::OnRep_HitScanTrace()
{
	SpawnShotEffects(HitScanTrace.TraceTo);
	SpawnHitEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

/*
There is a lot of object creation and calculation that takes place in this function.
Seems like this function is an obvious candidate for optimization.
*/
void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		ProcessLineTrace(MyOwner);
	}
}

void ASWeapon::ProcessLineTrace(AActor* OwnerActor)
{
	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (ShotDirection * LINE_TRACE_LENGTH);
	FVector TraceEndPoint = TraceEnd;
	EPhysicalSurface SurfaceHit = SurfaceType_Default;

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
		SurfaceHit = GetHitSurfaceType(HitResult);
		SpawnHitEffects(SurfaceHit, HitResult.ImpactPoint);
	}

	SpawnShotEffects(TraceEndPoint);
	PlayWeaponShakeAnimation();
	UpdateHitScanTrace(TraceEndPoint, SurfaceHit);
	LastFireTime = GetWorld()->TimeSeconds;
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

void ASWeapon::SpawnShotEffects(FVector TraceEndPoint)
{
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

void ASWeapon::SpawnHitEffects(EPhysicalSurface SurfaceHit, FVector ImpactPoint)
{
	UParticleSystem* HitSurfaceParticleEffect = GetHitSurfaceParticleEffect(SurfaceHit);
	FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
	FVector ShotDirection = ImpactPoint - MuzzleLocation;
	ShotDirection.Normalize();

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitSurfaceParticleEffect,
		ImpactPoint,
		ShotDirection.Rotation());
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

void ASWeapon::UpdateHitScanTrace(FVector TraceEndPoint, EPhysicalSurface SurfaceHit)
{
	if (Role == ROLE_Authority)
	{
		HitScanTrace.TraceTo = TraceEndPoint;
		HitScanTrace.SurfaceType = SurfaceHit;
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}