// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

ASCharacter::ASCharacter()
{
	EnableTicking();
	SetupSpringArmComponent();
	SetupCameraComponent();
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
}

void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

