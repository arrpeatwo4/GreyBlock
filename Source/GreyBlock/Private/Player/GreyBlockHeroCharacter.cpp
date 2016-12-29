// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "UsableActor.h"
#include "BaseWeapon.h"
#include "STypes.h"
#include "GreyBlockHeroCharacter.h"


AGreyBlockHeroCharacter::AGreyBlockHeroCharacter()
{

}

AGreyBlockHeroCharacter::AGreyBlockHeroCharacter(const class FObjectInitializer & ObjectInitializer) : Super(ObjectInitializer)
{
	// set this character to tick every frame
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Character movement adjustments for Z direction
	UCharacterMovementComponent * MoveComp = GetCharacterMovement();
	MoveComp->GravityScale = 1.5f;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	/* Ignore this channel or it will absorb the trace impacts instead of the skelatal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

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
	//GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoomComp = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoomComp->SocketOffset = FVector(0, 35, 0);
	CameraBoomComp->TargetOffset = FVector(0, 0, 55);
	CameraBoomComp->SetupAttachment(RootComponent);
	CameraBoomComp->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoomComp->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	CameraComp = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	CameraComp->SetupAttachment(CameraBoomComp, USpringArmComponent::SocketName);
	// Camera does not rotate relative to arm
	CameraComp->bUsePawnControlRotation = false;

	MaxUseDistance = 500;
	DropWeaponMaxDistance = 100;
	bHasNewFocus = true;
	TargetingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.5f;

	Health = 100;

	WeaponAttachPoint = TEXT("WeaponSocket");
	PelvisAttachPoint = TEXT("PelvisSocket");
	SpineAttachPoint = TEXT("SpineSocket");

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)		
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

}


//////////////////////////////////////////////////////////////////////////
// Input

void AGreyBlockHeroCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);

	// Crouch - boolean toggle
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AGreyBlockHeroCharacter::EnterCrouch);

	// Switch Weapon - increments forward, max of 5 and resets
	InputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AGreyBlockHeroCharacter::OnNextWeapon);

	// Attack - when pressed once, no repeated attacking
	InputComponent->BindAction("Attack", IE_Pressed, this, &AGreyBlockHeroCharacter::OnAttack);
	InputComponent->BindAction("Attack", IE_Released, this, &AGreyBlockHeroCharacter::OnEndAttack);

	// Sprint - sprint for as long as button is held
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AGreyBlockHeroCharacter::OnSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AGreyBlockHeroCharacter::OnStopSprinting);

	// Aim Pistol - aim for as long as button is pressed
	InputComponent->BindAction("Aim", IE_Pressed, this, &AGreyBlockHeroCharacter::AimPistol);
	InputComponent->BindAction("Aim", IE_Released, this, &AGreyBlockHeroCharacter::StopAimingPistol);

	// Reload - play once on press
	InputComponent->BindAction("Reload", IE_Pressed, this, &AGreyBlockHeroCharacter::OnReload);

	// Interract - play once on press
	InputComponent->BindAction("Interact", IE_Pressed, this, &AGreyBlockHeroCharacter::Interact);

	// Lure - play once on press
	InputComponent->BindAction("Lure", IE_Pressed, this, &AGreyBlockHeroCharacter::Lure);

	InputComponent->BindAxis("MoveForward", this, &AGreyBlockHeroCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGreyBlockHeroCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AGreyBlockHeroCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AGreyBlockHeroCharacter::LookUpAtRate);
}


void AGreyBlockHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AGreyBlockHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AGreyBlockHeroCharacter::MakePawnNoise(float Loudness)
{
	MakeNoise(Loudness, this, GetActorLocation());

	LastNoiseLoudness = Loudness;
	LastMakeNoiseTime = GetWorld()->GetTimeSeconds();
}


float AGreyBlockHeroCharacter::GetLastNoiseLoudness()
{
	return LastNoiseLoudness;
}


float AGreyBlockHeroCharacter::GetLastMakeNoiseTime()
{
	return LastMakeNoiseTime;
}

void AGreyBlockHeroCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGreyBlockHeroCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGreyBlockHeroCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AGreyBlockHeroCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		//const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


AUsableActor * AGreyBlockHeroCharacter::GetUsableInView()
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr)
	{
		return nullptr;
	}

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	/* Not tracing complex uses the rough collision instead making tiny objects easier to detect */
	TraceParams.bTraceComplex = false;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);

	return Cast<AUsableActor>(Hit.GetActor());
}


