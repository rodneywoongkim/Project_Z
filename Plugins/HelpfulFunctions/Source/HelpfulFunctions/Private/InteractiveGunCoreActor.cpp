// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveGunCoreActor.h"
#include "GameFramework/Character.h"

AInteractiveGunCoreActor::AInteractiveGunCoreActor()
{
	SetCanBeDamaged(false);

	Tags.Add("IgnoreForCover");
}


bool AInteractiveGunCoreActor::SetPerInstanceValuesFromDefinitionAsset()
{
	if (bIsPistol && PistolDefinitionData) //Do uzupe³nienia !!!
	{
		AutomaticShootingSpeed = PistolDefinitionData->AutomaticShootingSpeed;
		AimInstabilityScale = PistolDefinitionData->AimInstabilityScale;
		RecoilOffsetScale = PistolDefinitionData->RecoilOffsetScale;
		InitDamageValue = PistolDefinitionData->InitDamageValue;
		SingleMagCopacity = PistolDefinitionData->SingleMagCopacity;
	}
	else if(RifleDefinitionData)
	{
		AutomaticShootingSpeed = RifleDefinitionData->AutomaticShootingSpeed;
		AimInstabilityScale = RifleDefinitionData->AimInstabilityScale;
		RecoilOffsetScale = RifleDefinitionData->RecoilOffsetScale;
		InitDamageValue = RifleDefinitionData->InitDamageValue;
		SingleMagCopacity = RifleDefinitionData->SingleMagCopacity;
	}

	return false;
}


float AInteractiveGunCoreActor::GetWeaponShootingSpeed() const
{
	return AutomaticShootingSpeed;
}


void AInteractiveGunCoreActor::GetWeaponRecoilAndInstability(float& ReturnAimInstabilityScale, FVector& ReturnRecoilOffsetScale)
{
	ReturnAimInstabilityScale = AimInstabilityScale;
	ReturnRecoilOffsetScale = RecoilOffsetScale;
	return;
}


float AInteractiveGunCoreActor::GetWeaponInitDamageValue() const
{
	return InitDamageValue;
}


bool AInteractiveGunCoreActor::IsShotgun() const
{
	if (RifleDefinitionData)
	{
		return RifleDefinitionData->ModelCategory == E_RifleModelCategory::Shotgun;
	}
	return false;
}


bool AInteractiveGunCoreActor::IsSniperRifle() const
{
	if (RifleDefinitionData)
	{
		return RifleDefinitionData->ModelCategory == E_RifleModelCategory::SniperWithScope;
	}
	return false;
}

bool AInteractiveGunCoreActor::IsRevolverPistol() const
{
	if (!PistolDefinitionData || !bIsPistol) return false;

	return PistolDefinitionData->ModelCategoryPistol == E_PistolModelCategory::Revolver;
}


bool AInteractiveGunCoreActor::IsHaveParentAsCharacter_Implementation()
{
	if (GetAttachParentActor())
	{
		ACharacter* AsChar = Cast<ACharacter>(GetAttachParentActor());
		if (AsChar) return true;
		else return false;
	}
	return false;
}


void AInteractiveGunCoreActor::SpawnMuzzleFireParticle_Implementation(float Time)
{
}


void AInteractiveGunCoreActor::SpawnShellCasingAfterShot_Implementation(float DelayTime)
{
}
