// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

const int LINE_TRACE_LENGTH = 10000;
const FColor LINE_TRACE_COLOR = FColor::White;
const float LINE_TRACE_LIFETIME_SEC = 1.0f;
const bool LINE_TRACE_PERSISTENT = false;
const int LINE_TRACE_DEPTH_PRIORITY = 0;
const float LINE_TRACE_THICKNESS = 1.0f;

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

/*
There is a lot of object creation and calculation that takes place in this function. 
Seems like this function is an obvious candidate for optimization.
*/
void ASWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		LineTraceAndProcessDamage(MyOwner);		
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

	return QueryParams;
}

void ASWeapon::LineTraceAndProcessDamage(AActor* OwnerActor)
{
	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * LINE_TRACE_LENGTH);

	FHitResult HitResult;
	bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, ECC_Visibility, GetLineTraceCollisionQueryParams(OwnerActor));
	if (bBlockingHit)
	{
		//process damage.
	}

	DrawDebugLine(
		GetWorld(),
		EyeLocation,
		TraceEnd,
		LINE_TRACE_COLOR,
		LINE_TRACE_PERSISTENT,
		LINE_TRACE_LIFETIME_SEC,
		LINE_TRACE_DEPTH_PRIORITY,
		LINE_TRACE_THICKNESS);
}