// Callback implementation for crouching
void AGreyBlockHeroCharacter::EnterCrouch()
{
	
	if (IsSprinting())
	{
		SetSprinting(false);
	}	

	// If we are crouching the CanCrouch will return false, resulting in Crouch not doing anything.
	// Engine crouching is screwing with the camera height, making it kind of janky, we're going to bypass,
	// and use our own implentation until we figure the camera crap
	if (CanCrouch())
	{
		Crouch();
	}
	else
	{
		UnCrouch();		
	}
}


// Callback implementation for attack requests
void AGreyBlockHeroCharacter::OnAttack()
{
	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	if (CurrentWeapon)
	{
		// Get the currently equipped weapon
		EWeaponType ECurrentWeaponType = CurrentWeapon->GetWeaponType();
	
		if ((ECurrentWeaponType == EWeaponType::Pistol) || (ECurrentWeaponType == EWeaponType::Taser))
		{
			// We don't shoot from the hip
			if (bIsTargeting) 
			{
				// Fire le pistola
				StartWeaponFire();
			}		
		}
		else if (ECurrentWeaponType == EWeaponType::Unarmed)
		{
			// Assassination!!!
			StartWeaponFire();
		}
	}
}


FName AGreyBlockHeroCharacter::GetInventoryAttachPoint(EInventorySlot Slot) const
{
	/* Return the socket name for the specified storage slot */
	switch (Slot)
	{
	case EInventorySlot::Hands:
		return WeaponAttachPoint;
	case EInventorySlot::Primary:
		return SpineAttachPoint;
	case EInventorySlot::Secondary:
		return PelvisAttachPoint;
	default:
		// Not implemented
		return "";
	}
}


void AGreyBlockHeroCharacter::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
				
		// Call the currently equipped weapon's doAttack() method			
		CurrentWeapon->StartFire();		
	}
}


void AGreyBlockHeroCharacter::OnEndAttack()
{
	StopWeaponFire();

}


void AGreyBlockHeroCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{			
			CurrentWeapon->StopFire();
		}
	}
}


// Callback implementation for Aim Pistol requests
void AGreyBlockHeroCharacter::AimPistol()
{
	// set the isAiming state control variable
	if (bIsTargeting == false)
	{
		if ((CurrentWeapon->GetWeaponType() == EWeaponType::Pistol) || (CurrentWeapon->GetWeaponType() == EWeaponType::Taser))
		{
			// fix camera rotation to mouse input
			bUseControllerRotationYaw = true;

			bIsTargeting = true;

			// Set targeting speed
			GetCharacterMovement()->MaxWalkSpeed = TargetingSpeed;
		}
	}
}


// Callback implementation to stop aiming pistol - when button is released.
void AGreyBlockHeroCharacter::StopAimingPistol()
{
	// Release the camera rotation yaw
	bUseControllerRotationYaw = false;
	
	bIsTargeting = false;

	// Return to default speed
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
}


// public accessor for is aiming variable to drive state machine until one is developed
bool AGreyBlockHeroCharacter::GetIsTargeting() const
{
	return bIsTargeting;
}

// Callback implementation for Reload requests
void AGreyBlockHeroCharacter::OnReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}


void AGreyBlockHeroCharacter::SetSprinting(bool NewSprinting)
{
	
	Super::SetSprinting(NewSprinting);
}


