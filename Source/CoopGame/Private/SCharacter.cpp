// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SetupSpringArmComponent();
	SetupCameraComponent();
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
}

void ASCharacter::MoveForward(float MovementValue)
{
	AddMovementInput(GetActorForwardVector() * MovementValue);
}

void ASCharacter::MoveRight(float MovementValue)
{
	AddMovementInput(GetActorRightVector() * MovementValue);
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
