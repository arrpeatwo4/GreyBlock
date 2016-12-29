// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "BaseWeapon.h"
#include "STypes.h"
#include "GBPlayerController.h"
#include "BaseCharacter.h"


// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

ABaseWeapon::ABaseWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = Mesh;

	bIsEquipped = false;
	CurrentState = EWeaponState::Idle;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;

	MuzzleAttachPoint = TEXT("MuzzleFlashSocket");
	StorageSlot = EInventorySlot::Primary;

	ShotsPerMinute = 700;
	StartAmmo = 999;
	MaxAmmo = 999;
	MaxAmmoPerClip = 30;
	NoAnimReloadDuration = 1.5f;
	NoEquipAnimDuration = 0.5f;
}


void ABaseWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/* Setup configuration */
	TimeBetweenShots = 60.0f / ShotsPerMinute;
	CurrentAmmo = FMath::Min(StartAmmo, MaxAmmo);
	CurrentAmmoInClip = FMath::Min(MaxAmmoPerClip, StartAmmo);
}


void ABaseWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DetachMeshFromPawn();
	StopSimulatingWeaponFire();
}


void ABaseWeapon::SetOwningPawn(AGreyBlockHeroCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// Net owner for RPC calls.  RJP - Is this needed?
		SetOwner(NewOwner);
	}
}


void ABaseWeapon::AttachMeshToPawn(EInventorySlot Slot)
{
	if (MyPawn)
	{
		// Remove and hide
		DetachMeshFromPawn();

		USkeletalMeshComponent* PawnMesh = MyPawn->GetMesh();
		FName AttachPoint = MyPawn->GetInventoryAttachPoint(Slot);
		Mesh->SetHiddenInGame(false);
		Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
	}
}


void ABaseWeapon::DetachMeshFromPawn()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetHiddenInGame(true);
}


void ABaseWeapon::OnEquip(bool bPlayAnimation)
{
	bPendingEquip = true;
	DetermineWeaponState();

	if (bPlayAnimation)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if (Duration <= 0.0f)
		{
			// Failsafe in case animation is missing
			Duration = NoEquipAnimDuration;
		}
		EquipStartedTime = GetWorld()->TimeSeconds;
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(EquipFinishedTimerHandle, this, &ABaseWeapon::OnEquipFinished, Duration, false);
	}
	else
	{
		/* Immediately finish equipping */
		OnEquipFinished();
	}

	if (MyPawn)
	{
		PlayWeaponSound(EquipSound);
	}
}


void ABaseWeapon::OnUnEquip()
{
	bIsEquipped = false;
	StopFire();

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(EquipFinishedTimerHandle);
	}

	DetermineWeaponState();
}


void ABaseWeapon::OnEnterInventory(AGreyBlockHeroCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn(StorageSlot);
}


void ABaseWeapon::OnLeaveInventory()
{
	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}

	DetachMeshFromPawn();
}


void ABaseWeapon::StartFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}


void ABaseWeapon::StopFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}


void ABaseWeapon::StartMelee(FVector TargetLocation, FRotator TargetRotation, FVector TargetForwardVec)
{
	if (!bPlayingMeleeMontage)
	{
		bPlayingMeleeMontage = true;
		
		float AnimDuration = PlayWeaponAnimation(MeleeMontage);

		if (AnimDuration <= 0.0f)
		{
			AnimDuration = NoMeleeAnimDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopMelee, this, &ABaseWeapon::StopMelee, AnimDuration, false);
				
		MyPawn->OverrideCharacterPosition(TargetLocation, TargetRotation, TargetForwardVec);
		
	}
}


void ABaseWeapon::StopMelee()
{
	bPlayingMeleeMontage = false;
	StopWeaponAnimation(MeleeMontage);
}


bool ABaseWeapon::CanFire() const
{
	bool bPawnCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOK = CurrentState == EWeaponState::Idle || CurrentState == EWeaponState::Firing;
	return bPawnCanFire && bStateOK && !bPendingReload;
}


