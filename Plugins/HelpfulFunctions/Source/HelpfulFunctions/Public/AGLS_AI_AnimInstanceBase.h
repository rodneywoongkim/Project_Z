

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Animation/AnimInstance.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "PoseSearchAllMovementCollector.h"
#include "Animation/AnimNodeReference.h"
#include "MovementParamsControlComponent.h"
#include "AGLS_AI_CharacterInterface.h"
#include "ALS_HumanAI_InterfaceCpp.h"
#include "AGLS_AI_HumanCharInterface.h"
#include "AGLS_AI_AnimInstanceBase.generated.h"


struct FAGLS_CharacterParamsCollected : public FTableRowBase
{
	int MovementModeIndex = 0;
	CALS_MovementState MovementState;
	CALS_MovementAction MovementAction;
	CALS_RotationMode RotationMode;
	CALS_Gait Gait;
	CALS_Stance Stance;
	CALS_OverlayState OverlayState;
	int LocomotionModeIndex;
	FName LocomotionModeName;
	bool IsColliding = false;
	bool WalkOnStairs = false;
	bool IsInVehicle = false;
	bool HasRootMotion = false;
	AGLS_LOD_State LOD_State;
};


USTRUCT(BlueprintType)
struct FAGLS_MotionExtractionForTrajectory : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration Generating", meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float RootExtractionConstTimeOffset = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration Generating", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float RootExtractionDynamicTimeOffset = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration Generating")
	float RootRotationExtractionOffset = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration Generating")
	int RootRotationExtractionFramesNum = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity Generating", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float VelocityScale = 1.0;
};


/*
Main C++ class prepared for configuring PoseSearch Motion Matching, along with functionality intended for Human AI in the AGLS project.
It contains a large list of variables and functions, many of which can be overridden in Blueprint.*/
UCLASS()
class HELPFULFUNCTIONS_API UAGLS_AI_AnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   Config
#pragma region CONFIG VARIABLES

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	bool OffsetRootBoneEnabledC = true;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	UPoseSearchAllMovementCollector* PoseSearchLocomotionCollector = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectoryData TrajectoryConfigIdle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectoryData TrajectoryConfigMoving;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True", EditCondition = "OffsetRootBoneEnabledC", ClampMin = "0", ClampMax = "100"))
	float MaxOffsetRootBoneTranslation = 20.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True", ClampMin = "0", ClampMax = "10"))
	float ReadyStateDuration = 3.0;

	//⚠︎ X = Walk, Y = Run/Jog, Z = Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FVector IsStartingVelocityBiasPerGait = FVector(20,100,100);

	//⚠︎ X = Walk, Y = Run/Jog, Z = Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FVector IsStartingPastVelocityMax = FVector(150, 100, 100);

	//⚠︎ X = Walk, Y = Run/Jog, Z = Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FVector IsPivotingDeltaTrigger = FVector(30,60,40);

	//⚠︎ ONLY DEFAULT VALUE - In final code this value can be skipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True", ClampMin = "0", ClampMax = "300"))
	float IsSpinningDeltaTrigger = 120.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True", ClampMin = "5", ClampMax = "160"))
	float TurnInPlaceFacingDeltaTreshold = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	float HeavyLandSpeedThreshold = -500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True", ClampMin = "0.1", ClampMax = "2"))
	float BlendStackTimeBlendMultiply = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	float RootOffsetInterpSpeedMultiply = 1.0;

	//AGLS v1.9.1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	bool bUseTurnsForAimingAsMontages = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config", meta = (AllowPrivateAccess = "True"))
	FAGLS_MotionExtractionForTrajectory RootMotionTrajectorySolverConfig;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True"))
	FName FootIK_L_CurveName = TEXT("Enable_FootIK_L");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True"))
	FName FootIK_R_CurveName = TEXT("Enable_FootIK_R");

	//When this value is true the alpha for single foot trace equal 1 - FootIK_L/R_CurveName
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True"))
	bool bFootEnableCurvesAsDisableMode = true;

	//NOT FINISHED
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True"))
	bool bCanUseFootsLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True", EditCondition = "bCanUseFootsLock"))
	FName FootSpeedCurveName_L = "none";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True", EditCondition = "bCanUseFootsLock"))
	FName FootSpeedCurveName_R = "none";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimInstCore|Config|IK Settings", meta = (AllowPrivateAccess = "True"))
	float FootTraceRadius = 0.1f;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   References
#pragma region REFERENCES VARIABLES
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	ACharacter* CharacterC = nullptr;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	UMovementParamsControlComponent* MovementParamsControlComponent = nullptr;

	UCharacterMovementComponent* MovementComp = nullptr;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	float dt = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|References", meta = (AllowPrivateAccess = "True"))
	float TimeDilatationC = 1.0;

	float timer1 = 0.0;
	float timer2 = 0.0;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   STATES VALUES AS INDEX
