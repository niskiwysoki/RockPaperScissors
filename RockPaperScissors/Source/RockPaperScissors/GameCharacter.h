// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCharacter.generated.h"

class AOrbitingActor;
class USpringArmComponent;
class UCameraComponent;
class AGameProjectile;
class AShootingTarget;

UCLASS()
class ROCKPAPERSCISSORS_API AGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGameCharacter();

	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return m_CurrentHealth; }

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return m_CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return m_FollowCamera; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Animations")
	bool IsAiming() const { return m_bIsAiming; }

	void SetIsAiming(bool val) { m_bIsAiming = val; }

	UFUNCTION(BlueprintPure, Category = "Animations")
	bool IsCrouching() const { return m_bIsCrouching; }

	void SetIsCrouching(bool val) { m_bIsCrouching = val; }

	UFUNCTION(BlueprintPure, Category = "Animations")
	bool IsLeapPressed() const { return m_bIsLeapPressed; }

	void SetIsLeapPressed(bool val) { m_bIsLeapPressed = val; }

	float GetBaseRotationRate() const { return m_BaseRotationRate; }
	UChildActorComponent* GetCollidingOrbitingSpheres() const { return m_CollidingOrbitingSpheres; }
	UChildActorComponent* GetVisibleOrbitingSpheres() const { return m_VisibleOrbitingSpheres; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float m_CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Aim();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopAiming();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Leap();

	void StopLeaping();

	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(bool isAiming);
	
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StartCrouching();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopCrouching();

	UFUNCTION(Server, Reliable)
	void Server_SetIsCrouching(bool IsCrouching);

	UFUNCTION(Server, Reliable)
	void Server_SetIsLeaping(bool IsJumping);

	void SpeedUpSpheres();
	void RestoreSpheresRotationSpeed();
	void ChangeRotationDirection();

	UFUNCTION(Server, Reliable)
	void Server_SetRotationRateOfSpheres(float rotationRate, bool changeDirection = false);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SpheresRotationChangerate(float rot);

	UFUNCTION(Server, Reliable)
	void HandleFire();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void AttachActorToMovingTarget();

	void GenerateMovingTarget();

	void BindDeleteSpheresDelegateIfNeeded();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnHealthUpdate();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void SetRotationRateOfVisSpheres(float rotationRate, bool changeDirection = false);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* m_CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* m_FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float m_BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AGameProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* m_WeaponActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* m_WeaponSK;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_FireRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_BoostedRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_BaseRotationRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	float m_InterpSpeed;

	float m_CurrentRotationRate;
	bool m_CurrentDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* m_CollidingOrbitingSpheres;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* m_VisibleOrbitingSpheres;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_MovingTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming", meta = (AllowPrivateAccess = "true"))
	float m_CrouchingMovementSlowRatio;
	
	UPROPERTY(EditDefaultsOnly, Category = "Aiming", meta = (AllowPrivateAccess = "true"))
	float m_AimingMovementSlowRatio;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming", meta = (AllowPrivateAccess = "true"))
	float m_FieldOfView;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming", meta = (AllowPrivateAccess = "true"))
	float m_FieldOfViewWhileAiming;

	FTimerHandle m_FiringTimer;
	FTimerHandle m_LeapTimer;
	FTimerDelegate TimerDel;

	UPROPERTY(Replicated)
	bool m_bIsAiming;

	UPROPERTY(Replicated)
	bool m_bIsCrouching;

	UPROPERTY(Replicated)
	bool m_bIsLeapPressed;
	
	bool m_bIsFiringWeapon;
	bool m_bDelegatesBound;
	bool m_bIsZooming;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<AShootingTarget> m_SubClassOfShootingTarger;

	AShootingTarget* m_ShootingTarget;
	
};
