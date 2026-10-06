// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InteractiveActor.h"
#include "Sound/SoundCue.h"
#include "AGLS_RifleModelDefinition.generated.h"



UENUM(BlueprintType)
enum class E_RifleModelCategory : uint8
{
	FullAuto,
	SniperWithScope,
	HuntingRifle,
	Shotgun,
	SubmachineGun,
	Machinegun,
	Other
};


UENUM(BlueprintType)
enum class E_PistolModelCategory : uint8
{
	Default,
	Revolver,
	HalfAuto,
	Other
};


/*Using this class, we declare a new 𝙋𝙄𝙎𝙏𝙊𝙇 or 𝙍𝙄𝙁𝙇𝙀 model in the AGLS project.*/
UCLASS(BlueprintType, meta = (DisplayName = "GunModelDefinition"))
class HELPFULFUNCTIONS_API UAGLS_RifleModelDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	/*This name will be displayed on the interaction widget.  */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "1 Main")
	FName ModelName = TEXT("none");

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "1 Main")
	bool UseAsPistolConstruction = false;

	/*A parameter that defines the appropriate characteristics of the defined weapon model. A brief description from the available categories:
	1) FullAuto - Classic automatic rifle, e.g., M4A1, AK47
	2) SniperWithScope - a category for sniper rifles. It does not have automatic fire, and has a small amount of ammunition. For this state, 
	a crosshair/scope placeholder is displayed while aiming.
	3) HuntingRifle - similar to SniperWithScope, but one aim does not display the scope widget.
	4) Shotgun - uses many specific features for this type, including a different reloading class.
	5) SubmachineGun - similar to FullAuto
	6) Machinegun - similar to FullAuto */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "1 Main", meta = (EditCondition = "!UseAsPistolConstruction", EditConditionHides))
	E_RifleModelCategory ModelCategory = E_RifleModelCategory::FullAuto;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "1 Main", meta = (EditCondition = "UseAsPistolConstruction", EditConditionHides))
	E_PistolModelCategory ModelCategoryPistol = E_PistolModelCategory::Default;


	/*The default AActor class on which the rifle instance will be based.*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "1 Main")
	TSubclassOf<AInteractiveActor> ConstructClass;



	/*The most important aspect of defining a rifle model is that the weapon's skeletal mesh must be well-prepared to function 
	properly with its build class and other features.*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	TObjectPtr<UPhysicsAsset> PhysicAsset = nullptr;

	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	TSubclassOf<UAnimInstance> MainAnimInstance = nullptr;

	/*An alternative variable storing the AnimInstance class as a SoftReference. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	TSoftClassPtr<UAnimInstance>  MainAnimInstanceSoftRef = nullptr;

	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	TObjectPtr<UStaticMesh> MagazineStaticMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "2 Visual Construction")
	FRotator MagMeshOriginCorrectionRot = FRotator(0,0,0);

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 Visual Construction")
	FName MagazineSocketName;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 Visual Construction")
	FName MuzzleSocketName;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "2 Visual Construction")
	TObjectPtr<USoundCue> ShootingSoundClass = nullptr;



	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 Shooting", meta = (UIMin = "0.03", UIMax = "5", ClampMin = "0.03", ClampMax = "5", ForceUnits = "Seconds"))
	float AutomaticShootingSpeed = 0.05f;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 Shooting", meta = (UIMin = "0", ClampMin = "0", UIMax = "10", ClampMax = "10"))
	float AimInstabilityScale = 0.0;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 Shooting")
	bool bGunHaveSilencer = false;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 Shooting", meta = (EditCondition = "!UseAsPistolConstruction && ModelCategory == E_RifleModelCategory::SniperWithScope", EditConditionHides))
	bool EnableAimInPlaceWhenCrawl = false;

	
	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "3 Shooting")
	FVector RecoilOffsetScale = FVector(0, 0, 0);

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 Damage")
	float InitDamageValue = 10.0;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "4 Damage")
	bool HeadshotAlwaysCauseDeath = false;



	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "5 Animations|Poses")
	UAnimSequence* OverlayPosesPart01 = nullptr;

	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "5 Animations|Poses")
	UAnimSequence* OverlayPosesPart02 = nullptr;

	/*THIS OBJECT IS HOLDING AS SOFT REFERENCE.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Poses", meta = (EditCondition = "UseAsPistolConstruction", EditConditionHides))
	TSoftObjectPtr<UAnimSequence> OverlayPosesPart03 = nullptr;

	/*THIS OBJECT IS HOLDING AS SOFT REFERENCE.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Poses", meta = (EditCondition = "UseAsPistolConstruction", EditConditionHides))
	TSoftObjectPtr<UAnimSequence> OverlayPosesPart04 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload")
	bool bSkipReloadingAnimsSet = false;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload", meta = (EditCondition = "!bSkipReloadingAnimsSet"))
	UAnimMontage* Anim_ReloadMontageCharacter = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload", meta = (EditCondition = "!bSkipReloadingAnimsSet"))
	UAnimMontage* Anim_ReloadMontageProp = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload", meta = (EditCondition = "!bSkipReloadingAnimsSet"))
	UAnimMontage* Anim_ReloadNoDropMontageCharacter = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload", meta = (EditCondition = "!bSkipReloadingAnimsSet"))
	UAnimMontage* Anim_ReloadNoDropMontageProp = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Reload", meta = (EditCondition = "bSkipReloadingAnimsSet", EditConditionHides))
	FGameplayTag ReloadActionAbilityTag;


	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Recoil")
	UAnimMontage* Anim_SingleShotCharacter = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Recoil")
	UAnimMontage* Anim_SingleShotProp = nullptr;

	/*THIS OBJECT IS HOLDING AS SOFT REFERENCE.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Recoil")
	TArray<TSoftObjectPtr<UAnimMontage>> Anim_RecoilAnimationsCollection;


	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Transitions")
	UAnimSequence* Anim_ToAimTransition01 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Transitions")
	UAnimSequence* Anim_ToAimTransition02 = nullptr;


	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	bool ItsFullSetForFillAmmoAction = false;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_Start_Character= nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_Start_PropGun = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_Start_PropBackpack = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_End_Character = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_End_PropGun = nullptr;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "5 Animations|Ammo")
	TSoftObjectPtr<UAnimSequenceBase> Anim_FillMag_End_PropBackpack = nullptr;



	/* */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "6 Mag & Ammo")
	bool EnableSurvivalAmmoMode = true;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "6 Mag & Ammo", meta = (UIMin = "1", ClampMin = "1", UIMax = "90", ClampMax = "90"))
	int32 SingleMagCopacity = 30;

	/* */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "6 Mag & Ammo", meta = (UIMin = "1", ClampMin = "1", UIMax = "120", ClampMax = "120", 
		EditCondition = "EnableSurvivalAmmoMode == true || ModelCategory == E_RifleModelCategory::Shotgun || ModelCategory == E_RifleModelCategory::HuntingRifle"))
	int32 MaxAmmoCanHoldController = 60;

	/* */
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "6 Mag & Ammo", meta = (UIMin = "0", ClampMin = "0", UIMax = "8", ClampMax = "8"))
	//int32 MaxMagCanHoldController = 4;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "6 Mag & Ammo", meta = (EditCondition = "EnableSurvivalAmmoMode"))
	TArray<FName> CompatibleAmmoTypesName;




	
};
