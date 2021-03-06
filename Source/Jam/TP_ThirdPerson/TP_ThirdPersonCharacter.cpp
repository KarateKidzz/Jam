
#include "TP_ThirdPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "../ChainComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ATP_ThirdPersonCharacter

ATP_ThirdPersonCharacter::ATP_ThirdPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	TailSparksComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TailSparksComponent"));
	TailSparksComponent->SetupAttachment(GetMesh());
	TailSparksComponent->SetRelativeLocation(GetActorLocation() + FVector(0.0f, -150.0f, 170.0f));
	TailSparksComponent->bAutoActivate = true;
	TailSparksComponent->SetActive(true);

	// initialise it with the dash particles
	static ConstructorHelpers::FObjectFinder<UParticleSystem>
		TailSparksAsset(TEXT("ParticleSystem'/Game/Art/particleFX/othersparks.othersparks'"));
	if (TailSparksAsset.Succeeded())
	{
		TailSparksComponent->SetTemplate(TailSparksAsset.Object);
	}

	RunDustComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RunDustComponent"));
	RunDustComponent->SetupAttachment(RootComponent);
	RunDustComponent->bAutoActivate = false;
	RunDustComponent->SetActive(false);

	// initialise it with the dash particles
	static ConstructorHelpers::FObjectFinder<UParticleSystem>
		RunDustAsset(TEXT("ParticleSystem'/Game/Art/particleFX/RunDust.RunDust'"));
	if (RunDustAsset.Succeeded())
	{
		RunDustComponent->SetTemplate(RunDustAsset.Object);
	}

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATP_ThirdPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("ResetDustTimer", IE_Released, this, &ATP_ThirdPersonCharacter::ResetRunDustTimer);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATP_ThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATP_ThirdPersonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATP_ThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATP_ThirdPersonCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATP_ThirdPersonCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATP_ThirdPersonCharacter::TouchStopped);
}

void ATP_ThirdPersonCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ATP_ThirdPersonCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ATP_ThirdPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		if (!bTimerStarted)
		{
			ActivateFeetParticles();
		}
	}
}

void ATP_ThirdPersonCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);

		if (!bTimerStarted)
		{
			ActivateFeetParticles();
		}
	}
}

UChainComponent* ATP_ThirdPersonCharacter::CreateConnection(AActor* CollidedActor, FTransform Transform)
{
	if (bHasPickedUpCable)
	{
		// Detach current chain from this character and attach to the collided actor
		if (CurrentChain)
		{
			TotalChainLengthUsed += CurrentChain->GetOwner()->GetDistanceTo(CollidedActor);
			CurrentChain->CableLength = CurrentChain->GetOwner()->GetDistanceTo(CollidedActor);
			CurrentChain->SetAttachEndTo(CollidedActor, NAME_None);
			CurrentChain->EndLocation = Transform.GetLocation();
		}
		// New cable starts at collided actor and ends at the player
		UChainComponent* Chain = Cast<UChainComponent>(CollidedActor->AddComponentByClass(UChainComponent::StaticClass(), false, Transform, false));
		Chain->SetAttachEndTo(this, NAME_None);
		Chain->EndLocation = ConnectionLocation;
		Chains.Add(Chain);
		CurrentChain = Chain;
    
		if (OnCableConnectionAdded.IsBound())
		{
			OnCableConnectionAdded.Broadcast(this, CurrentChain);
		}

		if (TotalChainLengthUsed >= MaximumChainLength)
		{
			OnCableLengthExceeded.Broadcast();
			for (UChainComponent* ChainComponent : Chains)
			{
				if (ChainComponent)
				{
					ChainComponent->DestroyComponent();
				}
			}
			Chains.Empty();
			TotalChainLengthUsed = 0.0f;
			bHasPickedUpCable = false;
		}
	}
    
	return CurrentChain;
}

bool ATP_ThirdPersonCharacter::PluggedInSocket(AActor* CollidedActor, FTransform Transform)
{
	if (CurrentChain)
	{
		TotalChainLengthUsed += CurrentChain->GetOwner()->GetDistanceTo(CollidedActor);
		CurrentChain->CableLength = CurrentChain->GetOwner()->GetDistanceTo(CollidedActor);
		CurrentChain->SetAttachEndTo(CollidedActor, NAME_None);
		CurrentChain->EndLocation = Transform.GetLocation();
		CurrentChain = nullptr;

		if (OnCableConnectionAdded.IsBound())
		{
			OnCableConnectionAdded.Broadcast(this, CurrentChain);
		}
		return true;
	}
	return false;
}

void ATP_ThirdPersonCharacter::ActivateFeetParticles()
{
	int32 Index = FMath::RandRange(0, FeetLocations.Num() - 1);
	FVector Location = FeetLocations[Index];

	RunDustComponent->SetRelativeLocation(Location);
	RunDustComponent->SetActive(true);
	RunDustComponent->ActivateSystem(true);

	bTimerStarted = true;
	float InRate = FMath::RandRange(0.0f, 0.005f);
	GetWorld()->GetTimerManager().SetTimer(RunDustTimer, this, &ATP_ThirdPersonCharacter::ResetRunDustTimer, InRate, true, true);
}

void ATP_ThirdPersonCharacter::SetupRunDustTimer()
{
	bTimerStarted = true;

	float InRate = FMath::RandRange(0.0f, 0.5f);

	GetWorld()->GetTimerManager().SetTimer(RunDustTimer, this, &ATP_ThirdPersonCharacter::ActivateFeetParticles, InRate, true, true);
}

void ATP_ThirdPersonCharacter::ResetRunDustTimer()
{
	bTimerStarted = false;
	GetWorld()->GetTimerManager().ClearTimer(RunDustTimer);
}