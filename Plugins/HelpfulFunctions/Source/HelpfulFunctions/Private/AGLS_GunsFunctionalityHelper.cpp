// Fill out your copyright notice in the Description page of Project Settings.


#include "AGLS_GunsFunctionalityHelper.h"

#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

void UAGLS_GunsFunctionalityHelper::GetRifleDesiredUnequipAttachProperties_Implementation(bool& SocketValid, UPrimitiveComponent*& ParentComponent, FName& SocketName, EAttachmentRule& AttachRule, ACharacter* RefCharacter, int SocketIndex)
{
	// Wymagane zachowanie.
	AttachRule = EAttachmentRule::SnapToTarget;

	// Domyœlne wartoœci wyjœciowe.
	SocketValid = false;
	ParentComponent = nullptr;
	SocketName = FName("DefaultSocket");

	// Safety: brak postaci.
	if (!IsValid(RefCharacter)) { return; }

	// Fallback zgodny z Blueprintem: mesh postaci + DefaultSocket.
	USkeletalMeshComponent* CharacterMesh = RefCharacter->GetMesh();

	auto ReturnFallbackSocket = [&]()
		{
			ParentComponent = CharacterMesh;
			SocketName = FName("DefaultSocket");
			//SocketValid = IsValid(ParentComponent);
			SocketValid = false;
		};

	// Safety: brak mesha.
	if (!IsValid(CharacterMesh))
	{
		return;
	}

	// Safety: ochrona TArray przed niepoprawnym indeksem.
	if (!RifleAttachSocketsConfig.IsValidIndex(SocketIndex))
	{
		ReturnFallbackSocket();
		return;
	}

	const FAGLS_FindingAttachSocket& SocketData = RifleAttachSocketsConfig[SocketIndex];

	// Lokalna funkcja pomocnicza:
	// - znajduje UActorComponent po klasie i tagu,
	// - jeœli to PhysicsConstraintComponent, zwraca jego OutComponent1 + OutBoneName1,
	// - jeœli to UPrimitiveComponent, zwraca ten komponent + AttachToSocketName.
	auto ResolveSocketComponent = [&SocketData](
		AActor* TargetActor,
		UPrimitiveComponent*& OutParentComponent,
		FName& OutSocketName) -> bool
		{
			OutParentComponent = nullptr;
			OutSocketName = NAME_None;

			if (!IsValid(TargetActor))
			{
				return false;
			}

			if (!SocketData.ClassOfSocketComponent)
			{
				return false;
			}

			UActorComponent* FoundComponent = TargetActor->FindComponentByTag(
				SocketData.ClassOfSocketComponent,
				SocketData.PrimitiveComponentTag
			);

			if (!IsValid(FoundComponent))
			{
				return false;
			}

			// Specjalna logika dla PhysicsConstraintComponent.
			if (UPhysicsConstraintComponent* ConstraintComponent = Cast<UPhysicsConstraintComponent>(FoundComponent))
			{
				UPrimitiveComponent* OutComponent1 = nullptr;
				FName OutBoneName1 = NAME_None;

				UPrimitiveComponent* OutComponent2 = nullptr;
				FName OutBoneName2 = NAME_None;

				ConstraintComponent->GetConstrainedComponents(
					OutComponent1,
					OutBoneName1,
					OutComponent2,
					OutBoneName2
				);

				if (!IsValid(OutComponent2))
				{
					return false;
				}

				OutParentComponent = OutComponent2;
				OutSocketName = OutBoneName2;
				return true;
			}

			// Standardowa logika dla zwyk³ych komponentów.
			if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(FoundComponent))
			{
				OutParentComponent = PrimitiveComponent;
				OutSocketName = SocketData.AttachToSocketName;
				return true;
			}

			// Znaleziono komponent, ale nie jest ani PhysicsConstraintComponent,
			// ani UPrimitiveComponent, wiêc nie ma czego zwróciæ jako ParentComponent.
			return false;
		};

	// -------------------------------------------------------------------------
	// Wariant 1: szukamy komponentu na aktorze podpiêtym do RefCharacter.
	// -------------------------------------------------------------------------
	if (SocketData.bGetFromAttachedActor)
	{
		AActor* ChosenActor = nullptr;

		TArray<AActor*> AttachedActors;
		RefCharacter->GetAttachedActors(AttachedActors, true);

		for (AActor* AttachedActor : AttachedActors)
		{
			if (!IsValid(AttachedActor))
			{
				continue;
			}

			const bool bClassMatches =
				SocketData.ClassOfAttachedActor &&
				AttachedActor->IsA(SocketData.ClassOfAttachedActor);

			const bool bTagMatches =
				AttachedActor->ActorHasTag(SocketData.AttachedActorTagToIdentify);

			if (bClassMatches && bTagMatches)
			{
				ChosenActor = AttachedActor;
				break;
			}
		}

		UPrimitiveComponent* FoundParentComponent = nullptr;
		FName FoundSocketName = NAME_None;

		if (ResolveSocketComponent(ChosenActor, FoundParentComponent, FoundSocketName))
		{
			ParentComponent = FoundParentComponent;
			SocketName = FoundSocketName;
			SocketValid = true;
			return;
		}

		ReturnFallbackSocket();
		return;
	}

	// -------------------------------------------------------------------------
	// Wariant 2: szukamy komponentu bezpoœrednio na RefCharacter.
	// -------------------------------------------------------------------------
	UPrimitiveComponent* FoundParentComponent = nullptr;
	FName FoundSocketName = NAME_None;

	if (ResolveSocketComponent(RefCharacter, FoundParentComponent, FoundSocketName))
	{
		ParentComponent = FoundParentComponent;
		SocketName = FoundSocketName;
		SocketValid = true;
		return;
	}

	ReturnFallbackSocket();
}


