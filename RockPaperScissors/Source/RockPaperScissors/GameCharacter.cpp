// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GameProjectile.h"
#include "OrbitingActor.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AGameCharacter::AGameCharacter()
{
	
	//bindings
	m_FireRate = 0.25f;
	m_bIsFiringWeapon = false;
	m_bIsAiming = false;
	m_bIsCrouching = false;

	m_BaseTurnRate = 45.f;
	m_BaseLookUpRate = 45.f;

	m_CrouchingMovementSlowRatio = 0.7;
	m_AimingMovementSlowRatio = 0.5;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//components
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	m_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	m_CameraBoom->SetupAttachment(RootComponent);
	m_CameraBoom->TargetArmLength = 500.0f; 
	m_CameraBoom->bUsePawnControlRotation = true; 

	m_FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	m_FollowCamera->SetupAttachment(m_CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	m_FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	m_FieldOfView = 90.f;
	m_FieldOfViewWhileAiming = 60.f;
	m_FollowCamera->SetFieldOfView(m_FieldOfView);

	m_OrbitingSpheres = CreateDefaultSubobject<UChildActorComponent>(TEXT("Orbiting Spheres"));
	m_OrbitingSpheres->SetupAttachment(RootComponent);
	m_OrbitingSpheres->SetAbsolute(false, true, false);		// Spheres rotating independent of actor's orientation

	m_WeaponSK = CreateDefaultSubobject <USkeletalMeshComponent>(TEXT("WeaponSK"));
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	m_WeaponSK->AttachToComponent(GetMesh(), AttachmentTransformRules, "WeaponSocket");

	ProjectileClass = AGameProjectile::StaticClass();


	PrimaryActorTick.bCanEverTick = true;
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	
}

void AGameCharacter::BeginPlay()
{
	Super::BeginPlay();

}
void AGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGameCharacter::StartFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGameCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGameCharacter::StopAiming);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AGameCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AGameCharacter::StopCrouching);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGameCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGameCharacter::LookUpAtRate);

}

void AGameCharacter::StartFire()
{
	if (!m_bIsFiringWeapon)
	{
		m_bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(m_FiringTimer, this, &AGameCharacter::StopFire, m_FireRate, false);
		HandleFire();
	}
}

void AGameCharacter::StopFire()
{
	m_bIsFiringWeapon = false;
}

void AGameCharacter::Aim()
{
	m_bIsAiming = true;
	SetActorTickEnabled(true);
}

void AGameCharacter::StopAiming()
{
	m_bIsAiming = false;
}

void AGameCharacter::StartCrouching()
{
	m_bIsCrouching = true;
	Crouch();
}

void AGameCharacter::StopCrouching()
{
	m_bIsCrouching = false;
	UnCrouch();
}

void AGameCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector()  * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	AGameProjectile* spawnedProjectile = GetWorld()->SpawnActor<AGameProjectile>(ProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	
}

void AGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * m_BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * m_BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGameCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (IsCrouching())
			Value *= m_CrouchingMovementSlowRatio;
		if (IsAiming())
			Value *= m_AimingMovementSlowRatio;

		AddMovementInput(Direction, Value);
	}
}

void AGameCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		if (IsCrouching())
			Value *= m_CrouchingMovementSlowRatio;
		if (IsAiming())
			Value *= m_AimingMovementSlowRatio;
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AGameCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AGameCharacter, CurrentHealth);
}

void AGameCharacter::SetCurrentHealth(float healthValue)
{
	if (HasAuthority())
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AGameCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}


void AGameCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (HasAuthority())
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

void AGameCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

// Called every frame
void AGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAiming())
	{
		m_FollowCamera->FieldOfView -= (m_FieldOfView - m_FieldOfViewWhileAiming) * DeltaTime * 5;
		m_FollowCamera->FieldOfView = FMath::Clamp(m_FollowCamera->FieldOfView, m_FieldOfViewWhileAiming, m_FieldOfView);
		
		float DeltaRotation = (GetControlRotation().Yaw - GetMesh()->GetForwardVector().Rotation().Yaw) * DeltaTime;
		float newYaw = GetMesh()->GetRelativeRotation().Yaw + DeltaRotation;
		//SetActorRotation(FRotator(0.f,GetActorRotation().Yaw + DeltaRotation,0.f));
		GetMesh()->SetRelativeRotation(FRotator(0, newYaw, 0));


		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::SanitizeFloat(GetActorRotation().Yaw));
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::SanitizeFloat(GetControlRotation().Yaw));
		//bUseControllerRotationYaw = true;
	}
	else
	{
		m_FollowCamera->FieldOfView += (m_FieldOfView - m_FieldOfViewWhileAiming) * DeltaTime *5;
		m_FollowCamera->FieldOfView = FMath::Clamp(m_FollowCamera->FieldOfView, m_FieldOfViewWhileAiming, m_FieldOfView);
		if (FMath::IsNearlyZero(m_FieldOfView - m_FollowCamera->FieldOfView, 1.f))
		{
			SetActorTickEnabled(false);
			bUseControllerRotationYaw = false;
		}
	}

}


