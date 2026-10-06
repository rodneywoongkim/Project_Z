// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "JakubCablePhysic.h"
#include "DrawDebugHelpers.h"
#include "ALS_HookActorInterface.h"
#include "ModifyClimbingParamsVolume.h"
#include "AGLS_AdvancedLedgesFinder.h"
#include "Cpp_DynamicClimbingComponent.generated.h"

/*The most important setting for the DynamicLedgeClimbing configuration. Depending on your needs, you can choose one of
the available ledge/edge detection solvers.

1) Default – The default ledge detection algorithm. Its implementation originates from the IWALS project. Pros and cons:
- Features robust verification of the detected ledge
- Generally performs fewer traces compared to other algorithms
- Narrow search range for contact points
- Limited ability to detect perpendicular edges
- Struggles with significant surface irregularities
- Limited set of solver configuration parameters

2) ComplexTracesMode – Named for its frequent use of traces with the 'Complex' option enabled. Pros and cons:
- Less rigorous ledge verification compared to 'Default' (can be an advantage or disadvantage depending on the
  environment geometry)
- Can perform a large number of traces during deep searches, though the iteration count can be limited
- Wider contact point detection range compared to 'Default'
- Performs better with edges perpendicular to the FindingDirection vector
- Struggles more with maintaining the continuity of the generated ledge
- Extensive set of configuration parameters

3) Astra An algorithm designed and written almost entirely in C++ by the ChatGPT-6 model (Astra).
The code is highly complex and difficult for the user to read.
- Very broad ledge detection range
- Most computationally expensive algorithm of the three
- Largest number of configuration parameters, significantly impacting solver optimization and accuracy*/
UENUM(BlueprintType)
enum class AGLS_DynamicLedgeSolverType : uint8
{
	Default,
	ComplexTracesMode,
	Astra
};


USTRUCT(BlueprintType)
struct FAGLS_LedgeFinderConfig_ComplexTracigMethod : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float MaxLedgeLength = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float MinLedgeLength = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	FVector SearchingPointsOriginOffset = FVector::ZeroVector;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	double SearcherForwardWallLength = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	double SearcherForwardWallRadius = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	int32 SearcherUpTracesChecksNumber = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	int32 SearcherMaxUpTracesCandidates = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	int SearcherPhaseCheckTracesCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	float SearcherPhaseTollerance = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finding Ledge Point")
	FVector2D SearcherSurfaceLineTraceLength = FVector2D(20, 20);



	//Single Ledge Point finder function max calls per direction (left or right ledge point)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creating Ledge")
	int SingleLedgePointFinderMaxCalls = 3;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creating Ledge")
	//float OffsetBetweenLedgePointsFinder = 3.0;


	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check")
	float PlayerCapsuleRadiusBias = 0;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check")
	float PlayerCapsuleHalfHeightBias = 0;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (ClampMin = "-60", ClampMax = "60"))
	float CapsuleFreeSpaceCheckOriginOffsetUp = 0;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (ClampMin = "1", ClampMax = "16"))
	int32 CapsuleFreeSpaceCheckNumber = 4;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check")
	bool bUseAdvancedCapsuleFreeSpaceFinder = true;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "!bUseAdvancedCapsuleFreeSpaceFinder"))
	double ReduceCapsuleRadiusScaleBias = 0.1;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "!bUseAdvancedCapsuleFreeSpaceFinder"))
	double ReduceCapsuleHeightBias = 8.0;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "-1", ClampMax = "50"))
	float CapsuleFreeSpaceMaxTopReduction = 10;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "-1", ClampMax = "50"))
	float CapsuleFreeSpaceMaxBottomReduction = 10;


	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder"))
	float MinValidCapsuleOutHalfHeight = 50.0f;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "5", ClampMax = "80"))
	float MinValidCapsuleOutRadius = 20.0f;

	//For Veryfing Finded Ledge is valid for Climbing
	//X = Bottom, Y = Top
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder"))
	FVector2D MaxValidCapsuleReducedSize = FVector2D(50, 15);

	//X for MinLedgeLength, Y for MaxLedgeLength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Verify")
	FVector2D MaxHeightDifferencyBetweenPoints = FVector2D(5, 25);



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Efficiency And Accuracy", meta = (ClampMin = "1", ClampMax = "200"))
	int MaxLedgeFindingTracesQueries = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Efficiency And Accuracy", meta = (ClampMin = "4", ClampMax = "64"))
	int MaxSingleCapsuleFreeSpaceTracesQueries = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Efficiency And Accuracy", meta = (ClampMin = "2", ClampMax = "32"))
	int MaxCapsuleTotalRoomQueriesPerLedgeGen = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Efficiency And Accuracy", meta = (ClampMin = "10", ClampMax = "500"))
	int MaxTotalTracesQueries = 100;

};


