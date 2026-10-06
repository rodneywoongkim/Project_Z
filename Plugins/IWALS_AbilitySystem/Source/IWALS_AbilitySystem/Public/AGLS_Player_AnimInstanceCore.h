// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IWALS_AnimInstanceCpp.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Animation/AnimNodeReference.h"
#include "MovementParamsControlComponent.h"
#include "CharacterFocusingComponent.h"
#include "Cpp_TraversalActionComponent.h"
#include "PoseSearchAllMovementCollector.h"
#include "AGLS_Player_AnimInstanceCore.generated.h"

/*
This class is designed to integrate PoseSearch into AnimInstance and fully support AGLS. It includes many declarations 
of functions needed for more precise control of Motion Matching. These include trajectory analysis functions, such as:

- bool GetIsMoving()
- bool GetIsStarting()
- bool GetIsStoping()
- bool GetIsPivoting()
- bool GetShouldTurnInPlace()

Functions mainly prepared for AGLS Player Character, for example:

- void UpdateAimingValues(float SmoothingTime = 10.0, int NumOfSpineBones = 4);
- void UpdateLayeringValues(int32 InLocomotionIndex, bool MakeBasePoseAlphaFromState, float BasePoseInterpSpeed, ...
- void UpdateEssentialValuesSafe();

and many more...

The class also contains a large number of declarations of variables needed in the context of the AGLS project.
NOTE: Parent of this class is UIWALS_AnimInstanceCpp!*/
UCLASS()
class IWALS_ABILITYSYSTEM_API UAGLS_Player_AnimInstanceCore : public UIWALS_AnimInstanceCpp
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	UPoseSearchAllMovementCollector* PoseSearchLocomotionCollector = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	float MaxOffsetRootBoneTranslation = 20.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	float TeleportTreshold = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	int FootPlacementMode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	float HeavyLandSpeedThreshold = -500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FGameplayTagContainer OverridePelvisBlendForThisTags;


	//Odniesienie do Charaktera. Zazwyczej po prostu Cast<ACharacter*>(TryGetPawnOwner())
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	ACharacter* Character = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	float DeltaTimeX = 0.001f;

	/*
	Obecny typ klasy overlay z jakiego korzysta Main AnimInstance. Domyślnie warstwa bazowa związana jest z
	wartością OverlayState czli np: Default, Rifle, Pistol 1H, Bow itd.*/
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	TSubclassOf<UAnimInstance> CurrentMainLayerOverlayClass;

	/*
	Obecny typ klasy overlay warstwy drugiej z jakiego korzysta Main AnimInstance. Domyślnie warstwa druga 
	nakładane jest dodatkowo na MainLayerOverlay i zazwyczaj powiązana jest z wartością LocomotionMode 
	czyli np: Default, Covering, Hostage */
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	TSubclassOf<UAnimInstance> CurrentSecondLayerOverlayClass;

	/*
	Informuje z jakiego typu klasy związanej głównie z IK powinien korzystać MainAnimInstance. Zazwyczaj
	dzieli się to na typ Grounded lub Crawling.*/
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	int SkeletalControlLayerIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	UMovementParamsControlComponent* MovementParamsControlComponent = nullptr;



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	FVector ActorAcceleration = FVector::ZeroVector;

	/*
	Przechowuje aktualną wartość prędkości obecnego podłoża pobranego z CMC. */
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	FVector CurrentFloorVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	float AimYawRate = 0;





	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState MovementStateC = CALS_MovementState::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState PrevMovementStateC = CALS_MovementState::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState LastMovementStateC = CALS_MovementState::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction MovementActionC = CALS_MovementAction::None;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction PrevMovementActionC = CALS_MovementAction::None;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode CurrentRotationMode = CALS_RotationMode::LookingDirection;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode PrevRotationMode = CALS_RotationMode::LookingDirection;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait CurrentGait = CALS_Gait::Walking;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait PrevGait = CALS_Gait::Walking;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait GaitLastFrame = CALS_Gait::Walking;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_Stance CurrentStance = CALS_Stance::Standing;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_Stance PrevStance = CALS_Stance::Standing;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_OverlayState CurrentOverlayState = CALS_OverlayState::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_OverlayState PrevOverlayState = CALS_OverlayState::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|States", meta = (AllowPrivateAccess = "True"))
	CALS_TraversalAction TraversalAction = CALS_TraversalAction::None;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_WalkingType WalkingDatabasesType = AGLS_WalkingType::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_RunningType RunningDatabasesType = AGLS_RunningType::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_SprintingType SprintingDatabasesType = AGLS_SprintingType::Default;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	bool bPrevOnStairs = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations", meta = (AllowPrivateAccess = "True"))
	bool bPrevCapsuleCollidingC = false;



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Landing", meta = (AllowPrivateAccess = "True"))
	bool bPlayHeavyLandMontage = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Hostage", meta = (AllowPrivateAccess = "True"))
	bool bStartHostageMode = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Sliding", meta = (AllowPrivateAccess = "True"))
	bool bStartSliding = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Sliding", meta = (AllowPrivateAccess = "True"))
	float SlidingGroundAngle = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Combat", meta = (AllowPrivateAccess = "True"))
	float MeleeCombatAlpha = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	bool bStartCovering = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	bool bCoverIdleTransition = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	bool bToCoverTransition = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	bool bCoverDirectionChanged = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	AGLS_CoveringDirection CoveringDirection = AGLS_CoveringDirection::Left;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	FVector2D CoverMovingDirection = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Covering", meta = (AllowPrivateAccess = "True"))
	float CoverDesiredMoveDirection = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Pushing", meta = (AllowPrivateAccess = "True"))
	float PushingPostureType = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Pushing", meta = (AllowPrivateAccess = "True"))
	float Pushing_IK_Alpha = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Pushing", meta = (AllowPrivateAccess = "True"))
	FTransform PushingIK_EffectorL = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Pushing", meta = (AllowPrivateAccess = "True"))
	FTransform PushingIK_EffectorR = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Vehicle", meta = (AllowPrivateAccess = "True"))
	bool bIsInVehicle = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Character Informations|Interaction", meta = (AllowPrivateAccess = "True"))
	bool StartInteractionWithDynamicProp = false;






	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FTransformTrajectory Trajectory;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FTransformTrajectory TrajectoryWithoutCollision;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollision;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectoryData TrajData_Idle;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectoryData TrajData_MovingA;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectoryData TrajData_MovingB;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float PreviousDesiredControllerYaw = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PastVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_NearFutureVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_FutureVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PreviousFutureVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FRotator Traj_FutureFacing = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Traj_TurnAngle = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PastAngularVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_CurrentAngularVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	bool Traj_IsCircling = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	bool Traj_IsPivotingInCircleShape = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Traj_CirclingTime = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float FutureFacingDelta = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float FutureFacingDelta_LastFrame = 0.0;





	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	UPoseSearchDatabase* CurrentSelectedDatabase;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	TArray<UPoseSearchDatabase*> ValidDatabases;

	//UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True")) ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▶ Declarated in parent class ◀
	//TArray<FName> CurrentDatabaseTags;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	int MMDatabaseLOD = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	float MMSearchCost = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool ForceReleaseRootOffset = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection = AGLS_MovementDirectionState::F;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection_LastFrame = AGLS_MovementDirectionState::F;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection_Recent = AGLS_MovementDirectionState::F;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool bStanceTransition = false;

	//Specifies whether Motion matching should execute the 'Interrupt On Databases' option after loading databases into memory has been completed successfully.
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool bInterruptOnDatabasesLoadEnd = false;




	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float BasePose_N = 1.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float BasePose_CLF = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀ | By default value come from "Layering_Arm_L_LS" curve
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_ArmL_LS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_ArmL_MS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀ | By default value come from "Layering_Arm_R_LS" curve
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_ArmR_LS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_ArmR_MS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀ | By default value come from "Layering_Hand_L" curve
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_HandL = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀ | By default value come from "Layering_Hand_R" curve
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float Layering_HandR = 0.0;

	/*
	▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	Some actions, such as reloading, have animations prepared only for Stance == Standing, and additionally, during these sequences, it is possible to switch 
	to Crouching. In such a case, Layering_Legs and Layering_Pelvis should be set to 0.0. Therefore, if the 'OverrideBlendPelvis' variable is greater than 0.0, 
	the value of these curves will be modified.

	DEFAULT CODE:
	const bool HasActionTag = OwnerTagContainer.HasAny(OverridePelvisBlendForThisTags);
	OverridePelvisBlending = KML::FInterpTo_Constant(OverridePelvisBlending, BasePose_CLF * HasActionTag, DeltaTimeX, KML::SelectFloat(25, 0.5, HasActionTag));
	*/
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float OverridePelvisBlending = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layering", meta = (AllowPrivateAccess = "True"))
	float BaseLayerAddtiveWeight = 0.0;




	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|IK Control", meta = (AllowPrivateAccess = "True"))
	bool bForceFootPlacementReset = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Procedural", meta = (AllowPrivateAccess = "True"))
	float DistanceToNearestOpponent = -1;