FVector ABaseWeapon::GetAdjustedAim() const
{
	
	AGBPlayerController * const PC = Instigator ? Cast<AGBPlayerController>(Instigator->Controller) : nullptr;
	FVector FinalAim = FVector::ZeroVector;
	
	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		FinalAim = CamRot.Vector();
	}
	else if (Instigator)
	{
		FinalAim = Instigator->GetBaseAimRotation().Vector();
	}
	
	return FinalAim;
}


FVector ABaseWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AGBPlayerController * PC = MyPawn ? Cast<AGBPlayerController>(MyPawn->Controller) : nullptr;
	
	FVector OutStartTrace = FVector::ZeroVector;
	
	if (PC)
	{
		FRotator DummyRot;
		PC->GetPlayerViewPoint(OutStartTrace, DummyRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * (FVector::DotProduct((Instigator->GetActorLocation() - OutStartTrace), AimDir));
	}
	
	return OutStartTrace;
}


FHitResult ABaseWeapon::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const
{
	FCollisionQueryParams TraceParams(TEXT("WeaponTrace"), true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}



void ABaseWeapon::HandleFiring()
{
	//if (CurrentAmmoInClip > 0 && CanFire())
	//{
		if (MyPawn)
		{
			if (MyPawn->IsTargeting())
			{
				// Plays the montage, etc.
				SimulateWeaponFire();

				/*
				*	This is where the weapon is fired,
				*	because it's a pure virtual, it is called in weapon instant.
				*/
				FireWeapon();

				UseAmmo();

				// Update firing FX on remote clients if this is called on server
				BurstCounter++;
			}			
		}
	//}
	//else if (CanReload())
	//{
	//	StartReload();
	//}
	//else if (MyPawn)
	//{
	//	if (GetCurrentAmmo() == 0 && !bRefiring)
	//	{
	//		PlayWeaponSound(OutOfAmmoSound);
	//	}

		/* Reload after firing last round */
	//	if (CurrentAmmoInClip <= 0 && CanReload())
	//	{
	//		StartReload();
	//	}

		/* Stop weapon fire FX, but stay in firing state */
	//	if (BurstCounter > 0)
	//	{
	//		OnBurstFinished();
	///	}
	//}

	if (MyPawn)
	{
		/* Retrigger HandleFiring on a delay for automatic weapons */
		bRefiring = (CurrentState == EWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ABaseWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	/* Make Noise on every shot. The data is managed by the PawnNoiseEmitterComponent created in SBaseCharacter and used by PawnSensingComponent in SZombieCharacter */
	// May push this to the pawn.
	if (MyPawn)
	{
		MyPawn->MakePawnNoise(1.0f);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}


void ABaseWeapon::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	}

	if (!bPlayingFireAnim)
	{
	//	PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	PlayWeaponSound(FireSound);

	StopSimulatingWeaponFire();
}


void ABaseWeapon::StopSimulatingWeaponFire()
{
	if (bPlayingFireAnim)
	{		
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}
}


UAudioComponent* ABaseWeapon::PlayWeaponSound(USoundCue* SoundToPlay)
{
	UAudioComponent* AC = nullptr;
	if (SoundToPlay && MyPawn)
	{
		AC = UGameplayStatics::PlaySoundAttached(SoundToPlay, MyPawn->GetRootComponent());
	}

	return AC;
}


void ABaseWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		if (WeaponType == EWeaponType::Unarmed)
		{
			
		}
		else
		{
			OnBurstFinished();
		}
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		if (WeaponType == EWeaponType::Unarmed)
		{
			FireWeapon();
		}
		else
		{
			OnBurstStarted();
		}		
	}
}


void ABaseWeapon::OnBurstStarted()
{
	// Start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f &&
		LastFireTime + TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ABaseWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}


void ABaseWeapon::OnBurstFinished()
{
	BurstCounter = 0;

	//if (GetNetMode() != NM_DedicatedServer)
	//{
	//	StopSimulatingWeaponFire();
	//}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}


void ABaseWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		// Uncomment for reload logic integration

		if (bPendingReload)
		{
			if (CanReload())
			{
				NewState = EWeaponState::Reloading;
			}
			else
			{
				NewState = CurrentState;
			}
		}
		//if (!bPendingReload && bWantsToFire && CanFire())
		if (bWantsToFire)
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}


float ABaseWeapon::PlayWeaponAnimation(UAnimMontage* Animation, float InPlayRate, FName StartSectionName)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		if (Animation)
		{
			Duration = MyPawn->PlayAnimMontage(Animation, InPlayRate, StartSectionName);
		}
	}

	return Duration;
}


