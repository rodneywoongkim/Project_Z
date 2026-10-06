

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Engine/DataTable.h"
#include "HandleForItemCpp.h"
#include "ALS_StructuresAndEnumsCpp.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class AGLS_HumanAI_OtherActionState : uint8
{
	None,
	StealthFinisher,
	IsHostage,
	Surrendering,
	TryPickUpGun
};


UENUM(BlueprintType)
enum class AGLS_HumanAI_TraversalActionState : uint8
{
	None,
	GoingUp,
	GoingDown,
	ClimbUpDown
};


UENUM(BlueprintType)
enum class AGLS_HumanAI_SearchingState : uint8
{
	SawSomeone,
	SawCorpse,
	HearedSoundShot,
	HearedSoundSteps,
	LostSight,
	NotValid
};



UENUM(BlueprintType)
enum class AGLS_WalkingType : uint8
{
	Default,
	RelaxedWalk,
	StealthWalk,
	InjuredWalk,
	SlowWalk,
	Other
};

UENUM(BlueprintType)
enum class AGLS_RunningType : uint8
{
	Default,
	Jog,
	RelaxedRun,
	StealthRun,
	InjuredRun,
	SlowJog,
	Other
};

UENUM(BlueprintType)
enum class AGLS_SprintingType : uint8
{
	Default,
	NormalRun,
	RelaxedSprint,
	FastSprint,
	InjuredSprint,
	SlowSprint,
	Other
};


UENUM(BlueprintType)
enum class AGLS_CoveringDirection : uint8
{
	Left,
	Right,
	AimingLeft,
	AimingRight,
	AimingForward,
	UnCovering
};


UENUM(BlueprintType)
enum class AGLS_HumanAI_MainBehaviorMode : uint8
{
	Patroling,
	Finding,
	Fighting,
	Running,
	Interacting,
	None
};

UENUM(BlueprintType)
enum class AGLS_HumanAI_PatrolingMode : uint8
{
	FollowingPath,
	RandomMove,
	NoMoveAndStand,
	NoMoveAndSit,
	Interacting,
	None
};

UENUM(BlueprintType)
enum class AGLS_HumanAI_FightingMode : uint8
{
	HideBehindCover,
	NoCovering,
	RunningFromEnemy,
	SearchingForEnemy,
	WithoutWeapons,
	FullAgressive,
	None
};

UENUM(BlueprintType)
enum class AGLS_HumanAI_SightStatus : uint8
{
	SeesNothing,
	SawSomething,
	ActiveSeeEnemy,
	LostSight,
	NotAnymoreSee
};




UENUM(BlueprintType)
enum class AGLS_LOD_State : uint8
{
	LOD0,
	LOD1,
	LOD2,
	LOD3
};

UENUM(BlueprintType)
enum class AGLS_MovementDirectionState : uint8
{
	F,
	B,
	LL,
	LR,
	RL,
	RR
};


UENUM(BlueprintType)
enum class CALS_Gait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class CALS_MovementState : uint8
{
	None UMETA(DisplayName = "None"),
	Grounded UMETA(DisplayName = "Grounded"),
	InAir UMETA(DisplayName = "In Air"),
	Mantling UMETA(DisplayName = "Mantling"),
	Ragdoll UMETA(DisplayName = "Ragdoll"),
	Crawl UMETA(DisplayName = "Crawl"),
	Prone UMETA(DisplayName = "Prone")
};

UENUM(BlueprintType)
enum class CALS_OverlayState : uint8
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	Pistol1H,
	Pistol2H,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel,
	Rope,
	Axe,
	Knife
};

UENUM(BlueprintType)
enum class CALS_RotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

UENUM(BlueprintType)
enum class CALS_GroundedMoveMode : uint8
{
	Normal,
	SlowWalk,
	Injured,
	Tired,
	Stealth
};

UENUM(BlueprintType)
enum class CALS_Stance : uint8
{
	Standing,
	Crouching
};

UENUM(BlueprintType)
enum class CALS_MovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

UENUM(BlueprintType)
enum class CALS_OverlayPosesType : uint8
{
	Relaxed,
	Ready,
	Aiming
};


