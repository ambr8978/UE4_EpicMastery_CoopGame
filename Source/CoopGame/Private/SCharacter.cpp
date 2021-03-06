// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SHealthComponent.h"
#include "CoopGame.h"
#include "SWeapon.h"
#include "Net/UnrealNetwork.h"

const float ZOOMED_FOV_DEFAULT = 65.0f;
const float ZOOM_INTERP_SPEED_DEFAULT = 20.0f;
const float TIME_UNTIL_PAWN_DESTROY_AFTER_DEATH_SEC = 10.0f;
const FName WEAPON_SOCKET_NAME = "WeaponSocket";

ASCharacter::ASCharacter()
{
	EnableTicking();
	SetupSpringArmComponent();
	SetupHealthComponent();
	SetupCameraComponent();
	SetupCapsuleComponentCollision();

	bDied = false;
	ZoomedFOV = ZOOMED_FOV_DEFAULT;
	ZoomInterpSpeed = ZOOM_INTERP_SPEED_DEFAULT;
	WeaponAttachSocketName = WEAPON_SOCKET_NAME;
}

void ASCharacter::EnableTicking()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASCharacter::SetupSpringArmComponent()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;
}

void ASCharacter::SetupCameraComponent()
{
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void ASCharacter::SetupHealthComponent()
{
	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
}

void ASCharacter::SetupCapsuleComponentCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
}

void ASCharacter::EnableCrouching()
{
	/*
	Yes, this code is weird.
	NavAgentProperties is usually used for AI, but since parts of Unreal are coded very uncleanly and
	with weird dependencies, the engine code still gets around to checking this variable when we try
	to crouch.
	*/
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	EnableCrouching();
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	InitCurrentFOV();

	if (Role == ROLE_Authority)
	{
		SpawnDefaultWeapon();
	}
}

void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetCurrentFOV(DeltaTime);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	/*
	We use the camera location as our pawn view location so that
	our weapon line trace (which is based off our pawn's eye location)
	is more accurate to what the player would expect.
	*/
	if (CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
}

void ASCharacter::MoveForward(float MovementValue)
{
	AddMovementInput(GetActorForwardVector() * MovementValue);
}

void ASCharacter::MoveRight(float MovementValue)
{
	AddMovementInput(GetActorRightVector() * MovementValue);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::InitCurrentFOV()
{
	bWantsToZoom = false;
	DefaultFOV = CameraComponent->FieldOfView;
	SetCurrentFOV(0);
}

void ASCharacter::SetCurrentFOV(float DeltaTime)
{
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(
		CameraComponent->FieldOfView,
		TargetFOV,
		DeltaTime,
		ZoomInterpSpeed);

	CameraComponent->SetFieldOfView(NewFOV);
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::SpawnDefaultWeapon()
{
	/*
	Following two lines are done in order to insure that the weapon always spawns
	*/
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(
		StarterWeaponClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
		);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);

		CurrentWeapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocketName
		);
	}
}

void ASCharacter::OnHealthChanged(
	USHealthComponent* HealthComp,
	float Health,
	float  HealthDelta,
	const class UDamageType* DamageType,
	class AController* DamageInstigator,
	AActor* DamageCauser)
{
	if ((Health <= 0.0f) &&
		(!bDied))
	{
		Die();
	}
}

void ASCharacter::Die()
{
	bDied = true;
	GetMovementComponent()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetachFromControllerPendingDestroy();
	SetLifeSpan(TIME_UNTIL_PAWN_DESTROY_AFTER_DEATH_SEC);
}