void UAGLS_GunsFunctionalityHelper::GetDesiredRifleAttachSlotsIndices_Implementation(ACharacter* RefCharacter, TArray<int>& ReturnDesiredSlots)
{
	//TArray<int> DesiredSlots; 
	if (!RefCharacter)
	{
		for (int i = 0; i < NumberOfMaxRiflesSlots; i++)
		{
			ReturnDesiredSlots.Add(i);
		}
		return;
	}

	TArray<AActor*> AttachedActors;
	RefCharacter->GetAttachedActors(AttachedActors, true);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor)) { continue; }

		if (AttachedActor->ActorHasTag(TEXT("BackpackActor")) == true)
		{
			for (int i = 0; i < NumberOfMaxRiflesSlots; i++)
			{
				ReturnDesiredSlots.Add(i);
			}
			return;
		}

	}

	for (int i = 0; i < NumberOfMaxRiflesSlots; i++)
	{
		if (i == 0)
		{
			ReturnDesiredSlots.Add( 0 );
		}
		else
		{
			ReturnDesiredSlots.Add(i + 1);
		}
	}
	return;

}


bool UAGLS_GunsFunctionalityHelper::AttachRifleTo_Implementation(ACharacter* RefCharacter, bool AttachToHand, int SlotIndex, bool SolveAllRiflesInEq, bool ForLeftHand)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::AttachGunToByReference_Implementation(ACharacter* RefCharacter, AInteractiveGunCoreActor* GunInstance, bool AttachToHand, bool AutoGunType, bool AutoSlotIndex)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::AttachGunInstanceToUnequipSocket_Implementation(bool ForRifle)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::AttachPistolTo_Implementation(ACharacter* RefCharacter, bool AttachToHand)
{
	return false;
}


void UAGLS_GunsFunctionalityHelper::GetRifleCurrentAttachMode_Implementation(bool& RifleValid, bool& CurrentlyUse, bool& AttachedToHand, FName& SocketName, ACharacter* RefCharacter, int SlotIndex)
{
}


bool UAGLS_GunsFunctionalityHelper::SetupNewRifleInstance_Implementation(ACharacter* RefCharacter, AActor* GunInstance, int DesiredSlotIndex, bool IncludeAttach)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::SetupNewPistolInstance_Implementation(ACharacter* RefCharacter, AActor* GunInstance, bool IncludeAttach)
{
	return false;
}


void UAGLS_GunsFunctionalityHelper::ShotTrigger_Implementation()
{
}


void UAGLS_GunsFunctionalityHelper::UpdateOnTick_Implementation(float DeltaTime)
{
}


void UAGLS_GunsFunctionalityHelper::PerformSingleGunShot_Implementation(float& WaitTime)
{
}


void UAGLS_GunsFunctionalityHelper::CheckCanExecuteGunShot_Implementation(bool& Continue, bool& PlayEmptyAmmo, bool DontCheckTags, bool DontPlaySounds)
{
}


void UAGLS_GunsFunctionalityHelper::DefaultShotTrace_Implementation(FHitResult& DefaultHit, FHitResult& ScanHit, bool& HittedByScan)
{
}


void UAGLS_GunsFunctionalityHelper::ShotgunShotTraces_Implementation(TArray<FHitResult>& DefaultHits, FHitResult& ScanHit, bool& HittedByScan)
{
}


AInteractiveGunCoreActor* UAGLS_GunsFunctionalityHelper::GetCurrentUsedRifle_Implementation()
{
	return nullptr;
}


AInteractiveGunCoreActor* UAGLS_GunsFunctionalityHelper::GetCurrentUsedPistol_Implementation()
{
	return nullptr;
}


void UAGLS_GunsFunctionalityHelper::UpdateDispersionValueForShooting_Implementation(float RightScale, float UpScale, float dt)
{
}


bool UAGLS_GunsFunctionalityHelper::SetNewCurrentRifleSlotIndex_Implementation(int NewSlotIndex, bool CanReplaceGun, AInteractiveGunCoreActor* GunContext)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::TryActivateSwitchingGunInstanceAction_Implementation(int NewRifleSlot, bool ByOverlayMenu, bool ShouldSwitchToRifle)
{
	return false;
}


bool UAGLS_GunsFunctionalityHelper::TryDropPistolInstanceFromInventory_Implementation(UPARAM(ref)AInteractiveGunCoreActor*& CurrentPistolActor, AInteractiveGunCoreActor* PistolInstanceToDrop, 
	int SlotIndexToDrop, bool CanSetOverlayState)
{
	return false;
}
