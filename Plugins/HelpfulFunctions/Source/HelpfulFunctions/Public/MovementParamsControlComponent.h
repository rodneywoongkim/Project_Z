// AGLS v1.9 | JakubW 2026

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMC_MovementPreset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "MovementParamsControlComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewGaitRequired, CALS_Gait, NewGait, CALS_Gait, CurrentGait);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTickBeforeMovementComponent, float, DeltaTime);



/*
█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
An additional component designed for ACharacter-based classes that supports the functionality of the CharacterMovementComponent. 
Its purpose is to create and manage parameters within the CMC component. This allows for faster creation of configurable movement 
presets that affect the overall capsule movement behavior. By default, proper configuration of the required parameters and 
functions is necessary, including assigning objects for DefaultMovementPreset and StrafeSpeedMapCurve.

The component was primarily developed for the AGLS v1.9 project and operates using its systems and structures.
█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████
*/
UCLASS(Blueprintable, ClassGroup=(Movement), meta=(BlueprintSpawnableComponent) )
class HELPFULFUNCTIONS_API UMovementParamsControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMovementParamsControlComponent();


	UPROPERTY(BlueprintAssignable, Category = "Character Movement Control")
	FOnNewGaitRequired OnNewGaitRequired;

	UPROPERTY(BlueprintAssignable, Category = "Character Movement Control")
	FTickBeforeMovementComponent TickBeforeMovementComponent;


	UFUNCTION(BlueprintCallable, Category = "Character Movement Control", meta = (DisplayName = "Call On New Gait Required", Keywords = "Movement,Gait"))
	void CallOnNewGaitDeleage(CALS_Gait NewGait, CALS_Gait CurrentGait)
	{ OnNewGaitRequired.Broadcast(NewGait, CurrentGait); };

