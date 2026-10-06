// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AGLS_GameModeBase.generated.h"

/*AGLS Gamemode Core Class*/
UCLASS()
class HELPFULFUNCTIONS_API AAGLS_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (ExposeOnSpawn = "true"))
	bool EnemiesCanDropRifleOrPistol = true;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (UIMin = "0", ClampMin = "0", UIMax = "100", ClampMax = "100", ForceUnits = "Percent", EditCondition = "EnemiesCanDropRifleOrPistol"))
	double ChanceOfEnemyDroppingGun = 100;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (ExposeOnSpawn = "true"))
	bool EnemiesCanDropGunMags = false;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (UIMin = "0", ClampMin = "0", UIMax = "100", ClampMax = "100", ForceUnits = "Percent", EditCondition = "ChanceOfEnemyDroppingMags"))
	double ChanceOfEnemyDroppingMags = 50;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (ExposeOnSpawn = "true"))
	bool EnemiesCanDropGunAmmo = false;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (UIMin = "0", ClampMin = "0", UIMax = "100", ClampMax = "100", ForceUnits = "Percent", EditCondition = "ChanceOfEnemyDroppingAmmo"))
	double ChanceOfEnemyDroppingAmmo = 25;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (UIMin = "-1", ClampMin = "-1", UIMax = "60", ClampMax = "60", ExposeOnSpawn = "true"))
	int32 MaxAmmoAmountCanDropEnemy = 10;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (ExposeOnSpawn = "true"))
	bool GunsMagazinesArStoredInPlayerChar = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Configuration", meta = (ExposeOnSpawn = "true", EditCondition = "GunsMagazinesArStoredInPlayerChar"))
	bool CanUseSurvivalModeGunsMode = true;
	
};