public:

	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool JustTeleport();

	/*
	This function determines if root motion from the animations can be steered by checking if the character is 
	moving or in the air. This prevents idle animations from getting steered, which could cause sliding.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Motion Matching|Steering", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool EnableSteering(const FAnimNodeReference& Node);
	virtual bool EnableSteering_Implementation(const FAnimNodeReference& Node);

	/*
	This function gives the steering node a target rotation. This target is calculated using the future facing direction 
	from the predicted trajectory. This allows the steering node to rotate toward a future direction, rather than always 
	steering toward the current actor rotation, which could cause it to lag too far behind.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Motion Matching|Steering", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	FQuat GetDesiredFacing(FAnimNodeReference Node);
	virtual FQuat GetDesiredFacing_Implementation(FAnimNodeReference Node);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Motion Matching|Steering", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float GetProceduralTargetTime(FAnimNodeReference Node);
	virtual float GetProceduralTargetTime_Implementation(FAnimNodeReference Node);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Motion Matching", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	void UpdateMotionMatching(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	virtual void UpdateMotionMatching_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Motion Matching", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	void UpdateMotionMatching_PostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	virtual void UpdateMotionMatching_PostSelection_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	This function is used to change the blend time of the Motion Matching node, based on the current and previous states. 
	In the future, we plan to allow blend times to be more directly set from the chosen databases.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Motion Matching", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float Get_MMBlendTime();
	virtual float Get_MMBlendTime_Implementation();

	/*
	Look at the future velocity (determined by trajectory generation) to determine if the character is trying to move 
	(future velocity is greater than 0), or trying to stop (future velocity is 0).*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Moving", Keywords = "Motion,Matching,Movement"))
	bool GetIsMoving(); virtual bool GetIsMoving_Implementation();

	/*
	This function is used to determine if the character is starting to move by checking if the future velocity is greater than the 
	current velocity. If the current Database asset is a pivot database, this function will always return false. This prevents the 
	Motion Matching system from interrupting a pivot, since the second half of a pivot is very similar to a start.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Starting", Keywords = "Motion,Matching,Movement"))
	bool GetIsStarting(); virtual bool GetIsStarting_Implementation();

	/*
	This function is used to determine if the character is starting to move by checking if the future velocity is greater than the 
	current velocity. If the current Database asset is a pivot database, this function will always return false. This prevents the 
	Motion Matching system from interrupting a pivot, since the second half of a pivot is very similar to a start.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Stoping", Keywords = "Motion,Matching,Movement"))
	bool GetIsStoping(); virtual bool GetIsStoping_Implementation();

	/*
	This function is used to determine if the character is pivoting by checking if the character’s future trajectory is moving 
	in a much different direction than the character’s current trajectory.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Pivoting", Keywords = "Motion,Matching,Movement"))
	bool GetIsPivoting(); virtual bool GetIsPivoting_Implementation();

	/*
	Same functionality as GetIsPivoting() but required smaller turn angle*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Pivoting Small Delta", Keywords = "Motion,Matching,Movement"))
	bool GetIsPivotingSmallDelta(); virtual bool GetIsPivotingSmallDelta_Implementation();

	/*
	This function is used to determine if the character is turning in place by checking if the Future Facing Delta is greater than 
	50 degrees while the character is not moving.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Should Turn In Place", Keywords = "Motion,Matching,Movement"))
	bool GetShouldTurnInPlace(); virtual bool GetShouldTurnInPlace_Implementation();

	/*
	If the root bone rotation and character’s capsule rotations are very different while moving, this function will allow a spin 
	transition animation to play. Spin transitions are locomotion animations that rotate the character while moving in a fixed 
	world direction, and are useful when switching rotation modes. 
	For example, if the character is running toward the camera using the Orient to Movement mode and then switching to strafe, 
	this would require the character to spin 180 degrees very quickly. A spin transition animation would be an ideal transition 
	for this gameplay scenario. Currently, we are using refacing starts in place of spin transitions, but plan to provide actual 
	spin transition data in a future release.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Should Spin Transition", Keywords = "Motion,Matching,Movement"))
	bool GetShouldSpinTransition(); virtual bool GetShouldSpinTransition_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Is Pivoting In Circle Shape", Keywords = "Motion,Matching,Movement"))
	bool GetIsPivotingInCircleShape(); virtual bool GetIsPivotingInCircleShape_Implementation();

	/*
	This function is used to select the tail end of traversal animations when blending back to locomotion. For example, if the 
	MovingTraversal anim curve value is greater than 1, and the default slot is NOT active (slots are not active when blending out), 
	the character must be blending out from a moving traversal action, therefore this function will return true. The chooser then 
	allows the Motion Matching node to select from the "FromTraversal" databases for a seamless followthrough.*/
	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool JustTraversed();

	/*
	If the character has just landed and the land velocity was less than the heavy land speed threshold, play light land animations.*/
	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool JustLanded_Light();

	/*
	If the character has just landed and the land velocity was greater than the heavy land speed threshold, play heavy land animations.*/
	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool JustLanded_Heavy();

	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float GetLandVelocity();

	UFUNCTION(BlueprintPure, Category = "Movement Analysis", meta = (BlueprintThreadSafe, DisplayName = "Get Is Colliding", Keywords = "Motion,Matching,Movement"))
	bool GetIsColliding();



	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float Get_TrajectoryTurnAngle(); virtual float Get_TrajectoryTurnAngle_Implementation();

	/*
	This function tracks the total amount of rotational delta between the root bone and the future facing rotation. Intermediate rotations are 
	used so that we can calculate deltas beyond 180 degrees. This value is used heavily in the animation choosers to select refacing animations, 
	and values of beyond 180 prevent us from selecting animations that rotate in the wrong direction. For instance, the trajectory may be 
	rotating 200 total degrees to the right during a turn. If we just get the delta between the root and future, we would get -160, which would 
	not be reflective of how the pawn is actually rotating and would lead to bad animation selection.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float Get_TrajectoryFacingDelta(const TArray<float>& Times, FRotator RootRotation); virtual float Get_TrajectoryFacingDelta_Implementation(const TArray<float>& Times, FRotator RootRotation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Trajectory,Character"))
	void GenerateTrajectory(); virtual void GenerateTrajectory_Implementation();



	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (ForceAsFunction, Keywords = "Essential,Initialzie,Begin"))
	void OnInstanceInitializeNotSafe(); virtual void OnInstanceInitializeNotSafe_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (BlueprintThreadSafe, Keywords = "Essential,Core"))
	void UpdateEssentialValuesSafe(); virtual void UpdateEssentialValuesSafe_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (BlueprintThreadSafe, Keywords = "Essential,Layering,Blending", AdvancedDisplay = 2))
	void UpdateLayeringValues(int32 InLocomotionIndex, bool MakeBasePoseAlphaFromState, float BasePoseInterpSpeed, float BendDownAlphaStrength = 0.8);
	virtual void UpdateLayeringValues_Implementation(int32 InLocomotionIndex, bool MakeBasePoseAlphaFromState, float BasePoseInterpSpeed, float BendDownAlphaStrength = 0.8);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (BlueprintThreadSafe, Keywords = "Essential,Aiming,Character", AdvancedDisplay = 1))
	void UpdateAimingValues(float SmoothingTime = 10.0, int NumOfSpineBones = 4, bool UseSpineRotSmoothing = true, bool UseRootRotationAsRef = false);
	virtual void UpdateAimingValues_Implementation(float SmoothingTime = 10.0, int NumOfSpineBones = 3, bool UseSpineRotSmoothing = true, bool UseRootRotationAsRef = false);



	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (Keywords = "Essential,Aiming,Character"))
	void AsyncLoadOverlayEvent(CALS_OverlayState NewState); virtual void AsyncLoadOverlayEvent_Implementation(CALS_OverlayState NewState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (Keywords = "Essential,Aiming,Character"))
	void AsyncLoadSecondOverlayEvent(); virtual void AsyncLoadSecondOverlayEvent_Implementation();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AGLS Core", meta = (ForceAsFunction, Keywords = "Essential,Initialzie,Begin"))
	bool CalculateIsCollidingValue(float SpeedScaleTollerance = 0.3, float TimeDilatation = 1.0, bool bDrawTrace = false); 
	virtual bool CalculateIsCollidingValue_Implementation(float SpeedScaleTollerance = 0.3, float TimeDilatation = 1.0, bool bDrawTrace = false);



};