#pragma region STATES VARIABLES

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState MovementStateC = CALS_MovementState::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState PrevMovementStateC = CALS_MovementState::Grounded;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementState LastMovementStateC = CALS_MovementState::Grounded;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait GaitC = CALS_Gait::Running;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait PrevGait = CALS_Gait::Walking;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_Gait GaitLastFrame = CALS_Gait::Walking;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction MovementActionC = CALS_MovementAction::None;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction PrevMovementActionC = CALS_MovementAction::None;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_OverlayState OverlayStateC = CALS_OverlayState::Default;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode RotationModeC = CALS_RotationMode::LookingDirection;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode PrevRotationMode = CALS_RotationMode::LookingDirection;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_Stance StanceC = CALS_Stance::Standing;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_Stance PrevStance = CALS_Stance::Standing;

	//⚠︎ DEPRECATED VARIABLE - AGLS v1.9
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CALS_GroundedMoveMode GroundedMoveModeC = CALS_GroundedMoveMode::Normal;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	CMC_ActionTypeC ClimbingActionStateC = CMC_ActionTypeC::None;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	int LocomotionModeIndex = 0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|States", meta = (AllowPrivateAccess = "True"))
	int PrevLocomotionModeIndex = 0;


	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_WalkingType WalkingDatabasesType = AGLS_WalkingType::Default;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_RunningType RunningDatabasesType = AGLS_RunningType::Default;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_SprintingType SprintingDatabasesType = AGLS_SprintingType::Default;


#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   ---> Essensial VALUES <---
#pragma region ESSENTIAL VARIABLES

	FAGLS_CharacterParamsCollected CollectedCharacterParams;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	FGameplayTagContainer OwnerTagsContainer;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	bool ShouldMoveC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	bool PickUpLootItemC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	bool IsTurnInPlaceAimingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	bool IsCoveringC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	bool LeftSideCoverC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	bool CoverDirectionChangedC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	float DetectedEnemyTime = 0.0;

	//By default this value is collected from OwnedPawn. 
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Collected Values", meta = (AllowPrivateAccess = "True"))
	bool IsDeadC = false;

	//Speed value is a simple formula based on current Velocity value. SpeedC = Velocity.SizeXY()
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	float SpeedC = 0.0;

	//By Default this value is updated in UpdateEssentialValues() as VelocityC = MovementComp->Velocity;
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector VelocityC = FVector(0, 0, 0);

	//AGLS v1.9 Velocity calculated for Trajectory Generator
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector VelocityGenerated = FVector::ZeroVector;

	//AGLS v1.9 Acceleration calculated for Trajectory Generator
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector AccelerationGenerated = FVector::ZeroVector;

	//AGLS v1.9 This value is required to calculate physical acceleration but for Trajectory Generator input.
	// POSESEARCH_FUNCTIONS::MakeAccelerationValueForTrajectory(OutPlayingRootAnim, OutMontageFacing, this, dt, AccelerationGenerated, true, 0.03, 0.1);
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector PrevActorPosition = FVector::ZeroVector;

	//By Default this value is calculated in GenerateTrajectory_Implementation() and the code is: 
	//FutureVelocityC = (Trajectory.GetSampleAtTime(0.5, false).Position - Trajectory.GetSampleAtTime(0.4, false).Position) * 10.0;
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector FutureVelocityC = FVector(0, 0, 0);

	//By Default this value is updated in UpdateEssentialValues() as VelocityLastFrameC = VelocityC before VelocityC = MovementComp->Velocity;
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector VelocityLastFrameC = FVector(0, 0, 0);

	//Simple velocity holding variable but value is only updated when OwnedPawn current has velocity.Lengh() > Moving_Treshold
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector LastNonZeroVelocityC = FVector(0, 0, 0);

	//Physical Acceleration Value. Calculated using formula: (Velocity[n] - Velocity[n - 1]) / dt
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True", DisplayName = "Physical Acceleration"))
	FVector AccelerationC = FVector(0, 0, 0);

	/*✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic. VALUE updated before catching acceleration from CharacterMovementComponent and saved to MovementAcceleration*/
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector AccelerationLastFrame = FVector(0, 0, 0);

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	//By Default this value is updated in UpdateEssentialValues() as AccelerationC = MovementComp->GetCurrentAcceleration();
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector MovementAcceleration = FVector(0, 0, 0);

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic. This value is holding physical calculated acceleration and unrotated by actual RootTransform (as ActorTransform)
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector RelativeAccelerationAmout = FVector(0, 0, 0);

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic. Basicly can be used to set custom acceleration value for Trajectory Generator. By Default this value is not important
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector CustomAcceleration = FVector(0, 0, 0);

	//Saved velocity during landing
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FVector LandVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FRotator AimingRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FRotator PrevAimingRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	FRotator RotationVelocityC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Essential Values", meta = (AllowPrivateAccess = "True"))
	float HitReactionStrenghtC = 0.0;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒    ---> Movement Analize <---
#pragma region MOVEMENT ANALIZE VARIABLES

	//IsMoving variable by default should return 'true' when CMC->GetCurrentAcceleration() != ZERO AND Pawn->GetVelocity() != ZERO
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool IsMovingC = false;

	//Same value as IsMovingC, but should be updated before  IsMovingC to correctly detect value change frame
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool PrevIsMovingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool HasMovementInputC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool CapsuleCollidingC = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool bPrevCapsuleCollidingC = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	//Default value = CharacterC->HasAnyRootMotion() && SpeedC > 10.0;
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool bHasAnyRootMotion = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool bPrevHasAnyRootMotion = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool JustLandedC = false;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool OnStairsC = false;

	//Important value for correctly blending two stances mode when using PoseSearch. When is true MotionMatching should 
	// have more Databases including transitions between stances
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Movement Analize", meta = (AllowPrivateAccess = "True"))
	bool StanceTransitionC = false;
	FTimerHandle TimerHandle_StanceTransition;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒    ---> M O T I O N   M A T C H I N G <---
