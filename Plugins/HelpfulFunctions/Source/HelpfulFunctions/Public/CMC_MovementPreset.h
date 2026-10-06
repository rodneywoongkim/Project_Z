// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "CMC_MovementPreset.generated.h"

/*
A DataAsset designed to control values related to the CharacterMovementComponent. It allows you to define the current movement parameters 
for the Character class, including speeds for the 3-level gait system - Walk / Jog / Run - along with Strafe values, selection based on 
the capsule rotation mode, and Stance values - Standing / Crouching.

Additionally, it includes extra parameters intended for configuring PoseSearch Databases.

Mainly this class is created and prepared for AGLS v1.9+ project 
 */
UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UCMC_MovementPreset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Velocity Rotation")
	FCALSMovementSettingsStrafeExtend VelocityDirectionSettings_Stand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Velocity Rotation")
	FCALSMovementSettingsStrafeExtend VelocityDirectionSettings_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Looking Rotation")
	FCALSMovementSettingsStrafeExtend LookingDirectionSettings_Stand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Looking Rotation")
	FCALSMovementSettingsStrafeExtend LookingDirectionSettings_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Aiming Rotation")
	FCALSMovementSettingsStrafeExtend AimingDirectionSettings_Stand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Aiming Rotation")
	FCALSMovementSettingsStrafeExtend AimingDirectionSettings_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Stairs")
	bool bIncludeSettingsForStairsWalk = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Stairs", meta = (EditCondition = "bIncludeSettingsForStairsWalk"))
	FCALSMovementSettingsStrafeExtend OnStairsSettings_Stand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Stairs", meta = (EditCondition = "bIncludeSettingsForStairsWalk"))
	FCALSMovementSettingsStrafeExtend OnStairsSettings_Crouch;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings")
	bool CanSetSprintGaitWhenNoInputs = true;

	/*
	When WantsToSprint is true while in the Strafe rotation mode, we limit sprint to only be possible when the input direction is toward the 
	orientation intent, with a different threshold depending on the SprintingTreshold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings")
	bool CanSprintOnlyInForwardMove = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings", meta = (ClampMin = "-1.0", ClampMax = "1.0", EditCondition = "CanSprintOnlyInForwardMove"))
	float GaitSprintingTreshold = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings")
	bool SprintDiectionStateAlwaysForward = false;

	/*
	Using this option makes it so that values for time = 4 are also retrieved from the MovementCurve.
	By default, there are 3 available gait states:
	• gait = walk retrieves the FVector value from the curve where time = 1
	• gait = run retrieves the value where time = 2
	• gait = sprint retrieves the value where time = 3
	However, for gait = sprint, when UseDynamicParamsForSprintGait is set to true, values for time = 4 are also available.
	The choice between time = 3 and time = 4 depends on DynamicRangeInForAcceleration or DynamicRangeInForFriction.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings")
	bool UseDynamicParamsForSprintGait = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings", meta = (EditCondition = "UseDynamicParamsForSprintGait"))
	FVector2D DynamicRangeInForAcceleration = FVector2D(300, 500);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings", meta = (EditCondition = "UseDynamicParamsForSprintGait"))
	FVector2D DynamicRangeInForFriction = FVector2D(0, 500);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings|Global Settings")
	bool bUseDiagonalRotationOffsets = false;

};