void ABaseWeapon::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (MyPawn)
	{
		if (Animation)
		{
			MyPawn->StopAnimMontage(Animation);
		}
	}
}


void ABaseWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	if (MyPawn)
	{
		// Try to reload empty clip
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}
	}
}


void ABaseWeapon::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;
}


int32 ABaseWeapon::GiveAmmo(int32 AddAmount)
{
	const int32 MissingAmmo = FMath::Max(0, MaxAmmo - CurrentAmmo);
	AddAmount = FMath::Min(AddAmount, MissingAmmo);
	CurrentAmmo += AddAmount;

	/* Push reload request to client */
	if (GetCurrentAmmoInClip() <= 0 && CanReload() &&
		MyPawn->GetCurrentWeapon() == this)
	{
		//ClientStartReload();
	}

	/* Return the unused ammo when weapon is filled up */
	return FMath::Max(0, AddAmount - MissingAmmo);
}


void ABaseWeapon::SetAmmoCount(int32 NewTotalAmount)
{
	CurrentAmmo = FMath::Min(MaxAmmo, NewTotalAmount);
	CurrentAmmoInClip = FMath::Min(MaxAmmoPerClip, CurrentAmmo);
}


void ABaseWeapon::StartReload(bool bFromReplication)
{
	/* If local execute requested or we are running on the server */
	if (CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ABaseWeapon::StopSimulateReload, AnimDuration, false);
		
		if (MyPawn)
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}


void ABaseWeapon::StopSimulateReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}


void ABaseWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(MaxAmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}
}


bool ABaseWeapon::CanReload()
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = (CurrentAmmoInClip < MaxAmmoPerClip) && ((CurrentAmmo - CurrentAmmoInClip) > 0);
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	//return (bCanReload && bGotAmmo && bStateOKToReload);
	return true;
}


/*
* Public Getters
*/
USkeletalMeshComponent* ABaseWeapon::GetWeaponMesh() const
{
	return Mesh;
}


class ABaseCharacter* ABaseWeapon::GetPawnOwner() const
{
	return MyPawn;
}


bool ABaseWeapon::IsEquipped() const
{
	return bIsEquipped;
}


bool ABaseWeapon::IsAttachedToPawn() const // TODO: Review name to more accurately specify meaning.
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState ABaseWeapon::GetCurrentState() const
{
	return CurrentState;
}


FVector ABaseWeapon::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}


FVector ABaseWeapon::GetMuzzleDirection() const
{
	return Mesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}


float ABaseWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}


float ABaseWeapon::GetEquipDuration() const
{
	return EquipDuration;
}


int32 ABaseWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}


int32 ABaseWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}


int32 ABaseWeapon::GetMaxAmmoPerClip() const
{
	return MaxAmmoPerClip;
}


int32 ABaseWeapon::GetMaxAmmo() const
{
	return MaxAmmo;
}


/*
void ABaseWeapon::OnRep_MyPawn()
{
if (MyPawn)
{
OnEnterInventory(MyPawn);
}
else
{
OnLeaveInventory();

}
}
*/

/*
void ABaseWeapon::OnRep_BurstCounter()
{
if (BurstCounter > 0)
{
SimulateWeaponFire();
}
else
{
StopSimulatingWeaponFire();
}
}
*/

/*
void ABaseWeapon::OnRep_Reload()
{
if (bPendingReload)
{
// By passing true we do not push back to server and execute it locally
StartReload(true);
}
else
{
StopSimulateReload();
}
}
*/