UENUM(BlueprintType)
enum class CMC_ActionTypeC : uint8
{
	None,
	ShortMove,
	CornerOuter,
	CornerInner,
	Turn180,
	JumpNextLedge,
	JumpBackToNextLedge,
	ForwardMove,
	PullUpToNarrowFloor,
	DropToNarrowFloor,
	DropFromNarrowFloor,
	JumpToBeamSwinging,
	JumpForwardToBeam,
	StartHoldingLedge
};

UENUM(BlueprintType)
enum class CALS_DeathType : uint8
{
	KilledByGun,
	FallFromHeight,
	HitByCar,
	Explosion,
	SilenthDeath,
	BiteByZombie
};


USTRUCT(BlueprintType)
struct FAGLS_HumanAI_EnemyTags : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	bool DetectedEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	AGLS_HumanAI_SightStatus SightStatus = AGLS_HumanAI_SightStatus::SeesNothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	ACharacter* EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	float ReactionProgressTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	FVector LostSightLocation = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	bool IsZombie = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	bool EnemySpottedHim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	bool ShouldHideSelfFromEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	float PiorityBias = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight Perception")
	float ChanceForFinisher = 0.0;

};


USTRUCT(BlueprintType)
struct FCALS_ComponentAndTransform : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
		FTransform Transform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
		UPrimitiveComponent* Component = nullptr;
};

USTRUCT(BlueprintType)
struct FCMC_SingleClimbPointC : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
	bool ValidPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FVector Location = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FVector Normal = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FTransform ActorTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		UPrimitiveComponent* Component = nullptr;
};

USTRUCT(BlueprintType)
struct FCMC_LedgeC : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FTransform LeftPoint = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FTransform RightPoint = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FTransform Origin = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		UPrimitiveComponent* Component = nullptr;
};


//Traversal state evaluation
USTRUCT(BlueprintType)
struct FTraversalStateEvaluation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollisionState")
	bool HasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollisionState")
	bool HasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollisionState")
	bool HasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranges")
	FVector2D ObstacleHeightRange = FVector2D(0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranges")
	FVector2D ObstacleDepthRange = FVector2D(0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranges")
	FVector2D BackLedgeHeightRange = FVector2D(0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConditionIgnore")
	TArray<bool> ConditionToIgnore = {};
};


USTRUCT(BlueprintType)
struct FTraversalSingeAnimAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	FVector StartingOffset = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float LowHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float HighHeight = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float MinAnimStartAt = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float MaxAnimStartAt = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float MinPlayRate = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mantle Asset")
	float MaxPlayRate = 1.1f;
};

USTRUCT(BlueprintType)
struct FCALSMovementSettingsStrafe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	FVector WalkSpeed = FVector(200,200,200);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	FVector RunSpeed = FVector(400, 400, 400);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	FVector SprintSpeed = FVector(500, 500, 500);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	UCurveVector* MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	UCurveFloat* RotationRateCurve = nullptr;
};



USTRUCT(BlueprintType)
struct FCALSMovementSettingsStrafeExtend
{
	GENERATED_BODY()

	//X = Forward Direction, Y = Left/Right Direction, Z = Backward Direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Speeds Control")
	FVector WalkSpeed = FVector(200, 200, 200);

	//X = Forward Direction, Y = Left/Right Direction, Z = Backward Direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Speeds Control")
	FVector RunSpeed = FVector(400, 400, 400);

	//X = Forward Direction, Y = Left/Right Direction, Z = Backward Direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Speeds Control")
	FVector SprintSpeed = FVector(500, 500, 500);

