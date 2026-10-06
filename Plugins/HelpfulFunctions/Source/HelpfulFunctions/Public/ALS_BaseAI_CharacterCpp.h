

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ALS_DamageConfigData.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "ALS_HumanAI_InterfaceCpp.h"
#include "AGLS_AI_CharacterInterface.h"
#include "ALS_BaseAI_CharacterCpp.generated.h"


/*
Structure intended for controlling parameters related to CharacterMovement.

MovementCurve is associated with:
- WalkingAcceleration (X curve),
- Deceleration (Y curve),
- GroundFriction (Z curve).

RotationRateCurve is intended for controlling the interpolation speed of capsule rotation toward DesiredRotation.

NOTE: This structure is marked as deprecated in AGLS v1.9.
Currently, these values for both PlayerCharacter and HumanAI are controlled by MovementParamsControlComponent,
which uses different structure types. */
USTRUCT(BlueprintType)
struct FCALSMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;
};


UCLASS()
class HELPFULFUNCTIONS_API AALS_BaseAI_CharacterCpp : public ACharacter, public IALS_HumanAI_InterfaceCpp, public IAGLS_AI_CharacterInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AALS_BaseAI_CharacterCpp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	ACharacter* Self;


	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values|Cached", meta = (AllowPrivateAccess = "True"))
	FVector PrevVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values|Cached", meta = (AllowPrivateAccess = "True"))
	float PrevAimYawC = 0.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config", meta = (AllowedClasses = "/Script/HelpfulFunctions.ALS_DamageConfigData"))
	UClass* DamageDataClass;

	/*AccelerationC is the physical representation of the capsule’s acceleration. It is calculated based on the Velocity value.
	The equation representing this value can be written as: (Velocity[n] - Velocity[n - 1]) / dt*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	FVector AccelerationC = FVector(0, 0, 0);

	/*SpeedC is the speed calculated from Velocity while ignoring the Z axis. SpeedC = GetVelocity().SizeXY()*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	float SpeedC = 0.0f;

	/*IsMoving refers to whether the character is currently moving. By default, this value is calculated as Speed > 1.0.*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	bool IsMovingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Rotators", meta = (AllowPrivateAccess = "True"))
	FRotator LastVelocityRotationC = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Rotators", meta = (AllowPrivateAccess = "True"))
	FRotator LastMovementInputRotationC = FRotator::ZeroRotator;

	/*AimYawRateC = abs((GetControlRotation().Yaw - PreviousAimYawC) / SafeDelta);*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	float AimYawRateC = 0.0f;

	/*MovementInputAmountC = AccelerationXY.Length() / this->GetCharacterMovement()->GetMaxAcceleration();*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	float MovementInputAmountC = 0.0f;


	/*Usually indicates whether the player currently has any active movement input. In the case of a keyboard, this would be for example the W key.
	if (MovementInputAmountC > 0.0) { HasMovementInputC = true; } else { HasMovementInputC = false; }*/
	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Values", meta = (AllowPrivateAccess = "True"))
	bool HasMovementInputC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Rotators", meta = (AllowPrivateAccess = "True"))
	FRotator SmoothTargetAimingC = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Rotators", meta = (AllowPrivateAccess = "True"))
	FRotator TargetRotationC = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Movement Rotators", meta = (AllowPrivateAccess = "True"))
	FRotator InAirRotationC = FRotator::ZeroRotator;



	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Enemy Construction", meta = (AllowPrivateAccess = "True"))
	ACharacter* TargetEnemyActorC;

	UPROPERTY(BlueprintReadWrite, Category = "Base Character AI|Enemy Construction", meta = (AllowPrivateAccess = "True"))
	float DetectedEnemyTimeC = 0.0f;



	// States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True")) //Zmieniono np. TEnumAsByte<CALS_Gait> na CALS_Gait !!!
	CALS_Gait GaitC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait DesiredGaitC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_OverlayState OverlayStateC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_Stance StanceC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode RotationModeC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction MovementActionC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character AI|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState MovementStateC;



	/*⚠︎ DEPRECATED Variable - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FCALSMovementSettings CurrentMovementSettingsC;


	UPROPERTY(BlueprintReadWrite, Category = "Human AI|Essential", meta = (AllowPrivateAccess = "True"))
	bool LockRotationUpdatingC = false;



	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintCallable, Category = "Base Character AI|Movement", meta = (DisplayName = "Smoothed Character Rotation", Keywords = "ALS Character"))
	virtual void SmoothedCharRotation(FRotator Target, float TargetInterpSpeedConst, float ActorInterpSpeedSmooth, bool UpdateControl);


	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintCallable, Category = "Base Character AI|Movement", meta = (DisplayName = "Limit Rotation", Keywords = "ALS Character"))
	virtual void LimitRotationFast(float AimYawMin, float AimYawMax, float InterpSpeed);


	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Can Update Moving Rotation", Keywords = "ALS Character"))
	virtual bool CanUpdateMovingRotation();



	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintCallable, Category = "Base Character AI|Movement", meta = (DisplayName = "Calculate Grounded Rotation", Keywords = "ALS Character"))
	//virtual void CalculateGroundedRotation();


	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Get Mapped Speed Fast", Keywords = "ALS Character"))
	//virtual float GetMappedSpeedFast();


	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Can Sprint Fast", Keywords = "ALS Character"))
	//bool CalcCanSprint();
	//virtual bool CalcCanSprint_Implementation();


	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintCallable, Category = "Base Character AI|Movement", meta = (DisplayName = "Update Dynamic Movement Settings Fast", Keywords = "ALS Character"))
	//virtual void UpdateMovementSettings(CALS_Gait AllowedGait, FCALSMovementSettings CurrentMovement);


	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Get Allowed Gait Fast", Keywords = "ALS Character"))
	//CALS_Gait GetAllowedGaitFast();
	//virtual CALS_Gait GetAllowedGaitFast_Implementation();


	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Get Actual Gait Fast", Keywords = "ALS Character"))
	//CALS_Gait GetActualGaitFast(CALS_Gait AllowedGait);
	//virtual CALS_Gait GetActualGaitFast_Implementation(CALS_Gait AllowedGait);


	/*A function designed to update the values ​​of variables such as:
	- AccelerationC (Vector)
	- MovementSpeedDifferenceC (float)
	- SpeedC (float)
	- IsMovingC (bool)
	- MovementInputAmountC (float)
	- HasMovementInputC (bool)
	- AimYawRateC (float)
	- IsSwimmingC (bool)
	- LastMovementInputRotationC (Rotator)
	- LastVelocityRotationC (Rotator)
	- PreviousVelocityC (Vector)
	- PreviousAimYawC (float)*/
	UFUNCTION(BlueprintCallable, Category = "Base Character AI|Movement", meta = (DisplayName = "Update Essential Movement Values", Keywords = "Movement,CMC"))
	void UpdateEssentialMovementValues(float InDelta);

	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base Character AI|Movement", meta = (ForceAsFunction, DisplayName = "Update Grounded Rotation", Keywords = "ALS Character"))
	void UpdateGroundedRotation(); virtual void UpdateGroundedRotation_Implementation();

	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Base Character AI|Movement", meta = (ForceAsFunction, DisplayName = "Update InAir Rotation", Keywords = "ALS Character"))
	void UpdateInAirRotation(); virtual void UpdateInAirRotation_Implementation();

	/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
	UFUNCTION(BlueprintPure, Category = "Base Character AI|Movement", meta = (DisplayName = "Get Grounded Rotation Update Rate", Keywords = "ALS Character"))
	virtual float GetGroundedRotationUpdateRate();
	virtual float GetMappedSpeedFast();

};
