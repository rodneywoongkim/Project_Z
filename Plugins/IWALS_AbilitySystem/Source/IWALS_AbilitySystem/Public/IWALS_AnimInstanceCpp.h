// Jakub W

#pragma once

#include "CoreMinimal.h"
#include "IWALS_EnumsAndStruct.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "CharacterFocusingComponent.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "IWALS_AnimInstanceCpp.generated.h"

/**
 * 
 */
UCLASS()
class IWALS_ABILITYSYSTEM_API UIWALS_AnimInstanceCpp : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	AGLS_WalkingType CurrentWalkingType = AGLS_WalkingType::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	AGLS_RunningType CurrentRunningType = AGLS_RunningType::Jog;


	/* Zdefiniuj podstawowe zmienne, które będą potrzebne dla systemu Overlay States. żeby nie odwoływać się do klasy ALS_AnimBP przy pomocy Property Access utworzony został właśnie
	Anim Instance. W nim zdefiniowane są potrzebne zmienne, więc dzięki temu odwoływać się będziemy właśnie do tej klasy a nie całego ALS_AnimBP */


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		float LandPredictionC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Bow System", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool IsHeldArrowC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Bow System", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool IsHaveArrowsC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Shooting", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool AddRecoilImpulseC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Combat", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	int CombatStateIndexC = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	float SecondaryMotionMaskC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	int OverlayOverrideStateC = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	float BlendOverlayWithCoverModeC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FVector2D CoverCrouchWithDirectionC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FVector RelativeAccelerationAmoutC = FVector(0, 0, 0);

	//Foots IK Variables

	//Ragdoll

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Ragdoll", meta = (AllowPrivateAccess = "True"))
		float FlailRateC = 0.0;

	//Aiming

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FRotator SpineRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FRotator SmoothedAimingRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FVector2D AimingAngleC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FVector2D SmoothedAimingAngleC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		float AimSweepTimeC = 0.5;

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		float ForwardYawTimeC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FVector AnimPrepertiesCustomC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Aiming Values", meta = (AllowPrivateAccess = "True"))
		FTransform NeckTransformFromSnapshot = FTransform::Identity;

	//Anim Graph - Grounded

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		HipsDirectionC TrackedHipsDirectionC = HipsDirectionC::F;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		bool ShouldMoveC = false;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//bool PivotC = false;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//bool PivotPlayingC = false;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//bool PlayStopMovementTransitionC = false;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//bool FinishStopTransitionC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		bool Rotate_LC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		bool Rotate_RC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		float RotateRateC = 0.0;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//float RotationScaleC = 0.0;

	//UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		//float DiagonalScaleAmoutC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		FVelocityBlendC VelocityBlend = {};

	UPROPERTY(BlueprintReadWrite, Category = "Anim Graph - Grounded", meta = (AllowPrivateAccess = "True"))
		FLeanAmoutC LeanAmountC = {};

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	float RootYawChangeSpeed = 0.0;




	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		bool JumpedC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		float JumpPlayRateC = 1.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		float FallSpeedC = 0.0;



	//-------------------------------------------     For Motion Matching    ------------------------------------------------------
		 
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool OffsetRootBoneEnabledC = true;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool IsMovingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool HasMovementInputC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		bool JustLandedC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool OnStairsC = false;



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool IsTurnInPlaceAimingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	float SpeedC = 0.0;



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
		FVector VelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		FVector FutureVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
		FVector VelocityLastFrameC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
		FVector LastNonZeroVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
		FVector AccelerationC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		FVector LandVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		FRotator AimingRotationC = FRotator(0, 0, 0);



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool PickUpLootItemC = false;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool CapsuleCollidingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool InterruptOnDatabaseC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FTransform CharacterTransformC = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FTransform CharacterTransform_LastFrame = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FTransform RootTransformC = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FTransform InteractionTransformC = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|DEPRECATED", meta = (AllowPrivateAccess = "True"))
		FRotator FutureMovementAngleC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	TArray<FName> CurrentDatabaseTags;

	// Added FOR AGLS v1.8
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	bool UseCustomTrajectoryFacing = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FRotator CustomFacingDesiredRotation = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	UCharacterFocusingComponent* FocusingComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True")) // ◀◀◀◀ 𝐔𝐏𝐃𝐀𝐓𝐄𝐃
	FGameplayTagContainer OwnerTagContainer;

};
