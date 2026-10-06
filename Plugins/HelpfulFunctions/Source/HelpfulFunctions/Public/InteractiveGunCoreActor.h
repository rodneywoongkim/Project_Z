// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "AGLS_RifleModelDefinition.h"
#include "InteractiveGunCoreActor.generated.h"



/*AGLS v2.0 Guns System - Core Class*/
UCLASS()
class HELPFULFUNCTIONS_API AInteractiveGunCoreActor : public AInteractiveActor
{
	GENERATED_BODY()
	
public:

	AInteractiveGunCoreActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Weapon Config"))
	bool bIsPistol = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Weapon Config", EditCondition = "!bIsPistol", ExposeOnSpawn = "true"))
	UAGLS_RifleModelDefinition* RifleDefinitionData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Weapon Config", EditCondition = "bIsPistol", ExposeOnSpawn = "true"))
	UAGLS_RifleModelDefinition* PistolDefinitionData = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Weapon Runtime", ClampMin = "0", ClampMax = "120", DisplayName = "Current Ammo Amount In Mag"))
	int CurrentAmmoAmount = 30;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "Weapon Runtime"))
	int CurrentMagAmountSimple = 1;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "Weapon Runtime"))
	TArray<int> MagazinesDataCopy;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Weapon Config"))
	//TArray<FName> CompatibleAmmoTags;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "Weapon Runtime", ClampMin = "0", ClampMax = "4"))
	int HolderSlotIndex = 0;
	


	//W³aœciwoœci które mog¹ byæ ró¿nie w zale¿noœci od pojedyñczej instancji a nie tylko definicji poprzez DataAsset
	float AutomaticShootingSpeed = 0.05f;
	float AimInstabilityScale = 0.0;
	FVector RecoilOffsetScale = FVector::ZeroVector;
	float InitDamageValue = 10;
	int SingleMagCopacity = 0;

	UFUNCTION(BlueprintCallable, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun", DisplayName = "Set Per Instance Default Values From Definition Asset"))
	virtual bool SetPerInstanceValuesFromDefinitionAsset();

	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun"))
	float GetWeaponShootingSpeed() const;

	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun"))
	void GetWeaponRecoilAndInstability(float& ReturnAimInstabilityScale, FVector& ReturnRecoilOffsetScale);

	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun"))
	float GetWeaponInitDamageValue() const;


	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool IsShotgun() const;

	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool IsSniperRifle() const;

	UFUNCTION(BlueprintPure, meta = (Category = "Interactive Weapon Actor", Keywords = "Weapon,Pistol,Revolver,Interactive,Gun"))
	bool IsRevolverPistol() const;


	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactive Weapon Actor", meta = (ForceAsFunction, Keywords = "Weapon,Rifle,Interactive,Gun"))
	//void InitializeMagsAndAmmoForInstanceSimple(int NewAmmoAmount, int NewMagazinesAmount);
	//virtual void InitializeMagsAndAmmoForInstanceSimple_Implementation(int NewAmmoAmount, int NewMagazinesAmount);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactive Weapon Actor", meta = (ForceAsFunction, Keywords = "Weapon,Rifle,Interactive,Gun"))
	//void InitializeMagsAndAmmoForInstanceSurvival(const TArray<int>& MagazinesDefinitionData);
	//virtual void InitializeMagsAndAmmoForInstanceSurvival_Implementation(const TArray<int>& MagazinesDefinitionData);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Interactive Weapon Actor", meta = (ForceAsFunction, DisplayName = "Get Have Character Parent", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool IsHaveParentAsCharacter();
	virtual bool IsHaveParentAsCharacter_Implementation();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactive Weapon Actor", meta = (Keywords = "Weapon,Rifle,Interactive,Gun"))
	void SpawnMuzzleFireParticle(float Time);
	virtual void SpawnMuzzleFireParticle_Implementation(float Time);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactive Weapon Actor", meta = (Keywords = "Weapon,Rifle,Interactive,Gun"))
	void SpawnShellCasingAfterShot(float DelayTime);
	virtual void SpawnShellCasingAfterShot_Implementation(float DelayTime);


};
