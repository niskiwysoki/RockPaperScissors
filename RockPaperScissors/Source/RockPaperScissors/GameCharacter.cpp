// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCharacter.h"
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
#include "Kismet/KismetStringLibrary.h"
#include "TimerManager.h"
#include "GameFramework/RotatingMovementComponent.h"

// Sets default values
AGameCharacter::AGameCharacter()
{
	//bindings
	m_FireRate = 0.25f;
	m_bIsFiringWeapon = false;
	m_bIsAiming = false;
	m_bIsCrouching = false;
	m_bIsLeapPressed = false;

	//movement
	m_CrouchingMovementSlowRatio = 0.7;
	m_AimingMovementSlowRatio = 0.5;

	m_BaseTurnRate = 45.f;
	m_BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	m_bDelegateBound = false;
	
	//spheres
	m_CurrentDirection = true;
	m_BaseRotationRate = 80.f;
	m_CurrentRotationRate = m_BaseRotationRate;
	m_BoostedRotationRate = 160.f;
	m_InterpSpeed = 50.f;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 350.f;
	GetCharacterMovement()->AirControl = 0.3f;

	TimerDel.BindUFunction(this, FName("Server_SetIsLeaping"), false);

	//components
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	m_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	m_CameraBoom->SetupAttachment(RootComponent);
	m_CameraBoom->TargetArmLength = 600.0f; 
	m_CameraBoom->bUsePawnControlRotation = true; 

	m_FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	m_FollowCamera->SetupAttachment(m_CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	m_FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	m_FieldOfView = 90.f;
	m_FieldOfViewWhileAiming = 60.f;
	m_FollowCamera->SetFieldOfView(m_FieldOfView);

	m_CollidingOrbitingSpheres = CreateDefaultSubobject<UChildActorComponent>(TEXT("Orbiting Colliding Spheres"));
	m_CollidingOrbitingSpheres->SetupAttachment(RootComponent);
	m_CollidingOrbitingSpheres->SetAbsolute(false, true, false);		// Spheres rotating independent of actor's orientation

	m_VisibleOrbitingSpheres = CreateDefaultSubobject<UChildActorComponent>(TEXT("Orbiting Visible Spheres"));
	m_VisibleOrbitingSpheres->SetupAttachment(RootComponent);
	m_VisibleOrbitingSpheres->SetAbsolute(false, true, false);		// Spheres rotating independent of actor's orientation

	m_WeaponSK = CreateDefaultSubobject <USkeletalMeshComponent>(TEXT("WeaponSK"));
	m_WeaponSK->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

	PrimaryActorTick.bCanEverTick = true;
	MaxHealth = 100.0f;
	m_CurrentHealth = MaxHealth;
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	BindDeleteSpheresDelegateIfNeeded();
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::BindDeleteSpheresDelegateIfNeeded()
{
	if (m_bDelegateBound)
		return;

	if (AOrbitingActor* visibleSpheres = Cast<AOrbitingActor>(m_VisibleOrbitingSpheres->GetChildActor()))
	{
		if (AOrbitingActorCollisionState* collingspheres = Cast<AOrbitingActorCollisionState>(m_CollidingOrbitingSpheres->GetChildActor()))
		{
			collingspheres->deleteSpheresDelegate.BindUObject(visibleSpheres, &AOrbitingActor::DeleteSpheres);
			m_bDelegateBound = true;
		}
	}
}

/******************************************************************************************************************************************************************************************************/	
void AGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGameCharacter::Leap);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &AGameCharacter::StopLeaping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGameCharacter::StartFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGameCharacter::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGameCharacter::StopAiming);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AGameCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AGameCharacter::StopCrouching);

	PlayerInputComponent->BindAction("SpeedUpSpheres", IE_Pressed, this, &AGameCharacter::SpeedUpSpheres);
	PlayerInputComponent->BindAction("SpeedUpSpheres", IE_Released, this, &AGameCharacter::RestoreSpheresRotationSpeed);
	PlayerInputComponent->BindAction("ChangeRotationDirection", IE_Pressed, this, &AGameCharacter::ChangeRotationDirection);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGameCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGameCharacter::LookUpAtRate);

}
/******************************************************************************************************************************************************************************************************/
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
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::StopFire()
{
	m_bIsFiringWeapon = false;
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Aim()
{
	Server_SetIsAiming(true);
	//SetActorTickEnabled(true);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::StopAiming()
{
	Server_SetIsAiming(false);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Leap()
{
	if (!IsLeapPressed() && !IsCrouching())
	{
		Jump();
		Server_SetIsLeaping(true);
	}
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Server_SetIsAiming_Implementation(bool isAiming)
{
	m_bIsAiming = isAiming;
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::StartCrouching()
{
	Server_SetIsCrouching(true);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::StopCrouching()
{
	Server_SetIsCrouching(false);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::SpeedUpSpheres()
{
	Server_SetRotationRateOfSpheres(m_BoostedRotationRate);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::RestoreSpheresRotationSpeed()
{
	Server_SetRotationRateOfSpheres(m_BaseRotationRate);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::ChangeRotationDirection()
{
	Server_SetRotationRateOfSpheres(m_CurrentRotationRate, true);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::NetMulticast_SpheresRotationChangerate_Implementation(float rot)
{
	AOrbitingActorCollisionState* orbitingActor = Cast<AOrbitingActorCollisionState>(m_CollidingOrbitingSpheres->GetChildActor());
	if (orbitingActor)
		orbitingActor->GetRotatingMovementComp()->RotationRate.Yaw = rot;
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Server_SetRotationRateOfSpheres_Implementation(float rotationRate, bool changeDirection = false)
{
	AOrbitingActorCollisionState* orbitingActor = Cast<AOrbitingActorCollisionState>(m_CollidingOrbitingSpheres->GetChildActor());
	if (orbitingActor)
	{
		m_CurrentRotationRate = rotationRate;
		if (changeDirection)
			m_CurrentDirection = !m_CurrentDirection;
		if (m_CurrentDirection)
			orbitingActor->GetRotatingMovementComp()->RotationRate.SetComponentForAxis(EAxis::Z, m_CurrentRotationRate );
		else
		{
			orbitingActor->GetRotatingMovementComp()->RotationRate.SetComponentForAxis(EAxis::Z, m_CurrentRotationRate * (-1));
		}
		NetMulticast_SpheresRotationChangerate(orbitingActor->GetRotatingMovementComp()->RotationRate.Yaw);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Cast to orbitingActor failed"));
	}
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Server_SetIsLeaping_Implementation(bool IsJumpPressed)
{	
	m_bIsLeapPressed = IsJumpPressed;
	GetWorld()->GetTimerManager().SetTimer(m_LeapTimer, TimerDel, 0.7f, false);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Server_SetIsCrouching_Implementation(bool IsCrouching)
{
	m_bIsCrouching = IsCrouching;
	if (IsCrouching)
		Crouch();
	else
		UnCrouch();
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector()  * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	AGameProjectile* spawnedProjectile = GetWorld()->SpawnActor<AGameProjectile>(ProjectileClass, spawnLocation, spawnRotation, spawnParameters);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * m_BaseTurnRate * GetWorld()->GetDeltaSeconds());
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * m_BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::SetRotationRateOfSpheres(float rotationRate)
{
	AOrbitingActorCollisionState* orbitingActor = Cast<AOrbitingActorCollisionState>(m_CollidingOrbitingSpheres->GetChildActor());
	if (orbitingActor)
	{
		orbitingActor->GetRotatingMovementComp()->RotationRate.SetComponentForAxis(EAxis::Z, rotationRate);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Cast to orbitingActor failed"));
	}
}
/******************************************************************************************************************************************************************************************************/
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
/******************************************************************************************************************************************************************************************************/
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
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AGameCharacter, m_CurrentHealth);
	DOREPLIFETIME(AGameCharacter, m_bIsAiming);
	DOREPLIFETIME(AGameCharacter, m_bIsCrouching);
	DOREPLIFETIME(AGameCharacter, m_bIsLeapPressed);
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::SetCurrentHealth(float healthValue)
{
	if (HasAuthority())
	{
		m_CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}
/******************************************************************************************************************************************************************************************************/
float AGameCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = m_CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), m_CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (m_CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (HasAuthority())
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), m_CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}
/******************************************************************************************************************************************************************************************************/
void AGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BindDeleteSpheresDelegateIfNeeded();

	if (IsAiming())
	{
		m_FollowCamera->FieldOfView -= (m_FieldOfView - m_FieldOfViewWhileAiming) * DeltaTime * 5;
		m_FollowCamera->FieldOfView = FMath::Clamp(m_FollowCamera->FieldOfView, m_FieldOfViewWhileAiming, m_FieldOfView);
	}
	else
	{
		m_FollowCamera->FieldOfView += (m_FieldOfView - m_FieldOfViewWhileAiming) * DeltaTime *5;
		m_FollowCamera->FieldOfView = FMath::Clamp(m_FollowCamera->FieldOfView, m_FieldOfViewWhileAiming, m_FieldOfView);
		if (FMath::IsNearlyZero(m_FieldOfView - m_FollowCamera->FieldOfView, 1.f))
		{
			bUseControllerRotationYaw = false;
		}
	}

	AOrbitingActor* visibleOrbitingActor = Cast<AOrbitingActor>(m_VisibleOrbitingSpheres->GetChildActor());
	AOrbitingActorCollisionState* collidingOrbitngActor = Cast<AOrbitingActorCollisionState>(m_CollidingOrbitingSpheres->GetChildActor());
	if (visibleOrbitingActor && collidingOrbitngActor)
	{
		
		FRotator newRot = FMath::RInterpTo(visibleOrbitingActor->GetActorRotation(), collidingOrbitngActor->GetActorRotation(), DeltaTime, m_InterpSpeed);
		visibleOrbitingActor->SetActorRotation(newRot);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Failed cast to Orbiting actor "));
	}
	
	//if(HasAuthority())
	if(collidingOrbitngActor)
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Rot ") + collidingOrbitngActor->GetRotatingMovementComp()->RotationRate.ToString());
	//else	
	//	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("Client ") + UKismetStringLibrary::Conv_BoolToString(m_bIsAiming));
}


