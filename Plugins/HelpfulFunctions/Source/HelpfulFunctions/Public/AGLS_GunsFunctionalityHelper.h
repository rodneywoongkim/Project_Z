// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "InteractiveGunCoreActor.h"
#include "AGLS_GunsFunctionalityHelper.generated.h"



USTRUCT(BlueprintType)
struct FAGLS_GunMagazinesData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazines")
	TArray<int> MagazinesData;

};


USTRUCT(BlueprintType)
struct FAGLS_AmmoTypeConstruction : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Ammo Type Name"), Category = "Basic")
	FName AmmoTypeName = TEXT("None");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Is Rifle Ammo"), Category = "Basic")
	bool IsRifleAmmo = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Is Shotgun Ammo", EditCondition = "IsRifleAmmo"), Category = "Basic")
	bool IsShotgunAmmo = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Bullet Mesh Full"), Category = "Visual")
	UStaticMesh* BulletMeshFull = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Bullet Mesh Shell"), Category = "Visual")
	UStaticMesh* BulletMeshShell = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Instancing Offset"), Category = "Visual")
	FTransform InstancingOffset = FTransform::Identity;

};


USTRUCT(BlueprintType)
struct FAGLS_FindingAttachSocket : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config"))
	bool bGetFromAttachedActor = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config", EditCondition = "bGetFromAttachedActor"))
	TSubclassOf<AActor> ClassOfAttachedActor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config", EditCondition = "bGetFromAttachedActor"))
	FName AttachedActorTagToIdentify = FName("none");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config"))
	TSubclassOf<UActorComponent> ClassOfSocketComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config"))
	FName PrimitiveComponentTag = FName("none");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config"))
	FName AttachToSocketName = FName("Socket01");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Category = "Searching Socket Config"), AdvancedDisplay)
	FTransform AttachedObjectOffset = FTransform::Identity;

};



/*
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ An additional class supporting the operation of systems related to pistols and rifles.			
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛*/
UCLASS(BlueprintType, Blueprintable)
class HELPFULFUNCTIONS_API UAGLS_GunsFunctionalityHelper : public UObject
{
	GENERATED_BODY()
	
public:

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Config", meta = (ClampMin = "0", ClampMax = "3"))
	int NumberOfMaxRiflesSlots = 1;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Config", meta = (ClampMin = "0", ClampMax = "3"))
	int	NumberOfMaxPistolSlots = 1;