#pragma region TRAJECTORY VARIABLES

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FTransformTrajectory Trajectory;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FTransformTrajectory TrajectoryWithoutCollision;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollision;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float PreviousDesiredControllerYaw = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PastVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_NearFutureVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_FutureVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PreviousFutureVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FRotator Trj_FutureFacing = FRotator::ZeroRotator;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Trj_TurnAngle = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_PastAngularVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	FVector Trj_CurrentAngularVelocity = FVector::ZeroVector;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	bool Trj_IsCircling = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	bool Trj_IsPivotingInCircleShape = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Trj_CirclingTime = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Trj_FutureFacingDelta = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Trajectory", meta = (AllowPrivateAccess = "True"))
	float Trj_FutureFacingDelta_LastFrame = 0.0;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ 
#pragma region MOTION MATCHING VARIABLES

	//UPROPERTY(BlueprintReadWrite, Category = "Motion Matching", meta = (AllowPrivateAccess = "True"))
	//FTransformTrajectory Trajectory;

	// THIS VALUE is mostly declarated for debuging processing
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	UPoseSearchDatabase* CurrentSelectedDatabase;

	//Should return current tags array from current active PoseSearchDatabase
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	TArray<FName> CurrentDatabaseTags;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	// THIS VALUE is mostly declarated for debuging processing
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	TArray<UPoseSearchDatabase*> ValidDatabases;

	/*✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	A variable declared to properly handle an experimental feature related to asynchronous loading of PoseSearchDatabase assets required by Motion Matching*/
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	TArray<TSoftObjectPtr<UPoseSearchDatabase>> RequiredToLoadDatabases;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool CorrentyAnyDatabaseIsLoading = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	int MMDatabaseLOD = 0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	// THIS VALUE is mostly declarated for debuging processing
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	float MMSearchCost = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection = AGLS_MovementDirectionState::F;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection_LastFrame = AGLS_MovementDirectionState::F;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	AGLS_MovementDirectionState MovementDirection_Recent = AGLS_MovementDirectionState::F;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	float MovementDirection_Time = 0.0;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	float MovementDirection_LastStateTime = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool InterruptedOnDatabaseC = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	//Specifies whether Motion matching should execute the 'Interrupt On Databases' option after loading databases into memory has been completed successfully.
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool bInterruptOnDatabasesLoadEnd = false;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	bool ForceReleaseRootOffset = false;



	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FTransform CharacterTransformC = FTransform::Identity;

	//✔︎ Declarated for AGLS v1.9 - Rebuilding AnimInstances logic
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FTransform CharacterTransformLastFrame = FTransform::Identity;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FTransform RootTransformC = FTransform::Identity;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FTransform PrevRootTransform;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FTransform InteractionTransformC = FTransform::Identity;


	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Motion Matching", meta = (AllowPrivateAccess = "True"))
	FRotator FutureMovementAngleC = FRotator(0, 0, 0);

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒   --->  (A) (I) (M) (I) (N) (G)  <---
#pragma region AIMING VARIABLES

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	FRotator SpineRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	FRotator SmoothedAimingRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	FVector2D AimingAngleC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	FVector2D SmoothedAimingAngleC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	float AimSweepTimeC = 0.5;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	float InputYawOffsetTimeC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	float ForwardYawTimeC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	float AimYawRateC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Aiming Values", meta = (AllowPrivateAccess = "True"))
	float RootYawChangeSpeedC = 0.0;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒    --->  |L|A|Y|E|R|  |B|L|E|N|D|I|N|G|  <---
#pragma region LAYER BLENDING VARIABLES

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float StrideBlendC = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float BasePoseN = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float BasePoseCLF = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float ArmL_LS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float ArmL_MS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float ArmR_LS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float ArmR_MS = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float Hand_L = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float Hand_R = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float SecondaryMotionMaskC = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float DesiredBendDownAlpha = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float BendDownAlphaC = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	int OverlayOverrideStateC = 0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float OverlayStateElapsedTime = 0.0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float EnableHandIK_L = 0;

	//▶ 𝐋𝐀𝐘𝐄𝐑𝐒 𝐁𝐋𝐄𝐍𝐃𝐈𝐍𝐆 ◀
	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	float EnableHandIK_R = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Layer Blending", meta = (AllowPrivateAccess = "True"))
	CALS_OverlayPosesType OverlayPosesType = CALS_OverlayPosesType::Relaxed;

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒    --->  [F] [O] [O] [T] [S]   [I]. [K]. <---
#pragma region INVERSE KINEMATIC VARIABLES

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	float FootLockL_AlphaC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	float FootLockR_AlphaC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	FVector FootOffset_L_LocC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	FRotator FootOffset_L_RotC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	FVector FootOffset_R_LocC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	FRotator FootOffset_R_RotC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	FVector PelvisOffsetC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Foot IK", meta = (AllowPrivateAccess = "True"))
	float PelvisOffsetAlphaC = 0.0;

	FVector FootLock_L_Location = FVector(0, 0, 0);
	FVector FootLock_R_Location = FVector(0, 0, 0);

#pragma endregion

//▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Optimization
#pragma region OPTIMALIZATION VARIABLES

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Optimization", meta = (AllowPrivateAccess = "True"))
	AGLS_LOD_State LOD_State = AGLS_LOD_State::LOD0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Optimization", meta = (AllowPrivateAccess = "True"))
	AGLS_LOD_State PoseSearchLOD_State = AGLS_LOD_State::LOD0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Optimization", meta = (AllowPrivateAccess = "True"))
	AGLS_LOD_State AnimGraphDetailsLOD_State = AGLS_LOD_State::LOD0;

	UPROPERTY(BlueprintReadWrite, Category = "AnimInstCore|Optimization", meta = (AllowPrivateAccess = "True"))
	bool LOD_ToUp = false;