// Callback implementation for Sprint requests
void AGreyBlockHeroCharacter::OnSprint()
{
	// Update sprinting boolean, increase walk speed
	if ((bIsSprinting == false) && (bIsCrouching == false))
	{
		SetSprinting(true);
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		
	}
}


// Callback implementation to stop sprinting - when button is released.
void AGreyBlockHeroCharacter::OnStopSprinting()
{
	// reset max walk speed
	// unset sprinting boolean
	SetSprinting(false);
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
}


// Callback implementation for Interract requests
void AGreyBlockHeroCharacter::Interact()
{
	// Implement usable actor from survival game
}


// Callback implementation for Lure requests
void AGreyBlockHeroCharacter::Lure()
{
	// can do the similar to interact here, 
}


void AGreyBlockHeroCharacter::EquipWeapon(ABaseWeapon* Weapon)
{
	if (Weapon)
	{
		/* Ignore if trying to equip already equipped weapon */
		if (Weapon == CurrentWeapon)
		{
			return;
		}
		else
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}		
	}
}


// Callback to switch weapons
void AGreyBlockHeroCharacter::OnNextWeapon()
{
	//if (CarriedObjectComp->GetIsCarryingActor())
	//{
	//	CarriedObjectComp->Rotate(0.0f, 1.0f);
	//	return;
	//}

	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		ABaseWeapon* NextWeapon = Inventory[(CurrentWeaponIndex + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}

}


void AGreyBlockHeroCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EInventorySlot::Hands);
	}
}


void AGreyBlockHeroCharacter::SetCurrentWeapon(class ABaseWeapon* NewWeapon, class ABaseWeapon* LastWeapon)
{
	/* Maintain a reference for visual weapon swapping */
	PreviousWeapon = LastWeapon;	

	/*	Uncomment this to program in the animation swapping of weapons	*/
	ABaseWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// UnEquip the current
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	CurrentWeapon = NewWeapon;	

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		/* Only play equip animation when we already hold an item in hands */
		NewWeapon->OnEquip(bHasPreviousWeapon);
	}

	/* NOTE: If you don't have an equip animation w/ animnotify to swap the meshes halfway through, then uncomment this to immediately swap instead */
	SwapToNewWeaponMesh();
}


// Allow the capsule to overlap the enemy player capsule
// Maybe this should be handled at the Enemy Character level?
void AGreyBlockHeroCharacter::OverrideCharacterPosition(FVector NewLocation, FRotator NewRotation, FVector NewForwardVec)
{
	FVector newNormFVec, newNormRVec;

	float ForwardVecMag = -119.0f;
	float RightVecMag = -5.0f;

	FHitResult * SweepResult = nullptr;

	newNormFVec = NewForwardVec * ForwardVecMag;
	NewLocation = NewLocation + newNormFVec;

	// Apply forward vector & rotation correction
	SetActorLocation(NewLocation, true, nullptr);
	SetActorRotation(NewRotation);

	newNormRVec = GetActorRightVector() * RightVecMag;
	NewLocation = GetActorLocation() + newNormRVec;
	
	// Apply right vector correction
	SetActorLocation(NewLocation, true, nullptr);
}


void AGreyBlockHeroCharacter::UnsetMeleeCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}


ABaseWeapon * AGreyBlockHeroCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}


bool AGreyBlockHeroCharacter::CanFire() const
{
	return IsAlive();
}


bool AGreyBlockHeroCharacter::CanReload() const
{
	return IsAlive();
}


bool AGreyBlockHeroCharacter::IsFiring() const
{
	return CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing;
}


bool AGreyBlockHeroCharacter::IsMelee() const
{
	return CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing;
}


void AGreyBlockHeroCharacter::AddWeapon(class ABaseWeapon * Weapon)
{
	if (Weapon)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
	}
}

void AGreyBlockHeroCharacter::SetCameraControl(bool bCameraControl)
{
	CameraBoomComp->bUsePawnControlRotation = bCameraControl;
}