	//‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ amount of ammunition in the inventory. If this variable is disabled then value = -1
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "-1", ClampMax = "250"))
	//int CurrentUsableAmmoAmount = 0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Config")
	bool CanCollectingNonCompatibilityAmmo = true;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Config")
	bool CanCollectingNonCompatibilityMags = true;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Config")
	TArray<FAGLS_FindingAttachSocket> RifleAttachSocketsConfig;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	This is a important variable when 'Survival Mode Guns' is active. Information regarding the ammunition currently held in 
	the inventory is determined using a unique FName identifier and an integer value representing the quantity. Since different 
	weapon models may use different types of ammunition, it is necessary to associate them using an ID.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags")
	TMap<FName, int> CurrentAmmoDataInInventory;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "4", ClampMax = "160"))
	int MaxAmmoPerTypeInInventory = 60;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "10", ClampMax = "300"))
	int MaxTotalAmmoCanBeInInventory = 140;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	This is a simple value representing the number of available magazines in the player's inventory. It is relevant only when 
	this information should be stored by the player rather than the weapon instance itself. For example, if this number is 3 and 
	the current weapon model holds 30 rounds per magazine, the total available ammunition is 3 × 30 = 90.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "-1", ClampMax = "10"))
	TMap<FName, int> SimpleRifleMagazinesAmount;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	This is a simple value representing the number of available magazines in the player's inventory. It is relevant only when
	this information should be stored by the player rather than the weapon instance itself. For example, if this number is 3 and
	the current weapon model holds 30 rounds per magazine, the total available ammunition is 3 × 30 = 90.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "-1", ClampMax = "10"))
	TMap<FName, int> SimplePistolMagazinesAmount;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	This value is usable when 'Survival Mode Rifles' is enable. The structure of magazines available in the player's inventory. 
	Data is stored in the form of a TMap because a player may have multiple weapons, making it necessary to identify the 
	compatibility of the data with the currently used model.An array element can be interpreted as a magazine, while the value 
	represents the amount of ammunition currently in that mag.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags")
	TMap<FName, FAGLS_GunMagazinesData> RifleMagazinesInInventory;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	This value is usable when 'Survival Mode Pistols' is enable. The structure of magazines available in the player's inventory. 
	Data is stored in the form of a TMap because a player may have multiple weapons, making it necessary to identify the 
	compatibility of the data with the currently used model.An array element can be interpreted as a magazine, while the value 
	represents the amount of ammunition currently in that mag.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags")
	TMap<FName, FAGLS_GunMagazinesData> PistolMagazinesInInventory;

	//‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	//This value is usable when 'Survival Mode Rifles' is enable 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "0", ClampMax = "8"))
	int MaxRifleMagazinesCanBeInInventory = 4;

	//‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	//This value is usable when 'Survival Mode Pistols' is enable 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Ammo & Mags", meta = (ClampMin = "0", ClampMax = "8"))
	int MaxPistolMagazinesCanBeInInventory = 4;



	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Rifles Slots")
	TMap<int, AActor*> AllEquipmentRifles;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Rifles Slots", meta = (ClampMin = "0", ClampMax = "3"))
	int CurrentRifleSlotIndex = 0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Pistol Slots", meta = (ClampMin = "0", ClampMax = "3"))
	int DesiredRifleSlotIndex = 0;



	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Pistol Slots")
	TArray<AActor*> AllEquipmentPistols;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Pistol Slots", meta = (ClampMin = "0", ClampMax = "3"))
	int CurrentPistolSlotIndex = 0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Guns Helper|Pistol Slots", meta = (ClampMin = "0", ClampMax = "3"))
	int DesiredPistolSlotIndex = 0;


	//▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂▂
	//████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
	//▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Global References")
	ACharacter* OwnerCharacter = nullptr;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Global References")
	bool CharacterAiming = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Global References")
	bool CharacterHoldingGun = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	bool bLockShootingTrigger = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	bool bIsShooting = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	float AutomaticShootingElapsedTime = 0.0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	float UpCameraOffsetTime = 0.0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	float UpCameraOffsetScale = 0.0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	int SniperRifleFOVIndex = 0;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	FVector SniperRifleLastDestination = FVector::ZeroVector;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	FVector2D DispersionScale = FVector2D(0, 0);

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime", meta = (ClampMin = "0.3", ClampMax = "3.0"))
	float EquipAnimsPlayRate = 1.f;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(BlueprintReadWrite, Category = "Guns Helper|Runtime")
	bool EnableSniperRifleAimMode = false;


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Get Rifle Desired Unequip Attach Properties", Keywords = "Weapon,Rifle,Interactive,Gun", AdvancedDisplay = "AttachRule"))
	void GetRifleDesiredUnequipAttachProperties(bool& SocketValid, UPrimitiveComponent*& ParentComponent, FName& SocketName, EAttachmentRule& AttachRule, ACharacter* RefCharacter, int SocketIndex);
	virtual void GetRifleDesiredUnequipAttachProperties_Implementation(bool& SocketValid, UPrimitiveComponent*& ParentComponent, FName& SocketName, EAttachmentRule& AttachRule, ACharacter* RefCharacter, int SocketIndex);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Get Desired Rifle Attach Slots Indices", Keywords = "Weapon,Rifle,Interactive,Gun"))
	void GetDesiredRifleAttachSlotsIndices(ACharacter* RefCharacter, TArray<int>& ReturnDesiredSlots);
	virtual void GetDesiredRifleAttachSlotsIndices_Implementation(ACharacter* RefCharacter, TArray<int>& ReturnDesiredSlots);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	Rolą tej funkcji jest wykonanie prawidłowego AttachActorToComponent wskazanej instancji karabinu
	- Opcja AttachToHand powoduje że GunInstance będzie doczepiony tak aby pobierał transformacje z odpowiedniej kości Mesh lub komponentu 
	Root reprezentującego pozycje takiej kości
	- SlotIndex to natomias informacja o tym jaka instancja GunActor powinna zostać z aktualizowana pod kątem kopiowania pozycji Parenta*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Try Attach Rifle To", Keywords = "Weapon,Rifle,Interactive,Gun", AdvancedDisplay = "SolveAllRiflesInEq, ForLeftHand"))
	bool AttachRifleTo(ACharacter* RefCharacter, bool AttachToHand, int SlotIndex = -1, bool SolveAllRiflesInEq = false, bool ForLeftHand = false);
	virtual bool AttachRifleTo_Implementation(ACharacter* RefCharacter, bool AttachToHand, int SlotIndex = -1, bool SolveAllRiflesInEq = false, bool ForLeftHand = false);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ 
	Rolą tej funkcji jest wykonanie prawidłowego AttachActorToComponent wskazanej instancji pistoletu
	- Opcja AttachToHand powoduje że GunInstance będzie doczepiony tak aby pobierał transformacje z odpowiedniej kości Mesh lub komponentu 
	Root reprezentującego pozycje takiej kości*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Try Attach Pistol To", Keywords = "Weapon,Pistol,Interactive,Gun"))
	bool AttachPistolTo(ACharacter* RefCharacter, bool AttachToHand);
	virtual bool AttachPistolTo_Implementation(ACharacter* RefCharacter, bool AttachToHand);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Attach Gun To By Reference", Keywords = "Weapon,Rifle,Interactive,Gun", AdvancedDisplay = "AutoGunType, AutoSlotIndex"))
	bool AttachGunToByReference(ACharacter* RefCharacter, AInteractiveGunCoreActor* GunInstance, bool AttachToHand, bool AutoGunType = true, bool AutoSlotIndex = true);
	virtual bool AttachGunToByReference_Implementation(ACharacter* RefCharacter, AInteractiveGunCoreActor* GunInstance, bool AttachToHand, bool AutoGunType = true, bool AutoSlotIndex = true);

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Attach", meta = (ForceAsFunction, DisplayName = "Attach Gun Instance To Unequip Socket", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool AttachGunInstanceToUnequipSocket(bool ForRifle);
	virtual bool AttachGunInstanceToUnequipSocket_Implementation(bool ForRifle);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Guns Helper|Functions|Get", meta = (ForceAsFunction, DisplayName = "Get Rifle Current Attach Mode", Keywords = "Weapon,Rifle,Interactive,Gun", AdvancedDisplay = "SocketName"))
	void GetRifleCurrentAttachMode(bool& RifleValid, bool& CurrentlyUse, bool& AttachedToHand, FName& SocketName, ACharacter* RefCharacter, int SlotIndex);
	virtual void GetRifleCurrentAttachMode_Implementation(bool& RifleValid, bool& CurrentlyUse, bool& AttachedToHand, FName& SocketName, ACharacter* RefCharacter, int SlotIndex);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Instance Prepare", meta = (ForceAsFunction, DisplayName = "Setup New Rifle Instance", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool SetupNewRifleInstance(ACharacter* RefCharacter, AActor* GunInstance, int DesiredSlotIndex, bool IncludeAttach);
	virtual bool SetupNewRifleInstance_Implementation(ACharacter* RefCharacter, AActor* GunInstance, int DesiredSlotIndex, bool IncludeAttach);

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Instance Prepare", meta = (ForceAsFunction, DisplayName = "Setup New Pistol Instance", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool SetupNewPistolInstance(ACharacter* RefCharacter, AActor* GunInstance, bool IncludeAttach);
	virtual bool SetupNewPistolInstance_Implementation(ACharacter* RefCharacter, AActor* GunInstance, bool IncludeAttach);



	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Events", meta = (DisplayName = "Shot Trigger", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void ShotTrigger();
	virtual void ShotTrigger_Implementation();


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Events", meta = (DisplayName = "Update Event", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void UpdateOnTick(float DeltaTime);
	virtual void UpdateOnTick_Implementation(float DeltaTime);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Core", meta = (ForceAsFunction, DisplayName = "Perform Single Gun Shot", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void PerformSingleGunShot(float& WaitTime);
	virtual void PerformSingleGunShot_Implementation(float& WaitTime);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Shot Construction", meta = (ForceAsFunction, DisplayName = "Check Can Execute Gun Shot", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void CheckCanExecuteGunShot(bool& Continue, bool& PlayEmptyAmmo, bool DontCheckTags, bool DontPlaySounds);
	virtual void CheckCanExecuteGunShot_Implementation(bool& Continue, bool& PlayEmptyAmmo, bool DontCheckTags, bool DontPlaySounds);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Shot Construction", meta = (ForceAsFunction, DisplayName = "Default Shot Trace", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void DefaultShotTrace(FHitResult& DefaultHit, FHitResult& ScanHit, bool& HittedByScan);
	virtual void DefaultShotTrace_Implementation(FHitResult& DefaultHit, FHitResult& ScanHit, bool& HittedByScan);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Shot Construction", meta = (ForceAsFunction, DisplayName = "Shotgun Shot Traces", Keywords = "Weapon,Rifle,Interactive,Gun"))
	void ShotgunShotTraces(TArray<FHitResult>& DefaultHits, FHitResult& ScanHit, bool& HittedByScan);
	virtual void ShotgunShotTraces_Implementation(TArray<FHitResult>& DefaultHits, FHitResult& ScanHit, bool& HittedByScan);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Guns Helper|Functions|Get", meta = (ForceAsFunction, DisplayName = "Get Current Used Rifle", Keywords = "Weapon,Rifle,Interactive,Gun"))
	AInteractiveGunCoreActor* GetCurrentUsedRifle(); virtual AInteractiveGunCoreActor* GetCurrentUsedRifle_Implementation();


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Guns Helper|Functions|Get", meta = (ForceAsFunction, DisplayName = "Get Current Used Pistol", Keywords = "Weapon,Pistol,Interactive,Gun"))
	AInteractiveGunCoreActor* GetCurrentUsedPistol(); virtual AInteractiveGunCoreActor* GetCurrentUsedPistol_Implementation();


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Shot Construction", meta = (ForceAsFunction, DisplayName = "Update Dispersion Value For Shooting", Keywords = "Weapon,Rifle,Interactive,Gun,Pistol"))
	void UpdateDispersionValueForShooting(float RightScale = 1.f, float UpScale = 1.f, float dt = 0.1f);
	virtual void UpdateDispersionValueForShooting_Implementation(float RightScale = 1.f, float UpScale = 1.f, float dt = 0.1f);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Instance Prepare", meta = (ForceAsFunction, DisplayName = "Set New Current Rifle Slot Index", Keywords = "Weapon,Rifle,Interactive,Gun", AdvancedDisplay = "GunContext"))
	bool SetNewCurrentRifleSlotIndex(int NewSlotIndex, bool CanReplaceGun = true, AInteractiveGunCoreActor* GunContext = nullptr); 
	virtual bool SetNewCurrentRifleSlotIndex_Implementation(int NewSlotIndex, bool CanReplaceGun = true, AInteractiveGunCoreActor* GunContext = nullptr);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Action", meta = (ForceAsFunction, DisplayName = "Try Activate Switching Rifle Instance Action", Keywords = "Weapon,Rifle,Interactive,Gun"))
	bool TryActivateSwitchingGunInstanceAction(int NewRifleSlot, bool ByOverlayMenu, bool ShouldSwitchToRifle);
	virtual bool TryActivateSwitchingGunInstanceAction_Implementation(int NewRifleSlot, bool ByOverlayMenu, bool ShouldSwitchToRifle);


	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guns Helper|Functions|Instance Prepare", meta = (ForceAsFunction, DisplayName = "Try Drop Pistol Instance From Inventory", Keywords = "Weapon,Pistol,Interactive,Gun", AdvancedDisplay = "CanSetOverlayState"))
	bool TryDropPistolInstanceFromInventory(UPARAM(ref) AInteractiveGunCoreActor*& CurrentPistolActor, AInteractiveGunCoreActor* PistolInstanceToDrop, int SlotIndexToDrop, bool CanSetOverlayState = true);
	virtual bool TryDropPistolInstanceFromInventory_Implementation(UPARAM(ref) AInteractiveGunCoreActor*& CurrentPistolActor, AInteractiveGunCoreActor* PistolInstanceToDrop, int SlotIndexToDrop, bool CanSetOverlayState = true);

};