	/*
	Curve Index: X = Acceleration, Y = Deceleration, Z = GroundFriction
	Curve Time value: 0 = Idle, 1 = Walk, 2 = Run/Jog, 3 = Sprint/Run, 4 = Sprint/Run (Only When Required)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|CMC Params")
	UCurveVector* MovementCurve = nullptr;

	/*
	0.0 = Idle, 1.0/2.0 = Walk, 3.0/4.0 = Run/Jog, 5.0/6.0 = Sprint/Run
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|CMC Params")
	UCurveFloat* RotationRateCurve = nullptr;

	/*
	An important parameter in the context of Pose Search / Motion Matching. Here, you should specify a value that defines the correct 
	set of PoseSearchDatabase assets matching the parameters intended for the CharacterMovementComponent. By default, this does not 
	mean that a valid animation set is available for every enum value. However, this type of parameter can be useful when you have 
	several different walking/running styles available.

	Wazny parametr w kontekscie Pose Search / Motion Matching. Tutaj nalezalo by wskazac wartosc okreslajaca prawidlowy zestaw baz 
	PoseSearchDatabase pasujacy do parametrow przeznaczonych dla CharacterMovementComponent. Domyslnie nie oznacza to ze dla kazdej 
	wartosci enum dostepny jest prawidlowy zestaw animacji. Parametr tego typu moze byc jednak pomocy w przypadku kiedy mamy do
	dyspozycji kilka typow chodzenia/biegania.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Motion Matching")
	AGLS_WalkingType MatchingWalkDatabasesToSpeed = AGLS_WalkingType::Default;

	/*
	An important parameter in the context of Pose Search / Motion Matching. Here, you should specify a value that defines the correct
	set of PoseSearchDatabase assets matching the parameters intended for the CharacterMovementComponent. By default, this does not
	mean that a valid animation set is available for every enum value. However, this type of parameter can be useful when you have
	several different walking/running styles available.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Motion Matching")
	AGLS_RunningType MatchingRunDatabasesToSpeed = AGLS_RunningType::Default;

	/*
	An important parameter in the context of Pose Search / Motion Matching. Here, you should specify a value that defines the correct
	set of PoseSearchDatabase assets matching the parameters intended for the CharacterMovementComponent. By default, this does not
	mean that a valid animation set is available for every enum value. However, this type of parameter can be useful when you have
	several different walking/running styles available.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings|Motion Matching")
	AGLS_SprintingType MatchingSprintDatabasesToSpeed = AGLS_SprintingType::Default;


};



USTRUCT(BlueprintType)
struct FCALS_PropsAttachValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Prop")
	UPrimitiveComponent* TargetComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Values")
	USceneComponent* ParentComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Values")
	FName AttachSocketName = TEXT("none");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Values")
	FTransform AttachOffset = FTransform::Identity;

};

USTRUCT(BlueprintType)
struct FAGLS_FinishersDataForAI
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	float SequenceDuration = 2.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	TSoftObjectPtr<UAnimMontage> MontageAtt = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	TSoftObjectPtr<UAnimSequence> AnimSeqenceAtt = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	TSoftObjectPtr<UAnimMontage> MontageVic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	TSoftObjectPtr<UAnimSequence> AnimSeqenceVic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	FVector ConstCapsuleOffsets = FVector(-80.0, -5.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	FRotator CapsuleRotationOffset = FRotator(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	FName AnimSlotForAtt = TEXT("BaseLayer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	FName AnimSlotForVic = TEXT("BaseLayer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	FName WarpPointName = TEXT("FinisherAction");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Finisher Action")
	float WeightScale = 1.0;
};



//Structure declarated for AGLS v2.0. Mainly prerpared for props/weapons like bow, axe, knife, binoculars
USTRUCT(BlueprintType)
struct FAGLS_OtherPropDefinitionData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	FName PropName = TEXT("none");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	TEnumAsByte<EIWALS_HandleItemType::Type> PropCategory = EIWALS_HandleItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	bool UseAsSkeletalMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (EditCondition = "!UseAsSkeletalMesh"))
	UStaticMesh* PropStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (EditCondition = "UseAsSkeletalMesh"))
	USkeletalMesh* PropSkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (EditCondition = "UseAsSkeletalMesh"))
	UPhysicsAsset* PropSkeletalPhysics = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (EditCondition = "UseAsSkeletalMesh"))
	UStaticMesh* PropSkeletalCollisionMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (EditCondition = "UseAsSkeletalMesh"))
	TSubclassOf<UAnimInstance> PropAnimInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool bInsatanceCanRenderCustomDepth = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	bool bItsForLeftHandProp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	FTransform AttachTransformWhenNotUsable = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	FTransform AttachTransformWhenUsable = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	TMap<FName, bool> RuntimeBoolProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	TMap<FName, float> RuntimeFloatProperties;

};



class HELPFULFUNCTIONS_API ALS_StructuresAndEnumsCpp
{
public:
	ALS_StructuresAndEnumsCpp();
	~ALS_StructuresAndEnumsCpp();
};
