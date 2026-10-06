// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "AGLS_BlueprintFunctionsLibraryP2.generated.h"

/*
A list of parameters and required values ​​for the extended version of the trajectory generation function. 
This allows for greater control over trajectory fading. */
USTRUCT(BlueprintType)
struct FPoseSearchTrajectoryFacingProperties : public FTableRowBase
{
	GENERATED_BODY()

	/*
	Static Facing causes StaticDesiredLookingYaw and StaticDesiredFacing to be used as the default direction values.
	Without this option enabled, those values are Character->GetViewRotation().Yaw and TrajectoryDataState.DesiredControllerYawLastUpdate.
	Using bUseStaticFacing takes effect before the UpdatePrediction_SimulateCharacterMovement function is executed, which can still influence 
	the final facing per sample result.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	bool bUseStaticFacing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUseStaticFacing"))
	float StaticDesiredLookingYaw = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUseStaticFacing || bUsePhysicalFacing"))
	FQuat StaticDesiredFacing = FQuat::Identity;

	/*
	bUseLerpingMode is an option already in effect on UpdatePrediction_SimulateCharacterMovement. It allows linear interpolation between 
	BeginingFacingToLerp and the current Facing value of the trajectory sample being processed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	bool bUseLerpingMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUseLerpingMode || bUsePhysicalFacing"))
	FQuat BeginingFacingToLerp = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUseLerpingMode"))
	int LerpingSamplesNumber = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUseLerpingMode"))
	bool bUseLerpShortestPath = false;

	/*
	This option require:
	FQuat BeginingFacingToLerp - By default this value can be CombineRotators(GetActorRotation(), FRotator(0, -90, 0)
	FQuat StaticDesiredFacin - By default this value can be CombineRotators(GetControlRotator().Yaw, FRotator(0, -90, 0)
	float InRotationRate - Can be CharacterMovementComponent->GetRotationRate.Yaw
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing")
	bool bUsePhysicalFacing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facing", meta = (EditCondition = "bUsePhysicalFacing"))
	float InRotationRate = 80.0;

};




UCLASS()
class HELPFULFUNCTIONS_API UAGLS_BlueprintFunctionsLibraryP2 : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe, DisplayName = "Override Trajectory Facing", Keywords = "Pose,Search,Trajectory,Facing"), Category = "Animation|PoseSearch")
	static FTransformTrajectory OverrideTrajectoryFacing(UPARAM(ref) FTransformTrajectory& InTrajectoryData, FRotator InFacing, float NewFacingAlpha = 1.0, bool bIncludeHistory = true);

public:

	UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch", meta = (BlueprintThreadSafe, DisplayName = "Pose Search Generate Trajectory (Extended)", AdvancedDisplay = 12))
	static void PoseSearchGenerateTransformTrajectoryExtend
	(
		const UObject* InAnimInstance,
		UPARAM(ref) const FPoseSearchTrajectoryData& InTrajectoryData, 
		float InDeltaTime,
		UPARAM(ref) FTransformTrajectory& InOutTrajectory, 
		UPARAM(ref) float& InOutDesiredControllerYawLastUpdate,
		FVector InAcceleration,
		FVector InVelocity,
		bool bSolveRootMotionAsMovement,
		FPoseSearchTrajectoryFacingProperties FacingProperties,
		FTransformTrajectory& OutTrajectory,
		float InHistorySamplingInterval = 0.04f, 
		int32 InTrajectoryHistoryCount = 10, 
		float InPredictionSamplingInterval = 0.2f, 
		int32 InTrajectoryPredictionCount = 8
	);


	static bool PoseSearchTrajectoryUpdateData(
		FPoseSearchTrajectoryData InData, 
		float DeltaTime, 
		const FAnimInstanceProxy& AnimInstanceProxy, 
		FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived, 
		FPoseSearchTrajectoryData::FState& TrajectoryDataState,
		FVector InAcceleration,
		FVector InVelocity,
		FPoseSearchTrajectoryFacingProperties InFacingProperties,
		FQuat InRequiredFacing,
		bool bSolveRootMotionAsMovement
	);

	static bool PoseSearchTrajectoryUpdateData(
		FPoseSearchTrajectoryData InData,
		float DeltaTime,
		const UObject* Context,
		FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived,
		FPoseSearchTrajectoryData::FState& TrajectoryDataState,
		FVector InAcceleration,
		FVector InVelocity,
		FPoseSearchTrajectoryFacingProperties InFacingProperties,
		FQuat InRequiredFacing,
		bool bSolveRootMotionAsMovement
	);



	UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch", meta = (BlueprintThreadSafe, DisplayName = "Calculate Acceleration For Trajectory Generator", AdvancedDisplay = 5))
	static FVector MakeAccelerationValueForTrajectory
	(
		bool& IsPlayingRootMotage,
		FRotator& MontageFacing,
		const UAnimInstance* InAnimInstance, 
		float DeltaTime, 
		UPARAM(ref) FVector& Acceleration, 
		bool PredictAccelerationFromRootMotion = true,
		float ExtractTimeOffset = 0.0,
		float MaxTimeOffset = 0.0,
		float RotationExtrapolationOffset = 0.0,
		int RotationPredictionFrameNums = 2
	);

	UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch", meta = (BlueprintThreadSafe, DisplayName = "Calculate Velocity For Trajectory Generator", AdvancedDisplay = 3))
	static FVector MakeVelocityForTrajectory
	(
		const UAnimInstance* InAnimInstance,
		float DeltaTime,
		UPARAM(ref) FVector& Velocity,
		UPARAM(ref) FVector& PrevPosition,
		bool PredictAccelerationFromRootMotion = true,
		float VelocityScale = 1.0
	);


	UFUNCTION(BlueprintPure, Category = "Animation|PoseSearch", meta = (DisplayName = "Try Extract Root Motion From Range"))
	static FTransform TryExtractRootMotionFromRange(const UAnimMontage* Montage, float StartTime, float EndTime);



	//UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch|Experimental", meta = (BlueprintThreadSafe, DisplayName = "HandleTrajectoryWorldCollisions (Fixed)", WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "TraceChannel,bTraceComplex,ActorsToIgnore,DrawDebugType,bIgnoreSelf,MaxObstacleHeight,TraceColor,TraceHitColor,DrawTime"))
	static void CustomHandleTransformTrajectoryWorldCollisions(const UObject* WorldContextObject,
		UPARAM(ref) const FTransformTrajectory& InTrajectory, FVector StartingVelocity, bool bApplyGravity, FVector GravityAccel, float FloorCollisionsOffset, FTransformTrajectory& OutTrajectory, FPoseSearchTrajectory_WorldCollisionResults& CollisionResult,
		ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, bool bIgnoreSelf = true, float MaxObstacleHeight = 10000.f, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);


	//A function that returns the PoseSearchDatabases currently loaded into memory and those waiting to be loaded. This function is available for ThreadSafe logic.
	UFUNCTION(BlueprintPure, Category = "Pose Search", meta = (BlueprintThreadSafe, DisplayName = "Resolve Loaded Pose Search Databases"))
	static bool ResolveLoadedPoseSearchDatabases
	(
		const TArray<TSoftObjectPtr<UPoseSearchDatabase>>& SoftDatabases,
		TArray<UPoseSearchDatabase*>& AvaliableDatabases,
		TArray<TSoftObjectPtr<UPoseSearchDatabase>>& NotLoadedDatabases
	);


	static void UpdatePrediction_SimulateCharacterMovement(FTransformTrajectory& Trajectory,
		const FPoseSearchTrajectoryData& TrajectoryData, const FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived,
		const FPoseSearchTrajectoryData::FSampling& TrajectoryDataSampling, float DeltaTime, const FPoseSearchTrajectoryFacingProperties& FacingValues);


	static FQuat RotateYawTowardsConstant(const FQuat& CurrentFacing, const FQuat& DesiredFacing, float DeltaSeconds, float RotationRateDegPerSecond);




	// Update history by tracking offsets that result from character intent (e.g. movement component velocity) and applying
	// that to the current world transform. This works well on moving platforms as it only stores a history of movement
	// that results from character intent, not movement from platforms.
	// Important: CurrentVelocity should be the velocity relative to the ground as reported by the character movement component or character mover etc
	static void UpdateHistory_TransformHistoryWithZ(FTransformTrajectory& Trajectory, FVector CurrentPosition, FVector CurrentVelocity, const FPoseSearchTrajectoryData::FSampling& TrajectoryDataSampling, float DeltaTime);



	UFUNCTION(BlueprintCallable, Category = "Animation|PoseSearch|Experimental", meta = (BlueprintThreadSafe, DisplayName = "Custom Handle Transform Trajectory Stair World Collisions", WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = 10))
	static void CustomHandleTransformTrajectoryStairWorldCollisions(
		const UObject* WorldContextObject,
		UPARAM(ref) const FTransformTrajectory& InTrajectory,
		FVector StartingVelocity,
		bool bApplyGravity,
		FVector GravityAccel,
		float FloorCollisionsOffset,
		FTransformTrajectory& OutTrajectory,
		FPoseSearchTrajectory_WorldCollisionResults& CollisionResult,
		ETraceTypeQuery TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		EDrawDebugTrace::Type DrawDebugType,
		bool bIgnoreSelf = true,
		float MaxObstacleHeight = 80.0f,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor TraceHitColor = FLinearColor::Green,
		float DrawTime = 0.0f,

		// Stair-specific settings.
		float StairFloorTraceUpDistance = 80.0f,
		float StairFloorTraceDownDistance = 160.0f,
		float StairHorizontalTraceBackDistance = 20.0f,
		float StairHorizontalTraceForwardDistance = 90.0f,
		float StairHorizontalTraceZOffset = -3.0f,
		float StairEdgeSnapOffset = 0.0f,
		float StairMaxRiserNormalUpDot = 0.25f,
		float StairMinRiserFacingDot = 0.35f,
		float StairMinForwardSpacing = 1.0f,
		float StairMaxSnapDistanceAlongMove = 90.0f,
		bool bSnapOnlyToOuterStairEdges = true,
		bool bKeepTrajectoryMonotonic = true
	);




};
