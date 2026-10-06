// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AGLS_GunsFunctionalityHelper.h"
#include "InteractiveActor.h"
#include "AGLS_PlayerInitializationData.generated.h"


UENUM(BlueprintType)
enum class AGLS_InitializeInstanceMode : uint8
{
	NoInitialize,
	SpawnInstance,
	GetInstanceFromWorld,
	Custom
};


USTRUCT(BlueprintType)
struct FAGLS_Setup_GunInstanceSpawnConfig : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration")
	bool bInitializeThisInstance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration")
	bool GetInstanceFromCurrentWorld = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (EditCondition = "GetInstanceFromCurrentWorld", EditConditionHides))
	FString GunInstanceObjectName = TEXT("GunInstance01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	TSoftClassPtr<AActor> GunClassToSpawn = nullptr;

	/*Since the `UAGLS_PlayerInitializationData` class is created in C++, it does not have access to 
	Blueprint enum declarations. Therefore, instead of enum values, it is necessary to use integers 
	representing the enum values. By default in AGLS, the `Rifle_Model` and `Pistol_Model` structures 
	are defined as follows:
	Rifles:
		𝟬。M4A1
		𝟭。AK-47
		𝟮。Famas
		𝟯。AS-50
		𝟰。aa-50-beowulf
		𝟱。perun-x16
		𝟲。C8
		𝟳。Mossberg-590

	Pistols:
		𝟬。M9
		𝟭。P-018
		𝟮。S&W Model 39
		𝟯。Desert Eagle
		𝟰。RSH-12*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (ClampMin = "0", ClampMax = "10", EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	int ModelIndex = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (ClampMin = "0", ClampMax = "100", EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	int AmmoAmountOnSpawn = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (ClampMin = "0", ClampMax = "10", EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	int MagazinesAmountOnSpawn = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	float CustomCollectedParam01 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Configuration", meta = (EditCondition = "bInitializeThisInstance && !GetInstanceFromCurrentWorld", EditConditionHides))
	float CustomCollectedParam02 = 0.0f;

};


USTRUCT(BlueprintType)
struct FAGLS_Setup_OtherPropSpawnConfig : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration")
	AGLS_InitializeInstanceMode InitializeMode = AGLS_InitializeInstanceMode::SpawnInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration", 
		meta = (EditCondition = "InitializeMode == AGLS_InitializeInstanceMode::SpawnInstance || InitializeMode == AGLS_InitializeInstanceMode::GetInstanceFromWorld", EditConditionHides))
	TSoftClassPtr<AActor> InstanceClassType = AInteractiveActor::StaticClass();

	//Remember ObjectName != DisplayName
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration", meta = (EditCondition = "InitializeMode == AGLS_InitializeInstanceMode::GetInstanceFromWorld", EditConditionHides))
	FString InstanceObjectName = TEXT("OtherProp01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration", meta = (EditCondition = "InitializeMode == AGLS_InitializeInstanceMode::SpawnInstance", EditConditionHides))
	FString PropDataTableRowName = TEXT("Default");

	//By default this parameter is not important
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration", meta = (EditCondition = "InitializeMode == AGLS_InitializeInstanceMode::SpawnInstance", EditConditionHides))
	float CustomCollectedParam01 = 0.0f;

	//By default this parameter is not important
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Configuration", meta = (EditCondition = "InitializeMode == AGLS_InitializeInstanceMode::SpawnInstance", EditConditionHides))
	float CustomCollectedParam02 = 0.0f;

};


/*✶ 𝖕𝖑𝖆𝖞𝖊𝖗 𝖏𝖓𝖎𝖙𝖆𝖑𝖎𝖟𝖆𝖙𝖎𝖔𝖓 𝖉𝖆𝖙𝖆 ✶
Using this class, a player character in AGLS can be initialized with the appropriate equipment.
Instances can be retrieved from the current scene or created anew by specifying the appropriate class.*/
UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UAGLS_PlayerInitializationData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack Initialization")
	AGLS_InitializeInstanceMode BackpackInitializationMode = AGLS_InitializeInstanceMode::NoInitialize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack Initialization", meta = 
		(EditCondition = "BackpackInitializationMode == AGLS_InitializeInstanceMode::GetInstanceFromWorld || BackpackInitializationMode == AGLS_InitializeInstanceMode::SpawnInstance", EditConditionHides))
	FString BackpackInstanceObjectName = TEXT("Backpack01");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack Initialization", meta = (EditCondition = "BackpackInitializationMode == AGLS_InitializeInstanceMode::SpawnInstance", EditConditionHides))
	TSoftClassPtr<AActor> BackpackClassToSpawn = nullptr;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rifles Initialization")
	TArray<FAGLS_Setup_GunInstanceSpawnConfig> RiflesConstructionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistols Initialization")
	TArray<FAGLS_Setup_GunInstanceSpawnConfig> PistolsConstructionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Other Props Initialization")
	TArray<FAGLS_Setup_OtherPropSpawnConfig> OtherPropsConstrictionData;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Stats")
	bool bOverrideInventoryStatsForGuns = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Stats", meta = (EditCondition = "bOverrideInventoryStatsForGuns"))
	TMap<FName, FAGLS_GunMagazinesData> PistolMagazinesInInventory;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Stats", meta = (EditCondition = "bOverrideInventoryStatsForGuns"))
	TMap<FName, FAGLS_GunMagazinesData> RifleMagazinesInInventory;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Stats", meta = (EditCondition = "bOverrideInventoryStatsForGuns"))
	TMap<FName, int> CurrentAmmoDataInInventory;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons Config")
	bool bOverrideDefaultGunsSystemConfig = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons Config", meta = (ClampMin = "0", ClampMax = "3", EditCondition = "bOverrideDefaultGunsSystemConfig"))
	int NumberOfMaxRiflesSlots = 1;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons Config", meta = (ClampMin = "0", ClampMax = "3", EditCondition = "bOverrideDefaultGunsSystemConfig"))
	int	NumberOfMaxPistolSlots = 1;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons Config", meta = (EditCondition = "bOverrideDefaultGunsSystemConfig"))
	bool CanCollectingNonCompatibilityAmmo = false;

	/*‖ 𝐖𝐄𝐀𝐏𝐎𝐍𝐒 𝕊𝕐𝕊𝕋𝔼𝕄 ‖ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons Config", meta = (EditCondition = "bOverrideDefaultGunsSystemConfig"))
	bool CanCollectingNonCompatibilityMags = false;


	//When this value is < 0.0 then override is not executed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float OverrideHealthPoints = -1.0f;

};