private:
	CALS_Gait ActualGaitState = CALS_Gait::Walking;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|References", meta = (AllowPrivateAccess = "True"))
	ACharacter* CharacterRef = nullptr;

	UCharacterMovementComponent* CMC = nullptr;

	CALS_Gait CollectedDesiredGait = CALS_Gait::Walking;
	CALS_Gait CollectedCurrentGait = CALS_Gait::Walking;
	CALS_MovementState CollectedMovement = CALS_MovementState::Grounded;
	CALS_RotationMode CollectedRotationMode = CALS_RotationMode::LookingDirection;
	CALS_Stance CollectedStance = CALS_Stance::Standing;

	bool bTransitionIsRunning = false;
	float TransitionBlendAlpha = 0.0;
	float CurrentPresetsTransitionDuration = 0.0;
	float CurrentPresetTransitionTimer = -1;

	float SpeedValue2D = 0.0;
	float AnyStateChangedTimer = -1;

	float SmoothGaitTransition = 0.0;

	FCALSMovementSettingsStrafeExtend BlendTwoModels(FCALSMovementSettingsStrafeExtend PrevModel, FCALSMovementSettingsStrafeExtend CurrentModel);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool bEnableComponent = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True", EditCondition = "bEnableComponent"))
	UCMC_MovementPreset* DefaultMovementPreset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	FName ModifyDecelerationCurveName = TEXT("Modify_Deceleration");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	FName ModifyGroundFrictionCurveName = TEXT("Modify_GroundFriction");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool CalculateSimpleOnStairsValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool CanUseTransitionsBetweenPresets = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	float DefaultTransitionsDuration = 0.4f;

	//Scale WalkingDeceleration value for CharacterMovement component when has have MovementInput
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	float ScaleDecelerationWhenHasInputs = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool UseInterpModeToCalculateGaitMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True", EditCondition = "UseInterpModeToCalculateGaitMap"))
	float GaitMapInterpSpeed = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	UCurveFloat* StrafeSpeedMapCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool bSkipUpdateWhenNotOnGround = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement Control|Config", meta = (AllowPrivateAccess = "True"))
	bool bDisplayDebugInformations = false;


	UCMC_MovementPreset* CurrentMovementPreset = nullptr;
	UCMC_MovementPreset* PrevMovementPreset = nullptr;
	float WalkingOnStairsAlpha = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|Runtime", meta = (AllowPrivateAccess = "True"))
	FCALSMovementSettingsStrafeExtend CurrentMovementModel;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|Runtime", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState CurrentMovementDirection = AGLS_MovementDirectionState::F;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|Runtime", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState PreviousMovementDirection = AGLS_MovementDirectionState::F;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|Runtime", meta = (AllowPrivateAccess = "True"))
	float RotationOffsetPreSim = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement Control|Runtime", meta = (AllowPrivateAccess = "True"))
	FVector OrientationIntent = FVector::ZeroVector;



	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	void CollectStatesValues(CALS_MovementState& InMovementState, CALS_Stance& InStance, CALS_RotationMode& InRotationMode, CALS_Gait& InDesiredGait, CALS_Gait& InCurrentGait);
	virtual void CollectStatesValues_Implementation(CALS_MovementState& InMovementState, CALS_Stance& InStance, CALS_RotationMode& InRotationMode, CALS_Gait& InDesiredGait, CALS_Gait& InCurrentGait);

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	Calculate the Allowed Gait. This represents the maximum Gait the character is currently allowed to be in, and can be determined by the desired gait, the rotation mode, 
	the stance, etc. For example, if you wanted to force the character into a walking state while indoors, this could be done here.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	CALS_Gait GetAllowedGait();
	virtual CALS_Gait GetAllowedGait_Implementation();

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	CALS_Gait GetActualGait(CALS_Gait InAllowedGait);
	virtual CALS_Gait GetActualGait_Implementation(CALS_Gait InAllowedGait);

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	void SetNewGaitState(CALS_Gait NewState);
	virtual void SetNewGaitState_Implementation(CALS_Gait NewState);

	/*■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	Useful function when you want to apply a custom condition related to updating CMC component parameters. If GetCustomUpdatingCondition() == false, the call 
	to UpdateDynamicMovementSettings() will be skipped, meaning that values such as Acceleration, Deceleration, and MaxSpeed will not be updated.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	bool GetCustomUpdatingCondition();
	virtual bool GetCustomUpdatingCondition_Implementation();



	FCALSMovementSettingsStrafeExtend GetTargetMovementSettings();

	float CalculateMaxSpeed(CALS_Gait AllowedGait);

	float GetMappedSpeed(float Scale, CALS_Gait AllowedGait);
	float UpdateMappedSpeedInterpMode(float Scale, CALS_Gait AllowedGait, float dt);

	float GetTargetSpeedWithStrafe(FVector InSpeeds);

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	This function returns the desired movement direction of the AI controller (if valid), or the player's current movement input direction in camera space.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	FVector GetMovementInput();
	virtual FVector GetMovementInput_Implementation();

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	This function returns the thresholds used to determine which MovmentDirection the pawn should be in, which in turn determines the rotation offset 
	and which directional anims to play. The thresholds represent angles between the orientation intent and the movement direction with a -180 to 180 range.
	You can view these thresholds by enabling the Shapes debug draws from the widget while in Strafe or Aim */
	FVector4d GetMovementDirectionTreshold();

	AGLS_MovementDirectionState GetMovementDirectionStateFromTresholds(FVector4d InTresholds, float Direction);

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	This function returns the MovementDirection and RotationOffset, which controls which directional animations to play as well as how the pawn's 
	rotation is offset relative to the Orientation Intent. Its likely that these behaviors will be built directly into the movement modes in the 
	future, but the concept should remain the same. */
	void CalculateAndUpdateMovementDirectionState(FVector MoveInput, FVector InOrientationIntent, CALS_Gait InGait, bool AlwaysForwardWhenSprint);

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	This function returns the OrientationIntent, which is the direction we want the pawn to face, and is determined primarily by the Rotation Mode. 
	Since we also apply a rotation offset in some movement modes, and since movement modes often will have smoothing applied to the rotation, 
	this can be thought of as the general direction we want to pawn to orient toward. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	FVector GetOrientationIntent(FVector CurrentOrientationIntent);
	virtual FVector GetOrientationIntent_Implementation(FVector CurrentOrientationIntent);

	/*■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	FVector GetCurrentMovementDirection();
	virtual FVector GetCurrentMovementDirection_Implementation();

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	This function returns the AimingRotation, which is the rotation the pawn should be looking when in the Strafe or Aim rotation modes. */
	FRotator GetAimingRotation();

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintCallable, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	void UpdateDynamicMovementSettings(CALS_Gait AllowedGait);

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character Movement Control", meta = (ForceAsFunction, Keywords = "Character,Movement,Component"))
	float UpdateWalkOnStairsAlpha();
	virtual float UpdateWalkOnStairsAlpha_Implementation();

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	Sets a new Preset of values ​​for the CharacterMovementComponent. If UseTransition == true and CanUseTransitionsBetweenPresets 
	== true then a smooth transition between the new and old MovementPreset will be triggered.*/
	UFUNCTION(BlueprintCallable, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	bool SetNewMovementPreset(UCMC_MovementPreset* NewPreset, bool UseTransition = false, float TransitionDurationScale = 1.0);

	/*
	■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	Sets a Default Preset of values ​​for the CharacterMovementComponent. If UseTransition == true and CanUseTransitionsBetweenPresets
	== true then a smooth transition between the new and old MovementPreset will be triggered.*/
	UFUNCTION(BlueprintCallable, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	bool SetDefaultMovementPreset(bool UseTransition = false, float TransitionDurationScale = 1.0);

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	UCMC_MovementPreset* GetCurrentMovementPreset() const;

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	UCMC_MovementPreset* GetPreviousMovementPreset() const;

	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	void GetDesiredMovementsTypeStates(AGLS_WalkingType& OutWalkType, AGLS_RunningType& OutRunType, AGLS_SprintingType& OutSprintType) const;

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Try Get Movement Control Component", Keywords = "Movement,Control,Component,CMC"))
	static UMovementParamsControlComponent* GryGetMovementControlComponent(UObject* WorldContextObject, ACharacter* Target);

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	void GetCollectedStates(CALS_MovementState& ReturnMovementState, CALS_RotationMode& ReturnRotationMode, CALS_Gait& ReturnDesiredGait, CALS_Gait& ReturnCurrentGait);

	//■■▶ 𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐂𝐎𝐍𝐓𝐑𝐎𝐋 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 ◀■■
	UFUNCTION(BlueprintPure, Category = "Character Movement Control", meta = (Keywords = "Character,Movement,Component"))
	float GetOnStairsWalkingAlpha() const;


	void RenderDebugInformations(float DeltaX, CALS_Gait FinalGait);
};