#pragma endregion



/*██████████████████████████████████████████████████ 🅵🆄🅽🅲🆃🅸🆀🅽🆂 ██████████████████████████████████████████████████
█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████*/

public:

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	void CreateOverlayPosesModeState();

#pragma region THREAD SAFE LOGIC

	/*This function is responsible for generating a trajectory and storing values ​​in TrajectoryWithoutCollision and Trajectory. 
	By default, this function is also responsible for setting the values ​​of variables with the "Trj_" prefix, e.g., 
	Trj_PastVelocity, Trj_NearFutureVelocity, Trj_FutureVelocity*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Generate Trajectory", Keywords = "Motion Matching", BlueprintThreadSafe, AdvancedDisplay = 1))
	FTransformTrajectory GenerateTrajectory(float HistoryInterval = 0.01, int HistoryCount = 30, float InPredictionInterval = 0.1, int PreditionCount = 15, bool UseDirectionStateCorrection = true, float DiectionCorrectionTreshold = 100, int LerpingFacingSamplesNum = 8);
	virtual FTransformTrajectory GenerateTrajectory_Implementation(float HistoryInterval = 0.01, int HistoryCount = 30, float InPredictionInterval = 0.1, int PreditionCount = 15, bool UseDirectionStateCorrection = true, float DiectionCorrectionTreshold = 100, int LerpingFacingSamplesNum = 8);


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Anim Instance Core", meta = (DisplayName = "Get Override Pre Sim Trajectory Facing", Keywords = "Motion Matching", BlueprintThreadSafe))
	void Get_OverrideTrajectoryFacing(bool& ShoundOverride, float& ReturnLookingYaw, FQuat& ReturnFacing);
	virtual void Get_OverrideTrajectoryFacing_Implementation(bool& ShoundOverride, float& ReturnLookingYaw, FQuat& ReturnFacing);



	/*if (!TryGetPawnOwner()) return;
	if (!MovementComp) return;
	CharacterTransformLastFrame° = CharacterTransformC;
	CharacterTransformC° = TryGetPawnOwner()->GetTransform();	

	const FTransform NodeRootTransform = GetRootTransformFromNodeRef();
	RootTransformC° = FTransform(
			FRotator(NodeRootTransform.Rotator().Pitch, NodeRootTransform.Rotator().Yaw + 90, NodeRootTransform.Rotator().Roll),
			NodeRootTransform.GetLocation(),
			NodeRootTransform.GetScale3D());

	AccelerationLastFrame° = MovementAcceleration;
	MovementAcceleration° = MovementComp->GetCurrentAcceleration();
	if (bHasAnyRootMotion) { MovementAcceleration = AccelerationGenerated; } ✔︎𝐍𝐎𝐓𝐄 - For Solving PoseSearch durning RootMotion
	const float RotationAmout = KML::SafeDivide(MovementAcceleration.Length(), MovementComp->GetMaxAcceleration());

	VelocityLastFrameC° = VelocityC;
	VelocityC° = MovementComp->Velocity;
	if (bHasAnyRootMotion) { VelocityC = VelocityGenerated; } ✔︎𝐍𝐎𝐓𝐄 - For Solving PoseSearch durning RootMotion animation
	SpeedC° = VelocityC.Size2D();
	const bool HasVelocity = SpeedC > 5.0;
	if (HasVelocity) LastNonZeroVelocityC° = VelocityC;

	AccelerationC° = (VelocityC - VelocityLastFrameC) / KML::FMax(this->GetDeltaSeconds(), 0.0005);
	RelativeAccelerationAmout° = KML::Quat_UnrotateVector(RootTransformC.GetRotation(), AccelerationC);

	if (CurrentSelectedDatabase) { CurrentDatabaseTags° = CurrentSelectedDatabase->Tags; }

	IsMovingC° = !MovementComp->GetCurrentAcceleration().Equals(FVector::ZeroVector, 0.1) && SpeedC > 2;*/
	UFUNCTION(BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Update Essential Values", Keywords = "Motion Matching", BlueprintThreadSafe))
	void UpdateEssentialValues();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Get Root Transform From Node Ref", Keywords = "Motion Matching", BlueprintThreadSafe))
	FTransform GetRootTransformFromNodeRef();
	virtual FTransform GetRootTransformFromNodeRef_Implementation();


	/*const FRotator RotationDeltaBase = FRotator(0, CharacterTransformC.Rotator().Yaw + GetCurveValue("AimingRotationOffset"), 0);
	SmoothedAimingRotationC° = KML::RInterpTo(SmoothedAimingRotationC, AimingRotationC, this->GetDeltaSeconds(), 10.0);

	//Calculate the Aiming angle and Smoothed Aiming Angle by getting the delta between the aiming rotation and the actor rotation.
	const FRotator DeltaRot = KML::NormalizedDeltaRotator(AimingRotationC, RotationDeltaBase);
	const FRotator DeltaRotSmooth = KML::NormalizedDeltaRotator(SmoothedAimingRotationC, RotationDeltaBase);
	AimingAngleC° = FVector2D(DeltaRot.Yaw, DeltaRot.Pitch);
	SmoothedAimingAngleC° = FVector2D(DeltaRotSmooth.Yaw, DeltaRotSmooth.Pitch);

	const float DetectedEnemyTimeValue = DetectedEnemyTime;
	AimSweepTimeC° = KML::MapRangeClamped(AimingAngleC.Y, -90, 90, 1.0, 0.0);
	const float R = KML::ClampAngle(KML::Lerp(SmoothedAimingAngleC.X, AimingAngleC.X, DetectedEnemyTimeValue), -89, 89);
	SpineRotationC.Yaw° = R / 4;*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Update Rotation Values", Keywords = "Motion Matching", BlueprintThreadSafe, AdvancedDisplay = 1))
	void UpdateRotationValues(float SmoothingTime = 10, int NumOfSpineBones = 3, bool UseSmoothOnSpineRotation = false, float SpineRotSmoothMinSpeed = 50, bool UseRootBoneAsRotationRef = false);
	virtual void UpdateRotationValues_Implementation(float SmoothingTime = 10, int NumOfSpineBones = 3, bool UseSmoothOnSpineRotation = false, float SpineRotSmoothMinSpeed = 50, bool UseRootBoneAsRotationRef = false);


	/*const FVector AccelerationXY = FVector(MovementComp->GetCurrentAcceleration().X, 
	MovementComp->GetCurrentAcceleration().Y, 0.0);
	if (AccelerationXY.Length() > 5.0)
	{
		FVector AccDirection = AccelerationXY; AccDirection.Normalize();
		float DotValue = KML::Dot_VectorVector(CharacterC->GetActorRightVector(), AccDirection);
		if (abs(DotValue) > 0.02)
		{
			PrevCoverDirection = LeftSideCoverC;
			LeftSideCoverC = DotValue < 0.0;
			if (LeftSideCoverC != PrevCoverDirection)
			{
				CoverDirectionChangedC = true;
				timer2 = 0.25;
	}}}*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Update Cover Values", Keywords = "Motion Matching", BlueprintThreadSafe))
	void UpdateCoverValues();
	virtual void UpdateCoverValues_Implementation();


	/*BasePoseN = GetCurveValue(TEXT("BasePose_N"));
	BasePoseCLF = GetCurveValue(TEXT("BasePose_CLF"));

	Hand_R = GetCurveValue(TEXT("Layering_Hand_R"));
	Hand_L = GetCurveValue(TEXT("Layering_Hand_L"));

	ArmR_LS = GetCurveValue(TEXT("Layering_Arm_R_LS"));
	ArmL_LS = GetCurveValue(TEXT("Layering_Arm_L_LS"));

	ArmR_MS = (1 - UKismetMathLibrary::FFloor(ArmR_LS)) * 1.0;
	ArmL_MS = (1 - UKismetMathLibrary::FFloor(ArmL_LS)) * 1.0;*/
	UFUNCTION(BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Update Layering Values", Keywords = "Motion Matching", BlueprintThreadSafe, AdvancedDisplay = 1))
	void UpdateLayeringValues(bool MakeBasePoseAlphaFromState = false, float BasePoseInterpSpeed = 6.0);

	/*By Default function mainly update states variables including new and previous value.
	Main States to update list:

	➊ PrevMovementStateC = MovementStateC
	- MovementStateC = CollectedCharacterParams.MovementState

	➋ PrevMovementActionC = MovementActionC
	- MovementActionC = CollectedCharacterParams.MovementAction

	➌ PrevRotationMode = RotationModeC
	- RotationModeC = CollectedCharacterParams.RotationMode

	➍ PrevGait = GaitC
	- GaitC = CollectedCharacterParams.Gait

	➎ PrevStance = StanceC
	- StanceC = CollectedCharacterParams.Stance

	➏ LocomotionModeIndex = CollectedCharacterParams.LocomotionModeIndex

	➐ LOD_State = CollectedCharacterParams.LOD_State

	➑ bPrevCapsuleCollidingC = CapsuleCollidingC
	- CapsuleCollidingC = CollectedCharacterParams.IsColliding
	
	The CollectedCharacterParams variable should hold a list of new values ​​typically retrieved on UpdateAnimationTick 
	(not thread safe) using a reference to the Character class and the interfaces that are attached to it.

	Very important is also updating this values: 	MovementDirection, MovementDirection_LastFrame, 
	MovementDirection_Recent,  MovementDirection_Time, MovementDirection_LastStateTime. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (DisplayName = "Update Main States Values", Keywords = "Motion Matching", BlueprintThreadSafe, AdvancedDisplay = "0"))
	void UpdateMainStatesValues(float RecentTimeLimit = 0.1); virtual void UpdateMainStatesValues_Implementation(float RecentTimeLimit = 0.1);

#pragma endregion


#pragma region FOOTS INVERSE KINEMATIC FUNCTIONS

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Foot IK Core", meta = (DisplayName = "Update Foots IK", Keywords = "Foots IK", AdvancedDisplay = "TraceAboveFoot,TraceBelowFoot,FootHeight,ActiveOnLOD,TraceDebugIndex,DebugTime"))
	void UpdateFootsIK(
		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility,
		bool UseFootsLock = false,
		float TraceAboveFoot = 50.0,
		float TraceBelowFoot = 45.0,
		float FootHeight = 13.5,
		int ActiveOnLOD = 1,
		int TraceDebugIndex = 0,
		float DebugTime = 0.2
	);

	virtual void UpdateFootsIK_Implementation(
		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility,
		bool UseFootsLock = false,
		float TraceAboveFoot = 50.0,
		float TraceBelowFoot = 45.0,
		float FootHeight = 13.5,
		int ActiveOnLOD = 1,
		int TraceDebugIndex = 0,
		float DebugTime = 0.2
	);

	UFUNCTION(BlueprintCallable, Category = "Foot IK Core", meta = (DisplayName = "Set Foot Offsets", Keywords = "Foots IK"))
	bool SetFootOffsets(
		FName Enable_FootIK_Curve,
		FName IKFootBone,
		FName RootBone,
		UPARAM(ref) FVector& CurrentLocationTarget,
		UPARAM(ref) FVector& CurrentLocationOffset,
		UPARAM(ref) FRotator& CurrentRotationOffset,
		ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility,
		float TraceAboveFoot = 50.0,
		float TraceBelowFoot = 45.0,
		float FootHeight = 13.5,
		int TraceDebugIndex = 0,
		float DebugTime = 0.2
	);

	UFUNCTION(BlueprintCallable, Category = "Foot IK Core", meta = (DisplayName = "Set Pelvis IK Offset", Keywords = "Foots IK"))
	void SetPelvisIK_Offset(FVector FootOffset_L_Target, FVector FootOffset_R_Target);

	UFUNCTION(BlueprintCallable, Category = "Foot IK Core", meta = (DisplayName = "Reset IK Offsets", Keywords = "Foots IK"))
	void ResetIK_Offsets(float InterpSpeed = 15.0);


	// FOOTS LOCKING - Manual Configuartion
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Foot IK Core", meta = (ForceAsFunction, Keywords = "Foots IK"))
	void SetFootLocking
	(
		FName FootEnableCurve,
		FName FootLockCurve,
		FName IKFootBone,
		UPARAM(ref) float& CurrentFootLockAlpha,
		UPARAM(ref) FVector& CurrentFootLockLocation,
		UPARAM(ref) FRotator& CurrentFOotLockRotation
	);
	virtual void SetFootLocking_Implementation
	(
		FName FootEnableCurve,
		FName FootLockCurve,
		FName IKFootBone,
		UPARAM(ref) float& CurrentFootLockAlpha,
		UPARAM(ref) FVector& CurrentFootLockLocation,
		UPARAM(ref) FRotator& CurrentFOotLockRotation
	);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Foot IK Core", meta = (ForceAsFunction, Keywords = "Foots IK"))
	void SetFootLockOffset
	(
		float Alpha,
		UPARAM(ref) FVector& LocalLoc,
		UPARAM(ref) FRotator& LocalRot
	);

	virtual void SetFootLockOffset_Implementation
	(
		float Alpha,
		UPARAM(ref) FVector& LocalLot,
		UPARAM(ref) FRotator& LocalRot
	);

#pragma endregion


#pragma region NOT SAFE FUNCTIONS
	//An event that initiates an attempt to load PoseSearchDatabases, which are not yet available.This option is highly EXPERIMENTAL.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (Keywords = "Event,Database,Matching,Pose"))
	void TryAsyncLoadDatabases(); virtual void TryAsyncLoadDatabases_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (Keywords = "Trajectory,Transition,Matching,Pose"))
	void OnStanceTransitionEnded(); virtual void OnStanceTransitionEnded_Implementation();

	/*This function should capture the most important variables from the Character class. This process is typically performed by interface functions:
	- IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_CurrentStates
	- IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_LocomotionModeIndex
	- IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_LOD_State
	- IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_EssentialValues
	- IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_MainTagsContainerData
	- IALS_HumanAI_InterfaceCpp::Execute_HAI_GetControllerSmallValues
	- IAGLS_AI_HumanCharInterface::Execute_BPI_HCAI_Get_StartedCoverMode

	In addition, the default implementation updates values ​​such as:
	- IsCoveringC = CollectedCharacterParams.LocomotionModeIndex == 1;
	- BendDownAlphaC = KML::FInterpTo(BendDownAlphaC, DesiredBendDownAlpha, this->GetDeltaSeconds(), 4.0);
	- OverlayStateElapsedTime
	- MovementParamsControlComponent->GetDesiredMovementsTypeStates(WalkingDatabasesType, RunningDatabasesType, SprintingDatabasesType);
	- StanceTransitionC (TIMER)
	- (RequiredToLoadDatabases.Num() > 0 --> AsyncLoadDatabases*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim Instance Core", meta = (ForceAsFunction, DisplayName = "Collect Values From Character And Update Not Safe Data", Keywords = "Event,Database,Matching,Pose"))
	void CollectAndUpdateDataFromCharacter(); virtual void CollectAndUpdateDataFromCharacter_Implementation();


	UFUNCTION(BlueprintCallable, Category = "Motion Matching Core", meta = (DisplayName = "Make Is Collide Value", Keywords = "Motion Matching", AdvancedDisplay = "DebugIndex,DebugTime,IgnoreCharacters"))
	void MakeIsCollideValue(TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects, int DebugIndex = 0, float DebugTime = 0.1, bool IgnoreCharacters = false);


#pragma endregion


#pragma region TRAJECTORY MOVEMENT ANALYZE

	//Look at the future velocity (determined by trajectory generation) to determine if the character is trying to move (future velocity is greater than 0), or trying to stop (future velocity is 0).
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Is Moving", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool GetIsMovingValue();
	virtual bool GetIsMovingValue_Implementation();

	/*This function is used to determine if the character is starting to move by checking if the future velocity is greater than the current velocity. If the current Database asset is a pivot database,
	this function will always return false. This prevents the Motion Matching system from interrupting a pivot, since the second half of a pivot is very similar to a start.*/
	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Is Starting", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool IsStarting();

	/*This function is used to determine if the character is starting to move by checking if the future velocity is greater than the current velocity. If the current Database asset is a pivot database,
	this function will always return false. This prevents the Motion Matching system from interrupting a pivot, since the second half of a pivot is very similar to a start.*/
	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Is Stopping", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool IsStopping();

	/*This function is used to determine if the character is pivoting by checking if the character’s future trajectory is moving in a much different direction than the character’s current trajectory.
	The Rotation Modes have a different threshold, since 45 degree pivots work nicely during strafing, but are not necessary during Orient to Movement.*/
	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Is Pivoting", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool IsPivoting();

	/*If the root bone rotation and character’s capsule rotations are very different while moving, this function will allow a spin 
	transition animation to play. Spin transitions are locomotion animations that rotate the character while moving in a fixed 
	world direction, and are useful when switching rotation modes. 
	For example, if the character is running toward the camera using the Orient to Movement mode and then switching to strafe, 
	this would require the character to spin 180 degrees very quickly. A spin transition animation would be an ideal transition 
	for this gameplay scenario. Currently, we are using refacing starts in place of spin transitions, but plan to provide actual 
	spin transition data in a future release.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Should Spin Transition", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool ShouldSpinTransition(); virtual bool ShouldSpinTransition_Implementation();

	/*If the character has just landed and the land velocity was less than the heavy land speed threshold, play light land animations.*/
	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Just Laned Light", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool JustLanedLight();

	/*If the character has just landed and the land velocity was greater than the heavy land speed threshold, play heavy land animations.*/
	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Just Laned Heavy", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool JustLanedHeavy();

	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Just Landed Neutral", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool JustLandedNeutral();

	/*This function is used to determine if the character is turning in place by checking if the Future Facing Delta is greater than 
	50 degrees while the character is not moving.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Should Turn In Place", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool ShouldTurnInPlace();
	virtual bool ShouldTurnInPlace_Implementation();

	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze", meta = (DisplayName = "Get Should Use RootMotion DataBase", Keywords = "Motion Matching", BlueprintThreadSafe))
	bool ShouldUseRootMotionData();
	virtual bool ShouldUseRootMotionData_Implementation();


	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (BlueprintThreadSafe, DisplayName = "Get Is Colliding", Keywords = "Motion,Matching,Movement"))
	bool GetIsColliding();


	UFUNCTION(BlueprintPure, Category = "MM Movement Analyze", meta = (BlueprintThreadSafe, DisplayName = "Get Is Walk On Slope", Keywords = "Motion,Matching,Movement"))
	bool GetIsWalkOnSlope();


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze", meta = (BlueprintThreadSafe, DisplayName = "Get Is Pivoting In Circle Shape", Keywords = "Motion,Matching,Movement"))
	bool GetIsPivotingInCircleShape(); virtual bool GetIsPivotingInCircleShape_Implementation();

#pragma endregion


#pragma region TRAJECTORY FUNCTIONS

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze|Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float Get_TrajectoryTurnAngle(); virtual float Get_TrajectoryTurnAngle_Implementation();

	/*
	This function tracks the total amount of rotational delta between the root bone and the future facing rotation. Intermediate rotations are
	used so that we can calculate deltas beyond 180 degrees. This value is used heavily in the animation choosers to select refacing animations,
	and values of beyond 180 prevent us from selecting animations that rotate in the wrong direction. For instance, the trajectory may be
	rotating 200 total degrees to the right during a turn. If we just get the delta between the root and future, we would get -160, which would
	not be reflective of how the pawn is actually rotating and would lead to bad animation selection.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze|Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	float Get_TrajectoryFacingDelta(const TArray<float>& Times, FRotator RootRotation); virtual float Get_TrajectoryFacingDelta_Implementation(const TArray<float>& Times, FRotator RootRotation);


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Movement Analyze|Trajectory", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	void GetIsCirclingTresholds(FVector2D& ReturnPastAngularRange, FVector2D& ReturnCurrentAngularRange, float& ReturnAngle);
	virtual void GetIsCirclingTresholds_Implementation(FVector2D& ReturnPastAngularRange, FVector2D& ReturnCurrentAngularRange, float& ReturnAngle);

#pragma endregion


#pragma region ROOT OFFSET CONTROL FUNCTIONS

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Root Offset", meta = (DisplayName = "Get Offset Root Rotation Mode Index", Keywords = "Motion Matching", BlueprintThreadSafe))
	int GetOffsetRootRotationMode(); virtual int GetOffsetRootRotationMode_Implementation();

	/*This function is used to determine the Offset Root Translation mode. If we are currently playing a montage in the default slot, if we are in the air, or if we are on the ground but not moving,
	we do not want to maintain any Translational offset.
	The Release Enum essentially blends out any offset, after which it will be locked to the capsule location, just as it would be without a root offset node.
	The Interpolate Enum means the root is allowed to deviate slightly from the capsule location based on root motion, but will always try to interpolate back toward center. \
	This is helpful when the animation data and capsule movement are not perfectly matched, such as during starts, pivots, and other complex movements.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Root Offset", meta = (DisplayName = "Get Offset Root Location Mode Index", Keywords = "Motion Matching", BlueprintThreadSafe))
	int GetOffsetRootLocationMode(); virtual int GetOffsetRootLocationMode_Implementation();

	UFUNCTION(BlueprintPure, Category = "MM Root Offset", meta = (DisplayName = "Get Offset Root Translation Half Life", Keywords = "Motion Matching", BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife();

#pragma endregion


#pragma region MOTION MATCHING CONTROLING

	/* This function is used to change the blend time of the Motion Matching node, based on the current and previous states.
	In the future, we plan to allow blend times to be more directly set from the chosen databases.*/
	UFUNCTION(BlueprintPure, Category = "MM Pose Search", meta = (DisplayName = "Get MM Blend Time", Keywords = "Motion Matching", BlueprintThreadSafe))
	float Get_MMBlendTime();


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Pose Search", meta = (DisplayName = "Get MM Interrupt Mode", Keywords = "Motion Matching", BlueprintThreadSafe))
	EPoseSearchInterruptMode Get_MMInterruptMode(); virtual EPoseSearchInterruptMode Get_MMInterruptMode_Implementation();


	/*This Anim Node Function is called whenever the Motion Matching node is updated. It evaluates a Chooser asset which returns an array of Pose Search Database 
	assets based on the current gameplay context. This allows us to perform higher level filtering, giving us more control over what animations the motion matching 
	system is able to select from. For example, we only search from the walk databases when the character is walking (controlled via the player’s input), preventing 
	motion matching from selecting runs when the character is trying to walk.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MM Pose Search", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	void UpdateMotionMatching(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	virtual void UpdateMotionMatching_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);


	/*This function is called after the Motion Matching node has selected an animation. In this case, we cache the database the selected animation is in, in order to grab the tags in the
	EventGraph (due to a thread safety issue).
	In the future, we plan to use this function to control additional things such as blend time and blend profiles based on the selected animation.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MM Pose Search", meta = (DisplayName = "Update Motion Matching Post Selection", Keywords = "Motion Matching", BlueprintThreadSafe))
	void UpdateMotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	virtual void UpdateMotionMatchingPostSelection_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*This function determines if root motion from the animations can be steered by checking if the character is moving or in the air. This prevents idle animations from getting steered, which could cause sliding.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Pose Search", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement"))
	bool EnableSteering(const FAnimNodeReference& Node);
	virtual bool EnableSteering_Implementation(const FAnimNodeReference& Node);


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Pose Search", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement", AdvancedDisplay = 1))
	bool GetEnableSteeringForTurns(const FAnimNodeReference& Node, float RotationAmoutTreshold = 2);
	virtual bool GetEnableSteeringForTurns_Implementation(const FAnimNodeReference& Node, float RotationAmoutTreshold = 2);


	UFUNCTION(BlueprintPure, Category = "MM Pose Search", meta = (DisplayName = "Get Orientation For Warping", Keywords = "Motion Matching", BlueprintThreadSafe))
	FVector GetOrientationForWarping();

	/*This function gives the steering node a target rotation. This target is calculated using the future facing direction
	from the predicted trajectory. This allows the steering node to rotate toward a future direction, rather than always
	steering toward the current actor rotation, which could cause it to lag too far behind.*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Pose Search|Steering", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement", AdvancedDisplay = 1))
	FQuat GetDesiredFacing(const FAnimNodeReference& Node, FVector2D FacingSampleTimeRange = FVector2D(0.8, 1.5), FName SteringTimeCurve = TEXT("SteeringTargetTime"));
	virtual FQuat GetDesiredFacing_Implementation(const FAnimNodeReference& Node, FVector2D FacingSampleTimeRange = FVector2D(0.8, 1.5), FName SteringTimeCurve = TEXT("SteeringTargetTime"));


	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MM Pose Search|Steering", meta = (BlueprintThreadSafe, Keywords = "Motion,Matching,Movement", AdvancedDisplay = 1))
	float GetProceduralTargetTime(const FAnimNodeReference& Node, FVector2D DefaultTargetTimeRage = FVector2D(0.08, 0.16), float MaxTargetTime = 0.3, float ForSpinsTargetTime = 0.3);
	virtual float GetProceduralTargetTime_Implementation(const FAnimNodeReference& Node, FVector2D DefaultTargetTimeRage = FVector2D(0.08, 0.16), float MaxTargetTime = 0.3, float ForSpinsTargetTime = 0.3);


	UFUNCTION(BlueprintPure, Category = "MM Pose Search", meta = (DisplayName = "Get Speed Difference Delta", Keywords = "Motion Matching", BlueprintThreadSafe))
	float Get_SpeedDifferenceDelta();


#pragma endregion


private:
	float VectorLenghtXY(FVector In);

	template<typename T>
	FORCEINLINE T& IgnoreOut()
	{
		// Oddzielny bufor na wątek; bezpieczny dla Game Thread.
		static thread_local T Dummy{};   // wymaga domyślnego konstruktora / trivialnego T
		return Dummy;
	}


	template<typename TEnum>
	FORCEINLINE static void UpdateStateValuesMacro(
		const TEnum NewState,
		TEnum& CurrentState,
		TEnum& LastFrameState,
		TEnum& RecentState,
		float& TimeInState,
		float& LastStateTime,
		const float RecentTimeLimit,
		const float DeltaSeconds)
	{
		LastFrameState = CurrentState;
		CurrentState = NewState;

		if (CurrentState != LastFrameState)
		{
			LastStateTime = TimeInState;
			TimeInState = 0.0f;
		}
		else
		{
			TimeInState += DeltaSeconds;
		}

		if (TimeInState >= RecentTimeLimit)
		{
			RecentState = CurrentState;
		}
	}


};