USTRUCT(BlueprintType)
struct FAGLS_LedgeFinderConfig_DefaultSolver : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float MaxLedgeLength = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float MinLedgeLength = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float ForwardTraceLength = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float RightOffsetScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	bool UseWallCondition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	bool InverseTracing = true;


	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check")
	bool bUseAdvancedCapsuleFreeSpaceFinder = false;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "-1", ClampMax = "50", EditConditionHides))
	float CapsuleFreeSpaceMaxTopReduction = 10;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Check", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "-1", ClampMax = "50", EditConditionHides))
	float CapsuleFreeSpaceMaxBottomReduction = 10;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", EditConditionHides))
	float MinValidCapsuleOutHalfHeight = 50.0f;

	//For Veryfing Finded Ledge is valid for Climbing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", ClampMin = "5", ClampMax = "80", EditConditionHides))
	float MinValidCapsuleOutRadius = 20.0f;

	//For Veryfing Finded Ledge is valid for Climbing
	//X = Bottom, Y = Top
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Free Space Result", meta = (EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", EditConditionHides))
	FVector2D MaxValidCapsuleReducedSize = FVector2D(50, 15);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Efficiency And Accuracy", meta = (ClampMin = "4", ClampMax = "64", EditCondition = "bUseAdvancedCapsuleFreeSpaceFinder", EditConditionHides))
	int MaxSingleCapsuleFreeSpaceTracesQueries = 12;

};



