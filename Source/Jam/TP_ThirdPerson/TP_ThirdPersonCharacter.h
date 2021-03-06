
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TP_ThirdPersonCharacter.generated.h"

class UChainComponent;
class UParticleSystemComponent;
class ATP_ThirdPersonCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCableConnectionAdded, ATP_ThirdPersonCharacter*, Character, UChainComponent*, Cable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableLengthExceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelFinished);

UCLASS(config=Game)
class ATP_ThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	ATP_ThirdPersonCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool LevelFinished = false;

	UPROPERTY(BlueprintAssignable)
	FOnLevelFinished OnLevelFinished;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chain")
	TArray<UChainComponent*> Chains;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chain")
	UChainComponent* CurrentChain;

	UFUNCTION(BlueprintCallable, Category = "Chain")
	UChainComponent* CreateConnection(AActor* CollidedActor, FTransform Transform);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chain")
	float MaximumChainLength = 200000.0f;

	UFUNCTION(BlueprintCallable, Category = "Chain")
	bool PluggedInSocket(AActor* CollidedActor, FTransform Transform);

	UFUNCTION(BlueprintCallable, Category = "Chain")
	float GetTotalChainLengthUsed() { return TotalChainLengthUsed;}

	UFUNCTION(BlueprintCallable, Category = "Chain")
	void ResetTotalChainLengthUsed() { TotalChainLengthUsed = 0.0f; }

	UPROPERTY(BlueprintReadWrite, Category = "Chain")
	FVector ConnectionLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Chain")
	bool bHasPickedUpCable = true;

    UPROPERTY(BlueprintAssignable)
    FOnCableConnectionAdded OnCableConnectionAdded;

	UPROPERTY(BlueprintAssignable, Category = "Chain")
	FOnCableLengthExceeded OnCableLengthExceeded;

public:
	UPROPERTY(VisibleAnywhere, Category = "Particles")
	UParticleSystemComponent* TailSparksComponent;

	UPROPERTY(VisibleAnywhere, Category = "Particles")
	UParticleSystemComponent* RunDustComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	TArray<FVector> FeetLocations;

	UPROPERTY()
	FTimerHandle RunDustTimer;

	bool bTimerStarted = false;

	UFUNCTION()
	void ActivateFeetParticles();

	UFUNCTION()
	void SetupRunDustTimer();

	UFUNCTION()
	void ResetRunDustTimer();

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalChainLengthUsed = 0.0f;

	FVector LastConnectionPosition;
};