/*A complex component designed to support the implementation of mechanics such as climbing, ziplining, pickaxe climbing, and 
rope swinging. Key functions within this class include:
1) TryCreateLedgeStructure() – A comprehensive function designed to locate a climbable ledge in the surrounding environment.
2) Rope swinging-related elements, including RopeSwingUpdatePhysic() and RopeLengthUpdate().*/
UCLASS(Blueprintable, ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_DynamicClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_DynamicClimbingComponent();

protected:
	virtual void BeginPlay() override;
	// Create Only Cpp Variables
	UCpp_DynamicClimbingComponent* SelfComp;

	// Create Essential Variables


#pragma region NEW VARIABLES ADDED FOR AGLS v2.0

/*The most important setting for the DynamicLedgeClimbing configuration. Depending on your needs, you can choose one of 
the available ledge/edge detection solvers.

1) Default – The default ledge detection algorithm. Its implementation originates from the IWALS project. Pros and cons:
- Features robust verification of the detected ledge
- Generally performs fewer traces compared to other algorithms
- Narrow search range for contact points
- Limited ability to detect perpendicular edges
- Struggles with significant surface irregularities
- Limited set of solver configuration parameters
- Works correctly with BSP objects (Geometry Brushes).

2) ComplexTracesMode – Named for its frequent use of traces with the 'Complex' option enabled. Pros and cons:
- Less rigorous ledge verification compared to 'Default' (can be an advantage or disadvantage depending on the 
  environment geometry)
- Can perform a large number of traces during deep searches, though the iteration count can be limited
- Wider contact point detection range compared to 'Default'
- Performs better with edges perpendicular to the FindingDirection vector
- Struggles more with maintaining the continuity of the generated ledge
- Extensive set of configuration parameters
- Does not respond to BSP (Geometry Brush) ❌

3) Astra (⚠️𝙑𝙀𝙍𝙔 𝙀𝙓𝙋𝙀𝙍𝙄𝙈𝙀𝙉𝙏𝘼𝙇) – An algorithm designed and written almost entirely in C++ by the ChatGPT-6 model (Astra). 
The code is highly complex and difficult for the user to read.❗ASTRA Solver currently is not implemented in this component
- Very broad ledge detection range
- Most computationally expensive algorithm of the three
- Largest number of configuration parameters, significantly impacting solver optimization and accuracy*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver"))
	AGLS_DynamicLedgeSolverType LedgeSolverAlgoritmType = AGLS_DynamicLedgeSolverType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver", EditCondition = "LedgeSolverAlgoritmType == AGLS_DynamicLedgeSolverType::Default", EditConditionHides))
	FAGLS_LedgeFinderConfig_DefaultSolver DefaultLedgeSolverConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver", EditCondition = "LedgeSolverAlgoritmType == AGLS_DynamicLedgeSolverType::ComplexTracesMode", EditConditionHides))
	FAGLS_LedgeFinderConfig_ComplexTracigMethod ComplexTracesLedgeSolverConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver", EditCondition = "LedgeSolverAlgoritmType == AGLS_DynamicLedgeSolverType::Astra", EditConditionHides))
	FAGLSLedgeSearchSettings AstraLedgeSolverConfig;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver"))
	int SingleLedgeFindTotalTracesResult = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Ledge Solver"))
	TSubclassOf<UObject> GeometryBrushCompoentClass = nullptr;


#pragma endregion

#pragma region Variables DECLARATION

public:
	//Modify VOLUME ACTOR
	AModifyClimbingParamsVolume* CurrentModifyVolume = nullptr;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Base"))
	float dt = 0.01f;

	//Base Bools Values
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Base", DisplayName = "IsClimbing"))
	bool IsClimbingC = false;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Base", DisplayName = "FreeHang"))
	bool FreeHangC = false;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Base", DisplayName = "StartNarrowFloorMovement"))
	bool StartNarrowFloorMovementC = false;

	//Main Action Enum
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Base", DisplayName = "Action"))
	CMC_ActionTypeC ActionC = CMC_ActionTypeC::None;



	//Transformations
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms", DisplayName = "LedgePointsLS"))
	FCMC_LedgeC LedgePointsLS_C;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms", DisplayName = "CachedLedgePointsLS"))
	FCMC_LedgeC CachedLedgePointsLS_C;



	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms", DisplayName = "TargetIK LedgeLS"))
	FCMC_LedgeC TargetIK_LedgeLS;

	//This Variable is declarated(new) as C++ for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms", DisplayName = "TargetIK LedgeWS"))
	FCMC_LedgeC TargetIK_LedgeWS;

	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms"))
	FCALS_ComponentAndTransform CapsuleTargetTransformLS;

	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms"))
	FCALS_ComponentAndTransform CapsuleTargetTransformWS;

	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms"))
	FCALS_ComponentAndTransform SavedCapsuleTransformLS;

	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms"))
	FTransform SavedCapsuleTransformWS;

	//This Variable is converted as C++ declaration for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Transforms"))
	FTransform CapsuleTargetPositionWS;



	//Init Values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "DefCapsuleSize"))
	FVector2D DefCapsuleSizeC = FVector2D(0.0,0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "NarrowFloorCapRadius"))
	float NarrowFloorCapRadiusC = 15.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "CapsuleUpOffset"))
	float CapsuleUpOffsetC = 65.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Pickaxe Climbing|Config", DisplayName = "ConstCapsuleOffsetBetWall"))
	float ConstCapsuleOffsetBetWallC = 5.0; //Picaxe Climbing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Pickaxe Climbing|Config", DisplayName = "PickaxeClimbChannel"))
	TEnumAsByte<ECollisionChannel> PickaxeClimbChannelC = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "Character"))
	ACharacter* CharacterC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "ForClimbingChannel"))
	TEnumAsByte<ECollisionChannel> ForClimbingChannelC = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config"))
	TSubclassOf<AActor> BeamForSwingingIdentifyClass = AActor::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "ClassToIgnoreByLedge"))
	TArray<UClass*> ClassToIgnoreByLedgeC = TArray<UClass*>();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config"))
	TArray<AActor*> ActorsInstancesIgnoredByLedgeTraces;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Debug", DisplayName = "DebugTraceIndex", ClampMin = "0", ClampMax = "2"))
	int DebugTraceIndexC = 0;

	//This Variable is declarated(new) as C++ for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Debug", DisplayName = "DebugFootsTraceIndex", ClampMin = "0", ClampMax = "2"))
	int DebugFootsTraceIndexC = 0;

	//This Variable is declarated(new) as C++ for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Debug"))
	bool bPrintStringForComplexLedgeVerify = false;

	//This Variable is declarated(new) as C++ for ★𝐀𝐆𝐋𝐒 𝐯2.0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "New Climbing System|Config|Debug", DisplayName = "DebugCapsuleSpaceVerfy", ClampMin = "0", ClampMax = "2"))
	int DebugCapsuleSpaceVerfy = 0;


	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Config", DisplayName = "FootsDefOffset"))
	FTwoVectors FootsDefOffsetsC = {};

	//Other
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Inputs", DisplayName = "AxisValuesInterp"))
	FVector2D AxisValuesInterpC = FVector2D(0.0, 0.0);

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Inputs", DisplayName = "AxisValuesInterpSlow"))
	FVector2D AxisValuesInterpSlowC = FVector2D(0.0, 0.0);

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Inputs", DisplayName = "AxisValues"))
	FVector2D AxisValuesC = FVector2D(0.0, 0.0);

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Time", DisplayName = "FootsRelativeVelocity"))
	FVector FootsRelativeVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Animations", DisplayName = "JumpBackPoseAlpha"))
	FVector2D JumpBackPoseAlphaC = FVector2D(0.0, 0.0);

	//Inputs Bools
	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Inputs", DisplayName = "ShiftPressed"))
	bool ShiftPressedC = false;

	UPROPERTY(BlueprintReadWrite, meta = (Category = "New Climbing System|Inputs", DisplayName = "SpaceBarImpulse"))
	bool SpaceBarImpulseC = false;

#pragma endregion

#pragma region Variables DECLARATION - Rope Swinging

	// [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-]
	// ROPE SWING SYSTEM VARIABLES:

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System|Config", DisplayName = "ConstSwingLenghtOffsetC"))
	float ConstSwingLenghtOffsetC = 100.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System|Config", DisplayName = "SwingDebugIndexC"))
	int SwingDebugIndexC = 0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "CableSimComponentC"))
	UJakubCablePhysic* CableSimC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "HookActorC"))
	AActor* HookActorC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "IsSwingingC"))
		bool bIsSwingingC = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "bIsFallingStartedC"))
		bool bIsFallingStartedC = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "AnchorPointInterpC"))
		FVector AnchorPointInterpC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "SwingRadiusSmoothC"))
		float SwingRadiusSmoothC = 500.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "TargetForceC"))
		FVector TargetForceC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "GravityStabilityForceC"))
		FVector GravityStabilityForceC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "CollisionIndexC"))
		int CollisionIndexC = -1;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "SwingAnimPropertyC"))
		FVector2D SwingAnimPropertyC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "TargetRopeLenghtC"))
		float TargetRopeLenghtC = 500.0;

#pragma endregion

	// ______________________________________________________________________________________________________________________________________________________________________________________________

#pragma region Functions - Mainly For Ledge Clibing

	// FUNCTIONS (NOT OVERRIDE)

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Create Axis Values With Interp", Keywords = "Axis"))
	virtual void CreateAxisValuesWithInterpFast(float InterpSpeed=5.0, float Delta = 0.01);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge To Cap Position", Keywords = "Transformation CPP"))
	virtual FCALS_ComponentAndTransform ConvertLedgeToCapPositionC(FCALS_ComponentAndTransform Center);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Floor To Cap Position", Keywords = "Transformation CPP"))
	virtual FCALS_ComponentAndTransform ConvertFloorToCapPositionC(FCALS_ComponentAndTransform Center);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Choose Ledge Finding Transform", Keywords = "Transformation CPP"))
	virtual void ChooseLedgeFindingTransformC(bool GetByLedge, FVector& ReturnLocation, FVector& ReturnDirection);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Direction By Axis Input", Keywords = "Vector"))
	virtual FVector GetDirectionByInputC(float LerpWithForward=0.2);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge Vector To Transform WS", Keywords = "Transformation CPP"))
	virtual FCALS_ComponentAndTransform ConvertLedgeStructToWS(FCMC_SingleClimbPointC SingleClimbPointWS = FCMC_SingleClimbPointC());

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge Vector To Transform LS", Keywords = "Transformation CPP"))
	virtual FCALS_ComponentAndTransform ConvertLedgeStructToLS(FCMC_SingleClimbPointC SingleClimbPointWS = FCMC_SingleClimbPointC());

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Ledge Finding", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Ledge Is Valid Part 2", Keywords = "Transformation CPP"))
	virtual bool LedgeValidationPart2C(bool Valid = false, FCMC_SingleClimbPointC LeftStruct = FCMC_SingleClimbPointC(), FCMC_SingleClimbPointC RightStruct = FCMC_SingleClimbPointC(), 
	float MinDistanceBetweenPoints = 14.0, float RotationTollerance = 0.4, float CapsuleUpOffset = -50.0, FVector2D CapsuleChecking = FVector2D(90.0, 30.0));

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Start Or End", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Can Drop To Ledge", Keywords = "Corner CPP"))
	virtual bool CheckCanDropToLedgeC(FCMC_LedgeC& LedgeStructWS);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Resize Radius To Default", Keywords = "CPP"))
	virtual void ResizeCapsuleToDefaultC(float InterpSpeed=100);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Do While Climbing", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Ledge Per Frame", Keywords = "CPP"))
	virtual void UpdateLedgePerFrameC(FCMC_LedgeC& OutLedge, FVector& OutOrigin, FVector2D SlopeScale = FVector2D(1,0.4),float ConstMovementOffset = 2, bool InputLock = false);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|IK", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Foot IK Valid", Keywords = "CPP"))
	virtual bool CheckFootIkValidC(FTransform Transform, bool ForRightFoot, float TraceUpOffset);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Foots Relative Velocity", Keywords = "CPP"))
	virtual FVector GetFootsRelativeVelocityC();

#pragma endregion

#pragma region Functions - Pickaxe Climbing

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Axis To Name", Keywords = "CPP"))
	virtual FName ConvertAxisToNameC();

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Player Can Move To Wall", Keywords = "CPP"))
	virtual bool CheckPlayerCanMoveToWallC(bool Check, FCALS_ComponentAndTransform TransformWS, FCALS_ComponentAndTransform& ReturnTransformWS);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Wall To Cap Position", Keywords = "CPP", CompactNodeTitle = "WallToCap"))
	virtual FCALS_ComponentAndTransform ConvertWallToCapPositionC(FCALS_ComponentAndTransform TransformWS);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Try Find Tangent For Wall", Keywords = "CPP"))
	virtual void TryFindTangentForWallC(bool& ReturnValid, FCALS_ComponentAndTransform& TransformWS, FVector FindingLocation, FVector FindingDirection, float FindingLength = 80, 
	float FirstRadius = 20, float DistanceOffsetScale = -0.5, int VerticalAccuracy = 2, FVector2D CapsuleSize = FVector2D(30, 90));

#pragma endregion

#pragma region Functions - Rope Swinging

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Rope Swing Update Physic ", Keywords = "CPP,Rope,Swing,Cable", AdvancedDisplay = 5))
	virtual void RopeSwingUpdatePhysicC
	(
		UCurveVector* RegulationCurve = nullptr,
		float SwingMinRange = 100.0,
		float SwingMaxRange = 800.0,
		int HandAttachIndex = 5,
		bool AddtiveCondition = true,
		float HysteresisMin = 100, 
		float GravityStabilityFactor = 0.1, 
		float ForceScaleFactor = 1.0, 
		float InterpSpeedIn = 18.0, 
		float SphereOriginInterpSpeed = 20.0,
		float RadiusInterpSpeed = 10.0,
		FVector2D SwingLenghtFactor = FVector2D(1.4, 1.3)
	);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Try Find Hook Point", Keywords = "CPP,Rope,Swing,Cable", AdvancedDisplay = 5))
	virtual bool TryFindHookPointC
	(
		AActor*& HookActor,
		TEnumAsByte<EObjectTypeQuery> TraceObject,
		float FindingRadius = 200.0,
		float CapsuleHeightScale = 5.0,
		FVector Direction = FVector(1, 0, 0),
		float DistancePioryty = 0.5,
		int DrawDebug = 0
	);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Rope Lenght Update", Keywords = "CPP,Rope,Swing,Cable", AdvancedDisplay = 4))
	virtual void RopeLenghtUpdateC
	(
		UCurveVector* ForceCurve = nullptr,
		float RopeMinLeght = 50.0,
		float RopeMaxLenght = 800.0,
		int HandAttachIndex = 5,
		FName TimerName = TEXT("RopeSwingTimer2"),
		float TimerMaxTime = 4.0,
		float UpForceStrenght = -800.0,
		int ExpandDivite = 4,
		FVector2D InterpSpeedRange = FVector2D(6, 12)
	);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Reducing Velocity When Swing", Keywords = "CPP,Rope,Swing,Cable"))
	virtual void ReducingVelocityWhenSwingC
	(
		float ReductionDampingFactor = 0.25,
		float VelocityTrigger = 500.0,
		float SwingingDampingFactor = 0.6,
		int ActionIndex = 0
	);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Update Air Control", Keywords = "CPP,Rope,Swing,Cable", AdvancedDisplay = 4))
	virtual void UpdateAirControlC
	(
		int ActionIndex = 0,
		float RopeMinLenght = 50.0,
		float SwingMinRange = 100.0,
		float SwingMaxRange = 800.0,
		FVector2D AirControlRange = FVector2D(0.07, 0.13),
		float SwingMinBias = 100.0,
		float InterpToSpeed = 8.0,
		float ReduceToZeroSpeed = 10.0
	);

#pragma endregion

#pragma region Functions - Mainly For Ledge Climb With Override in BP

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Functions Library (Can Be Override!!!)

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "New Climbing System|Other", meta = (DisplayName = "Get Character Axis", Keywords = "Axis Character"))
	void GetCharacterAxisC(float& Forward, float& Right);
	virtual void GetCharacterAxisC_Implementation(float& Forward, float& Right);




	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	/*THIS IS CLIMBING SYSTEM MOST IMPORTANT FUNCTION*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder", meta = (DisplayName = "Try Create Ledge Structure", Keywords = "CPP Ledge", AdvancedDisplay = 10))
	void TryCreateLedgeStructureC(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid,
		FVector TraceOrigin = FVector(0.0, 0.0, 0.0), FVector TraceDirection = FVector(0.0, 0.0, 0.0), float Z_Offset = 0.0,
		float ForwardTraceLength = 45.0, bool UseWallCondition = true);
	virtual void TryCreateLedgeStructureC_Implementation(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid,
		FVector TraceOrigin = FVector(0.0, 0.0, 0.0), FVector TraceDirection = FVector(0.0, 0.0, 0.0), float Z_Offset = 0.0,
		float ForwardTraceLength = 45.0, bool UseWallCondition = true);

	

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder|Default Method", meta = (Keywords = "Ledge,Finder,Core", AdvancedDisplay = 9))
	void TryCreateLedgeUsingDefaultSolver(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid,
		FVector TraceOrigin = FVector::ZeroVector, FVector TraceDirection = FVector::ZeroVector, FVector2D AxisNormals = FVector2D(0.0, 0.0), float TracingOriginOffsetZ = 0.0f,
		FAGLS_LedgeFinderConfig_DefaultSolver SolverSettings = FAGLS_LedgeFinderConfig_DefaultSolver(), bool UseWallCondition = true);


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Do While Climbing|Corner", meta = (DisplayName = "Check Can Start Corner", Keywords = "CPP Corner"))
	void CheckCanStartCornerC(bool& DetectedCorner, bool& OuterType, FCMC_LedgeC& TargetLedgeStruct, bool Valid=true, bool InputLock=false);
	virtual void CheckCanStartCornerC_Implementation(bool& DetectedCorner, bool& OuterType, FCMC_LedgeC& TargetLedgeStruct, bool Valid = true, bool InputLock = false);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Do While Climbing|Jumps", meta = (DisplayName = "Check Can Jump Back", Keywords = "CPP"))
	void CheckCanJumpBackC(bool& ReturnValue, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, 
	bool UseCameraCondition = true, float JumpMaxDistance = 220);
	virtual void CheckCanJumpBackC_Implementation(bool& ReturnValue, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint,
	bool UseCameraCondition = true, float JumpMaxDistance = 220);


#pragma endregion

#pragma region Functions - ROPE SWIMGING With Override in BP

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Rope Swing System|Base", meta = (DisplayName = "Rope Hooked Condition", Keywords = "Rope Swing System"))
	bool RopeHookedConditionC();

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Rope Swing System|Base", meta = (DisplayName = "Check Normal For Point", Keywords = "Rope Swing System"))
	bool CheckNormalForPointC(FExposedCableParticle& InParticle);

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Detach Rope Or End Swing", Keywords = "Rope Swing System"))
	bool DetachRopeOrEndSwingC();

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Finish Rope Swing", Keywords = "Rope Swing System"))
	bool FinishRopeSwingC();

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Zipline System", meta = (DisplayName = "Started Zipline", Keywords = "Zipline"))
	bool StartedZiplineC();

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (DisplayName = "Started Pickaxe Climb ", Keywords = "Zipline"))
	bool StartedPickaxeClimbC();
	
#pragma endregion

#pragma region Functions - NEW LEDGE SOLVER functions collection ADDED FOR AGLS v2.0

	//AGLS v2.0

	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0
	 * Searches for a single ledge point in the specified direction.
	 *
	 * Method:
	 * 1. CapsuleTraceMulti finds an initial wall contact.
	 * 2. Vertical LineTraceMulti queries search for a surface near the contact.
	 *    Their offset direction is interpolated between the search direction
	 *    and the vector obtained from the wall normal.
	 * 3. SphereTraceMulti determines the wall normal near the detected surface.
	 * 4. Short LineTraceSingle queries validate the ledge profile.
	 *
	 * The ledge position combines the wall contact's XY coordinates with
	 * the upper surface's Z coordinate. The resulting rotation is derived
	 * from NormalToVector applied to the wall normal.
	 *
	 * Returns true when a valid ledge is found. PointTransform, WallHit,
	 * and SurfaceHit describe the result; when false is returned, they
	 * must not be treated as a valid climbing point.
	 *
	 * SearchDirection is not normalized internally.
	 * MaxLedgeWidth and MinLedgeWidth remain unused, matching the original BP.
	 * ReturnQueries counts inspected MultiTrace hits and final LineTraceSingle
	 * queries, rather than the total number of trace calls.
	 *
	 * Early returns from the original Blueprint graph are preserved:
	 * the function may return before checking all available candidates.
	 */
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder|Complex Tracing Method", meta = (AdvancedDisplay = 11))
	bool FindSingleLedgePointUsingComplexTraces(
		ACharacter* InCharacter,
		FTransform& PointTransform,
		FHitResult& WallHit,
		FHitResult& SurfaceHit,
		int32& ReturnQueries,
		FVector SearchOrigin = FVector::ZeroVector,
		FVector SearchDirection = FVector::ZeroVector,
		TEnumAsByte<ECollisionChannel> Channel = ECC_Visibility,
		double MaxLedgeWidth = 30.0,
		double MinLedgeWidth = 5.0,
		double ForwardWallSearchLength = 80.0,
		double ForwardWallSearchRadius = 25.0,
		int32 UpTracesChecksNumber = 5,
		int32 MaxUpTracesCandidates = 2,
		int PhaseCheckTracesCount = 2,
		float PhaseTollerance = 50,
		FVector2D SurfaceLineTraceLength = FVector2D(20, 20),
		int DebugingModeIndex = 0,
		float DebugDrawTime = 0.1f
	);


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0
	 * Validates a pair of ledge points and the space available for the character capsule.
	 *
	 * Checks the distance between the points, their height difference,
	 * and their orientation. Within a specific length range, the right endpoint
	 * may be shortened while preserving its rotation and scale, then verified
	 * with a trace.
	 *
	 * A trace at the ledge midpoint confirms the surface and identifies
	 * the contact component. The center transform is calculated from both
	 * endpoints, retaining only the Yaw rotation.
	 *
	 * UseComplexCapsuleFreeSpaceCheck selects the clearance check method:
	 * - true: FindUnblockedCapsule with additional acceptance conditions
	 *   for capsule half-height, endpoint reductions, and the query limit;
	 * - false: the existing vertical sphere sweep with gradual radius
	 *   reduction or shortening of the lower sweep extent.
	 *
	 * CapsuleHeight is treated as half the capsule's total height,
	 * including the hemispheres. CapsuleRoomQueries is passed by reference:
	 * the function increments its value without resetting it.
	 *
	 * Returns true when the ledge and capsule clearance are accepted.
	 * ReturnCapsulePosition describes the nominal capsule position at the ledge,
	 * including in the FindUnblockedCapsule branch; it is not the reduced
	 * capsule's center. On failure, some outputs may contain intermediate results.
	 */
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder|Complex Tracing Method", meta = (AdvancedDisplay = 11))
	bool VerifyLedgeGeneratedUsingComplexTraces(
		FTransform InLedgeLeft,
		FTransform InLedgeRight,
		UPARAM(ref) int32& CapsuleRoomQueries,
		FTransform& ReturnLedgeLeft,
		FTransform& ReturnLedgeRight,
		FTransform& ReturnLedgeCenter,
		FTransform& ReturnCapsulePosition,
		UPrimitiveComponent*& ReturnComponent,
		TEnumAsByte<ECollisionChannel> Channel = ECC_Visibility,
		double CapsuleRadius = 30.0,
		double CapsuleHeight = 90.0,
		double ForwardCapOffset = 2.0,
		double CapsuleOffsetZ = -50.0,
		FAGLS_LedgeFinderConfig_ComplexTracigMethod Settings = FAGLS_LedgeFinderConfig_ComplexTracigMethod(),
		int DrawDebugTracesIndex = 0,
		int DrawDebugShapesIndex = 0,
		float DrawDebugsTime = 0.1f
	);


private:

	
	/* ⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0
	 * Searches for an unblocked capsule by reducing its radius and/or height.
	 *
	 * The capsule is aligned with the world Z axis. SphereTraceSingle sweeps
	 * a sphere between the hemisphere centers to test the capsule volume.
	 * HalfHeight includes the hemispheres and must satisfy HalfHeight >= Radius > 0.
	 *
	 * The initial reduction method is selected from the contact normal and region:
	 * a lateral hit within the cylindrical region favors radius reduction,
	 * while a contact outside that region favors shortening the corresponding end.
	 * A contact Distance2D below MinimumRadius rules out radius reduction.
	 *
	 * The preferred reduction sequence is A, A, B, A, B...,
	 * where A and B represent radius and height reduction.
	 * The height side is locked when the first height reduction is applied.
	 * Flags, the contact region, and available limits take precedence
	 * over the sequence and may require the other available method.
	 *
	 * The target radius is calculated from the XY distance to the contact,
	 * while height reduction is calculated from its Z position, accounting
	 * for CollisionPadding. During initial overlap, the contact may be estimated
	 * from Normal and PenetrationDepth. Step parameters provide fallback progress.
	 *
	 * Shortening one end shifts the center while preserving the opposite end.
	 * MaxTopReduction and MaxBottomReduction limit cumulative reductions;
	 * -1 applies only the limit imposed by valid capsule geometry.
	 *
	 * Returns true only for a tested shape with no blocking collision.
	 * On success, outputs describe its geometry, volume, volume difference,
	 * and reduction percentage. On failure, solution outputs remain invalid.
	 * OutHit retains the last blocking hit, even after a successful search.
	 *
	 * Performs at most min(MaxTracesQueries, 12) queries, including the original shape.
	 * This is a heuristic: it does not guarantee finding a solution or preserving
	 * the largest possible volume. It does not modify the character's capsule.
	 */
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder|Complex Tracing Method", meta = (AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = 20))
	bool FindUnblockedCapsule(
		FVector CapsuleCenter, float Radius, float HalfHeight, const TArray<AActor*>& ActorsToIgnore, double& InputVolume, double& OutputVolume, double& VolumeDifference, double& ReductionPercent, FHitResult& OutHit,
		FVector& OutCapsuleCenter, float& OutRadius, float& OutHalfHeight, float& OutTopReduction, float& OutBottomReduction, int32& OutQueriesUsed, bool& bOutQueryLimitReached,
		TEnumAsByte<ECollisionChannel> Channel = ECC_Visibility,
		bool bTraceComplex = true,
		bool bIgnoreSelf = true,
		int32 MaxTracesQueries = 12,
		bool CanReduceRadius = true,
		bool CanReduceHeightFromTop = true,
		bool CanReduceHeightFromBottom = true,
		float RadiusReductionStep = 2.0f,
		float HeightReductionStep = 5.0f,
		float MinimumRadius = 2.0f,
		float CollisionPadding = 0.5f,
		float MaxTopReduction = -1.0f,
		float MaxBottomReduction = -1.0f,
		FLinearColor TraceColor = FLinearColor::Black, FLinearColor TraceHitColor = FLinearColor::Red, float DrawTime = 1.0f
	);


public:


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0*/
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Core|Ledge Finder|Complex Tracing Method", meta = (AutoCreateRefTerm = "ClassToIgnoreByTraces", AdvancedDisplay = 14))
	bool TryCreateLedgeUsingComplexTracesMethod(
		FCMC_SingleClimbPointC& LeftPoint,
		FCMC_SingleClimbPointC& RightPoint,
		FCMC_SingleClimbPointC& OriginPoint,
		int& TotalTracesQueries,
		const TArray<UClass*>& ClassToIgnoreByTraces,
		ACharacter* InCharacter = nullptr,
		FVector TraceOrigin = FVector::ZeroVector,
		FVector TraceDirection = FVector::ZeroVector,
		float InCapsuleRadius = 30.0f,
		float InCapsuleHalfHeight = 90.0f,
		float CapsuleOffsetFromWall = 0.0f,
		float CapsuleOffsetFromLedgeUpAxis = -50.0f,
		TEnumAsByte<ECollisionChannel> TracesChannel = ECC_Visibility,
		FAGLS_LedgeFinderConfig_ComplexTracigMethod SolverSettings = FAGLS_LedgeFinderConfig_ComplexTracigMethod(),
		bool CanDrawDebugShapes = false,
		bool CanDrawDebugTraces = false,
		float DrawDebugTime = 0.1f
	);

#pragma endregion

#pragma region Functions - CLIMBING SYSTEM EXTENSION FUNCTIONS - declarated for AGLS v2.0


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0
		* Finds a wall during falling, generates ledges at successive heights,
		* and selects the accepted candidate with the lowest accumulated weight.
		* Optionally searches for a swinging beam and redirects wall detection.
		* Preserves the original graph's early exits and query-count semantics.
	*/ 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Core|When NOT Climbing", meta = (AdvancedDisplay = 8))
	bool DetermineWallPositionAndFindLedgeDuringTheFall
	(
		FCMC_SingleClimbPointC & LeftPoint, 
		FCMC_SingleClimbPointC & RightPoint, 
		FCMC_SingleClimbPointC & OriginPoint,
		bool& NoEvenWallHit,
		int& CheckedLedgesNumber,
		FVector InitialCheckPosition = FVector::ZeroVector,
		FVector InitialCheckDirection = FVector::ZeroVector,
		float LedgeCheckOffsetZ = 0.0f,
		float LedgeCheckForwardWallTraceScale = 1.0f,
		bool CanUsePredictFallPosition = true,
		int MaxLedgeCheckExecute = 3,
		int MaxTotalTracesQueries = 100,
		int DrawAdditiveTracesIndex = 3
	);
	virtual bool DetermineWallPositionAndFindLedgeDuringTheFall_Implementation(FCMC_SingleClimbPointC & LeftPoint, FCMC_SingleClimbPointC & RightPoint, FCMC_SingleClimbPointC & OriginPoint, bool& NoEvenWallHit,
		int& CheckedLedgesNumber, FVector InitialCheckPosition = FVector::ZeroVector, FVector InitialCheckDirection = FVector::ZeroVector, float LedgeCheckOffsetZ = 0.0f, float LedgeCheckForwardWallTraceScale = 1.0f,
		bool CanUsePredictFallPosition = true, int MaxLedgeCheckExecute = 3, int MaxTotalTracesQueries = 100, int DrawAdditiveTracesIndex = 3 );


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Core|When NOT Climbing", meta = (AdvancedDisplay = 8))
	bool PrepareAndStartLedgeClimbing(bool& UseFreeHang, FCMC_SingleClimbPointC InLeftPoint, FCMC_SingleClimbPointC InRightPoint, FCMC_SingleClimbPointC InOriginPoint);
	virtual bool PrepareAndStartLedgeClimbing_Implementation(bool& UseFreeHang, FCMC_SingleClimbPointC InLeftPoint, FCMC_SingleClimbPointC InRightPoint, FCMC_SingleClimbPointC InOriginPoint);


	/*⛏︎ 𝐃𝐘𝐍𝐀𝐌𝐈𝐂 𝐋𝐄𝐃𝐆𝐄 𝐂𝐋𝐈𝐌𝐁𝐈𝐍𝐆 𝐂𝐎𝐌𝐏𝐎𝐍𝐄𝐍𝐓 + 𝘙𝘖𝘗𝘌 𝘚𝘞𝘐𝘕𝘎𝘐𝘕𝘎 + 𝘡𝘐𝘗𝘓𝘐𝘕𝘌 + 𝘞𝘈𝘓𝘓 𝘊𝘓𝘐𝘔𝘉 ⛰︎ ★𝐀𝐆𝐋𝐒  𝐯2.0*/
	UFUNCTION(BlueprintPure, Category = "New Climbing System|Core|When NOT Climbing", meta = (AdvancedDisplay = 1))
	void GetModifyParametersForWallDetection
	(
		int& AdditiveLedgeCheckInterations,
		float& OverrideSimTime,
		bool& bCanSearchForBeamForSwinging,
		float& AimWallDetectionTraceOnBeamPosition,
		int& OverrideWallDetectionMode,
		int& AddMaxTotalTracesCallPerExecution
	) const;

#pragma endregion

private:

	float GetSafeDeltaTime();

	bool GetHitIsBrushComponent(UPrimitiveComponent* InHitComponent);

	bool ClassToIgnoreSafe(FHitResult InHit, TArray<UClass*> ToIgnore, UPrimitiveComponent* HitComponent = nullptr);

};
