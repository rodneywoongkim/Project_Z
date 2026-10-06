// Fill out your copyright notice in the Description page of Project Settings.


#include "AGLS_BlueprintFunctionsLibraryP2.h"
#include "Animation/AnimInstanceProxy.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchDefines.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/TrajectoryTypes.h"

#define PSTL UPoseSearchTrajectoryLibrary 


FTransformTrajectory UAGLS_BlueprintFunctionsLibraryP2::OverrideTrajectoryFacing(UPARAM(ref) FTransformTrajectory& InTrajectoryData, FRotator InFacing, float NewFacingAlpha, bool bIncludeHistory)
{
	const FQuat FacingQuat = InFacing.Quaternion();
	TArray<FTransformTrajectorySample> NewSamples;

	for (int i = 0; i < InTrajectoryData.Samples.Num() - 1; i++)
	{
		//FTransformTrajectorySample Sample = InTrajectoryData.Samples[i];
		//FTransformTrajectorySample NewSample = Sample;

		//if (bIncludeHistory == false)
		//{
		//	if (Sample.TimeInSeconds < 0.0)
		//	{
		//		NewSamples.Add(NewSample);
		//		continue;
		//	}
		//}
		
		//InTrajectoryData.Samples[0].Facing = FQuat::Slerp(Sample.Facing, FacingQuat, NewFacingAlpha);
		//NewSample.Facing = FQuat::Slerp(Sample.Facing, FacingQuat, NewFacingAlpha);
		//NewSamples.Add(NewSample);

		FQuat DefFacing = InTrajectoryData.Samples[i].Facing;
		if (bIncludeHistory)
		{
			InTrajectoryData.Samples[i].Facing = FQuat::Slerp(DefFacing, FacingQuat, NewFacingAlpha);
		}
		else if (InTrajectoryData.Samples[i].TimeInSeconds >= 0.0)
		{
			InTrajectoryData.Samples[i].Facing = FQuat::Slerp(DefFacing, FacingQuat, NewFacingAlpha);
		}
	}

	//InTrajectoryData.Samples = NewSamples;

	return InTrajectoryData;
}


void UAGLS_BlueprintFunctionsLibraryP2::PoseSearchGenerateTransformTrajectoryExtend(
	const UObject* InContext,
	UPARAM(ref) const FPoseSearchTrajectoryData& InTrajectoryData, 
	float InDeltaTime, 
	UPARAM(ref)FTransformTrajectory& InOutTrajectory, 
	UPARAM(ref) float& InOutDesiredControllerYawLastUpdate, 
	FVector InAcceleration, 
	FVector InVelocity,
	bool bSolveRootMotionAsMovement,
	FPoseSearchTrajectoryFacingProperties FacingProperties,
	FTransformTrajectory& OutTrajectory, 
	float InHistorySamplingInterval, 
	int32 InTrajectoryHistoryCount, 
	float InPredictionSamplingInterval, 
	int32 InTrajectoryPredictionCount
)
{

	FPoseSearchTrajectoryData::FSampling TrajectoryDataSampling;
	TrajectoryDataSampling.NumHistorySamples = InTrajectoryHistoryCount;
	TrajectoryDataSampling.SecondsPerHistorySample = InHistorySamplingInterval;
	TrajectoryDataSampling.NumPredictionSamples = InTrajectoryPredictionCount;
	TrajectoryDataSampling.SecondsPerPredictionSample = InPredictionSamplingInterval;

	FPoseSearchTrajectoryData::FState TrajectoryDataState;
	TrajectoryDataState.DesiredControllerYawLastUpdate = InOutDesiredControllerYawLastUpdate;

	FPoseSearchTrajectoryData::FDerived TrajectoryDataDerived;

	UAGLS_BlueprintFunctionsLibraryP2::PoseSearchTrajectoryUpdateData(InTrajectoryData, InDeltaTime, InContext, TrajectoryDataDerived, TrajectoryDataState, InAcceleration, InVelocity, 
		FacingProperties, FacingProperties.StaticDesiredFacing, bSolveRootMotionAsMovement);

	PSTL::InitTrajectorySamples(InOutTrajectory, TrajectoryDataDerived.Position, TrajectoryDataDerived.Facing, TrajectoryDataSampling, InDeltaTime);


	UAGLS_BlueprintFunctionsLibraryP2::UpdateHistory_TransformHistoryWithZ(InOutTrajectory, TrajectoryDataDerived.Position, TrajectoryDataDerived.Velocity, TrajectoryDataSampling, InDeltaTime);



	UAGLS_BlueprintFunctionsLibraryP2::UpdatePrediction_SimulateCharacterMovement(InOutTrajectory, InTrajectoryData, TrajectoryDataDerived, TrajectoryDataSampling, InDeltaTime, FacingProperties);

	InOutDesiredControllerYawLastUpdate = TrajectoryDataState.DesiredControllerYawLastUpdate;

	OutTrajectory = InOutTrajectory;

}

bool UAGLS_BlueprintFunctionsLibraryP2::PoseSearchTrajectoryUpdateData(FPoseSearchTrajectoryData InData, float DeltaTime, const FAnimInstanceProxy& AnimInstanceProxy,  FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived, 
	FPoseSearchTrajectoryData::FState& TrajectoryDataState, FVector InAcceleration, FVector InVelocity, FPoseSearchTrajectoryFacingProperties InFacingProperties, FQuat InRequiredFacing, bool bSolveRootMotionAsMovement)
{
	return UAGLS_BlueprintFunctionsLibraryP2::PoseSearchTrajectoryUpdateData(InData, DeltaTime, AnimInstanceProxy.GetAnimInstanceObject(), TrajectoryDataDerived, TrajectoryDataState, 
		InAcceleration, InVelocity, InFacingProperties, InRequiredFacing, bSolveRootMotionAsMovement);
}

bool UAGLS_BlueprintFunctionsLibraryP2::PoseSearchTrajectoryUpdateData(FPoseSearchTrajectoryData InData, float DeltaTime, const UObject* Context,  FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived, 
	FPoseSearchTrajectoryData::FState& TrajectoryDataState, FVector InAcceleration, FVector InVelocity, FPoseSearchTrajectoryFacingProperties InFacingProperties, FQuat InRequiredFacing, bool bSolveRootMotionAsMovement)
{
	const ACharacter* Character = Cast<ACharacter>(Context);
	if (!Character)
	{
		if (const UAnimInstance* AnimInstance = Cast<UAnimInstance>(Context))
		{
			Character = Cast<ACharacter>(AnimInstance->GetOwningActor());
		}
		else if (const UActorComponent* AnimNextComponent = Cast<UActorComponent>(Context))
		{
			Character = Cast<ACharacter>(AnimNextComponent->GetOwner());
		}

		if (!Character)
		{
			return false;
		}
	}

	const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	const USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MoveComp || !MeshComp)
	{
		return false;
	}

	float InputModifier = MoveComp->GetAnalogInputModifier();
	if (bSolveRootMotionAsMovement && Character->HasAnyRootMotion())
	{
		InputModifier = 1.0;
	}

	TrajectoryDataDerived.MaxSpeed = FMath::Max(MoveComp->GetMaxSpeed() * InputModifier, MoveComp->GetMinAnalogSpeed());
	TrajectoryDataDerived.BrakingDeceleration = FMath::Max(0.f, MoveComp->GetMaxBrakingDeceleration());
	TrajectoryDataDerived.BrakingSubStepTime = MoveComp->BrakingSubStepTime;
	TrajectoryDataDerived.bOrientRotationToMovement = MoveComp->bOrientRotationToMovement;

	TrajectoryDataDerived.Velocity = InVelocity;
	if (InVelocity.Equals(FVector(-1, -1, -1)) == true)
	{
		TrajectoryDataDerived.Velocity = MoveComp->Velocity;
	}

	TrajectoryDataDerived.Acceleration = InAcceleration;

	TrajectoryDataDerived.bStepGroundPrediction = !MoveComp->IsFalling() && !MoveComp->IsFlying();

	if (TrajectoryDataDerived.Acceleration.IsZero())
	{
		TrajectoryDataDerived.Friction = MoveComp->bUseSeparateBrakingFriction ? MoveComp->BrakingFriction : MoveComp->GroundFriction;
		const float FrictionFactor = FMath::Max(0.f, MoveComp->BrakingFrictionFactor);
		TrajectoryDataDerived.Friction = FMath::Max(0.f, TrajectoryDataDerived.Friction * FrictionFactor);
	}
	else
	{
		TrajectoryDataDerived.Friction = MoveComp->GroundFriction;
	}

	//const float DesiredControllerYaw = Character->GetViewRotation().Yaw;
	float DesiredControllerYaw = Character->GetViewRotation().Yaw;
	if (InFacingProperties.bUseStaticFacing)
	{
		DesiredControllerYaw = InFacingProperties.StaticDesiredLookingYaw;
	}

	const float DesiredYawDelta = DesiredControllerYaw - TrajectoryDataState.DesiredControllerYawLastUpdate;
	TrajectoryDataState.DesiredControllerYawLastUpdate = DesiredControllerYaw;

	if (DeltaTime > UE_SMALL_NUMBER)
	{
		// An AnimInstance might call this during an AnimBP recompile with 0 delta time, so we don't update ControllerYawRate
		TrajectoryDataDerived.ControllerYawRate = FRotator::NormalizeAxis(DesiredYawDelta) / DeltaTime;
		if (InData.MaxControllerYawRate >= 0.f)
		{
			TrajectoryDataDerived.ControllerYawRate = FMath::Sign(TrajectoryDataDerived.ControllerYawRate) * FMath::Min(FMath::Abs(TrajectoryDataDerived.ControllerYawRate), InData.MaxControllerYawRate);
		}
	}

	TrajectoryDataDerived.Position = MeshComp->GetComponentLocation();
	TrajectoryDataDerived.MeshCompRelativeRotation = MeshComp->GetRelativeRotation().Quaternion();

	if (InFacingProperties.bUseStaticFacing)
	{
		TrajectoryDataDerived.Facing = InFacingProperties.StaticDesiredFacing;
	}
	else
	{
		if (TrajectoryDataDerived.bOrientRotationToMovement)
		{
			TrajectoryDataDerived.Facing = MeshComp->GetComponentTransform().GetRotation();
		}
		else
		{
			TrajectoryDataDerived.Facing = FQuat::MakeFromRotator(FRotator(0, TrajectoryDataState.DesiredControllerYawLastUpdate, 0)) * TrajectoryDataDerived.MeshCompRelativeRotation;
		}
	}

	return true;
}


FVector UAGLS_BlueprintFunctionsLibraryP2::MakeAccelerationValueForTrajectory(bool& IsPlayingRootMotage, FRotator& MontageFacing, const UAnimInstance* InAnimInstance, float DeltaTime, UPARAM(ref)FVector& Acceleration,
	bool PredictAccelerationFromRootMotion, float ExtractTimeOffset, float MaxTimeOffset, float RotationExtrapolationOffset, int RotationPredictionFrameNums)
{
	if (!InAnimInstance->TryGetPawnOwner()) return Acceleration;
	if (DeltaTime <= KINDA_SMALL_NUMBER) return Acceleration;

	ACharacter* AsChar = Cast<ACharacter>(InAnimInstance->TryGetPawnOwner());
	if (!AsChar) return Acceleration;

	const UCharacterMovementComponent* MoveComp = AsChar->GetCharacterMovement();
	const USkeletalMeshComponent* MeshComp = AsChar->GetMesh();
	if (!MoveComp || !MeshComp)
	{
		return Acceleration;
	}

	if (AsChar->HasAnyRootMotion() && PredictAccelerationFromRootMotion)
	{
		UAnimMontage* CurrentMontage = InAnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
		{
			const float TimeOffset = ExtractTimeOffset;
			float DynamicTimeOffset = 0.0;
			float TimeA = InAnimInstance->Montage_GetPosition(CurrentMontage);
			float TimeB = InAnimInstance->Montage_GetPosition(CurrentMontage) + (DeltaTime * 2);
			
			FAnimMontageInstance* MontageInstance = InAnimInstance->GetActiveMontageInstance();
			float TotalLenght = CurrentMontage->GetPlayLength();
			if (MontageInstance) TotalLenght = CurrentMontage->GetPlayLength() / MontageInstance->GetPlayRate();
			const float BlendOutTime = CurrentMontage->GetDefaultBlendOutTime();

			if (MaxTimeOffset > 0 && TotalLenght > (CurrentMontage->GetDefaultBlendInTime() + CurrentMontage->GetDefaultBlendOutTime()))
			{
				float BlendAlpha = 0.0;
				if (TimeA > (TotalLenght - (BlendOutTime * 2)))
				{
					BlendAlpha = UKismetMathLibrary::MapRangeClamped(TimeA, TotalLenght - (BlendOutTime * 2), TotalLenght - (BlendOutTime * 1), 1.0, 0.0);
				}
				else if(MontageInstance)
				{
					BlendAlpha = MontageInstance->GetWeight();
				}
				//GEngine->AddOnScreenDebugMessage(-1, 0.5, FColor::Yellow, FString::SanitizeFloat(BlendAlpha));
				DynamicTimeOffset = BlendAlpha * MaxTimeOffset;
			}

			TimeA = TimeA + ExtractTimeOffset + DynamicTimeOffset;
			TimeB = TimeB + ExtractTimeOffset + DynamicTimeOffset;

			FTransform RootMotionT = TryExtractRootMotionFromRange(CurrentMontage, 0.0, TimeA);

			const int SafeFramesPrediction = FMath::Clamp<int>(RotationPredictionFrameNums, 1, 50);

			IsPlayingRootMotage = true;
			MontageFacing = MoveComp->GetActorTransform().Rotator() + 
				TryExtractRootMotionFromRange(CurrentMontage, TimeA + RotationExtrapolationOffset, TimeA + (DeltaTime * SafeFramesPrediction) + RotationExtrapolationOffset).Rotator()
				+ FRotator(0, -90, 0);

			FVector PositionDelta = TryExtractRootMotionFromRange(CurrentMontage, 0.0, TimeB).GetLocation() - TryExtractRootMotionFromRange(CurrentMontage, 0.0, TimeA).GetLocation();
			//const FVector PrevPositionDelta = TryExtractRootMotionFromRange(CurrentMontage, 0.0, TimeB - DeltaTime).GetLocation() - TryExtractRootMotionFromRange(CurrentMontage, 0.0, TimeA - DeltaTime).GetLocation();
			//const FVector PhysicalAcceleration = ((PositionDelta - PrevPositionDelta) / DeltaTime) * 100;
			//GEngine->AddOnScreenDebugMessage(-1, 0.5, FColor::Yellow, PhysicalAcceleration.ToCompactString());

			if (PositionDelta.Length() > KINDA_SMALL_NUMBER)
			{
				PositionDelta.Normalize();
				const FRotator ToRotate = (MoveComp->GetActorTransform().Rotator() - RootMotionT.Rotator()) + FRotator(0, -90, 0);
				PositionDelta = UKismetMathLibrary::Quat_RotateVector(ToRotate.Quaternion(), PositionDelta);
				Acceleration = PositionDelta * MoveComp->GetMaxAcceleration();
				return Acceleration;
			}
			Acceleration = FVector::ZeroVector;
			return Acceleration;
		}
		else
		{
			FVector Velo = AsChar->GetVelocity();
			if (Velo.Length() < 0.01)
			{
				Acceleration = FVector::ZeroVector;
				IsPlayingRootMotage = false;
				return Acceleration;
			}
			else
			{
				Velo.Normalize();
				Acceleration = Velo * MoveComp->GetMaxAcceleration();
				IsPlayingRootMotage = false;
				return Acceleration;
			}
		}
	}
	else
	{
		Acceleration = MoveComp->GetCurrentAcceleration();
		IsPlayingRootMotage = false;
		return Acceleration;
	}
}


FVector UAGLS_BlueprintFunctionsLibraryP2::MakeVelocityForTrajectory(const UAnimInstance* InAnimInstance, float DeltaTime, UPARAM(ref)FVector& Velocity, UPARAM(ref) FVector& PrevPosition, 
	bool PredictAccelerationFromRootMotion, float VelocityScale)
{
	if (!InAnimInstance->TryGetPawnOwner()) return Velocity;

	ACharacter* AsChar = Cast<ACharacter>(InAnimInstance->TryGetPawnOwner());
	if (!AsChar) return Velocity;

	const UCharacterMovementComponent* MoveComp = AsChar->GetCharacterMovement();
	if (!MoveComp) { return Velocity; }

	if (AsChar->HasAnyRootMotion() && PredictAccelerationFromRootMotion)
	{
		FVector MoveDelta = AsChar->GetActorLocation() - PrevPosition;
		if (MoveDelta.Length() < 500 && DeltaTime > KINDA_SMALL_NUMBER)
		{
			Velocity = (MoveDelta / DeltaTime) * VelocityScale;
		}
		else
		{
			Velocity = MoveComp->Velocity;
		}
	}
	else
	{
		Velocity = MoveComp->Velocity;
	}
	PrevPosition = AsChar->GetActorLocation(); //Save Current Actor Position
	return Velocity; //Return Velocity
}


FTransform UAGLS_BlueprintFunctionsLibraryP2::TryExtractRootMotionFromRange(const UAnimMontage* Montage, float StartTime, float EndTime)
{
	if (!Montage) return FTransform::Identity;

	const FAnimExtractContext ExtractContext;
	return Montage->ExtractRootMotionFromTrackRange(StartTime, EndTime, ExtractContext);
}


void UAGLS_BlueprintFunctionsLibraryP2::CustomHandleTransformTrajectoryWorldCollisions(const UObject* WorldContextObject, UPARAM(ref) const FTransformTrajectory& InTrajectory, FVector StartingVelocity, 
	bool bApplyGravity, FVector GravityAccel, float FloorCollisionsOffset, FTransformTrajectory& OutTrajectory, FPoseSearchTrajectory_WorldCollisionResults& CollisionResult, ETraceTypeQuery TraceChannel, 
	bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, bool bIgnoreSelf, float MaxObstacleHeight, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	OutTrajectory = InTrajectory;

	TArray<FTransformTrajectorySample>& Samples = OutTrajectory.Samples;
	const int32 NumSamples = Samples.Num();

	FVector GravityDirection = FVector::ZeroVector;
	float GravityZ = 0.f;
	float InitialVelocityZ = StartingVelocity.Z;

	if (bApplyGravity && !GravityAccel.IsNearlyZero())
	{
		GravityAccel.ToDirectionAndLength(GravityDirection, GravityZ);
		GravityZ = -GravityZ;
		const FVector VelocityOnGravityAxis = StartingVelocity.ProjectOnTo(GravityDirection);

		InitialVelocityZ = VelocityOnGravityAxis.Length() * -FMath::Sign(GravityDirection.Dot(VelocityOnGravityAxis));
	}

	CollisionResult.TimeToLand = OutTrajectory.Samples.Last().TimeInSeconds;

	if (!FMath::IsNearlyZero(GravityZ))
	{
		FVector LastImpactPoint;
		FVector LastImpactNormal;
		bool bIsLastImpactValid = false;
		bool bIsFirstFall = true;

		const FVector Gravity = GravityDirection * -GravityZ;
		float FreeFallAccumulatedSeconds = 0.f;
		for (int32 SampleIndex = 1; SampleIndex < NumSamples; ++SampleIndex)
		{
			FTransformTrajectorySample& Sample = Samples[SampleIndex];
			if (Sample.TimeInSeconds > 0.f)
			{
				const int32 PrevSampleIndex = SampleIndex - 1;
				const FTransformTrajectorySample& PrevSample = Samples[PrevSampleIndex];

				FreeFallAccumulatedSeconds += Sample.TimeInSeconds - PrevSample.TimeInSeconds;

				DrawDebugSphere(WorldContextObject->GetWorld(), Sample.Position, 5.0, 6, FColor::Red, false, 0.0, 1, 0.5);


				if (bIsLastImpactValid)
				{
					const FPlane GroundPlane = FPlane(PrevSample.Position, -GravityDirection);
					Sample.Position = FPlane::PointPlaneProject(Sample.Position, GroundPlane);
				}

				// applying gravity
				const FVector FreeFallOffset = Gravity * (0.5f * FreeFallAccumulatedSeconds * FreeFallAccumulatedSeconds);
				Sample.Position += FreeFallOffset;

				FHitResult HitResult;
				if (FloorCollisionsOffset > 0.f && UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Sample.Position + (GravityDirection * -MaxObstacleHeight), Sample.Position - FVector(0,0,2), TraceChannel, bTraceComplex, ActorsToIgnore, DrawDebugType, HitResult, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime))
				{
					// Only allow our trace to move trajectory along gravity direction.
					LastImpactPoint = UKismetMathLibrary::FindClosestPointOnLine(HitResult.ImpactPoint, Sample.Position, GravityDirection);
					LastImpactNormal = HitResult.Normal;
					bIsLastImpactValid = true;

					Sample.Position = LastImpactPoint - GravityDirection * FloorCollisionsOffset;

					if (bIsFirstFall)
					{
						const float InitialHeight = OutTrajectory.GetSampleAtTime(0.0f).Position.Z;
						const float FinalHeight = Sample.Position.Z;
						const float FallHeight = FMath::Abs(FinalHeight - InitialHeight);

						bIsFirstFall = false;
						CollisionResult.TimeToLand = (InitialVelocityZ / -GravityZ) + ((FMath::Sqrt(FMath::Square(InitialVelocityZ) + (2.f * -GravityZ * FallHeight))) / -GravityZ);
						CollisionResult.LandSpeed = InitialVelocityZ + GravityZ * CollisionResult.TimeToLand;
					}

					DrawDebugSphere(WorldContextObject->GetWorld(), Sample.Position, 5.0, 6, FColor::Green, false, 0.0, 1, 0.5);

					FreeFallAccumulatedSeconds = 0.f;
				}
			}
		}
	}
	else if (FloorCollisionsOffset > 0.f)
	{
		for (int32 SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
		{
			FTransformTrajectorySample& Sample = OutTrajectory.Samples[SampleIndex];
			if (Sample.TimeInSeconds > 0.f)
			{
				FHitResult HitResult;
				if (UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Sample.Position + FVector::UpVector * 3000.f, Sample.Position, TraceChannel, bTraceComplex, ActorsToIgnore, DrawDebugType, HitResult, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime))
				{
					//Sample.Position.Z = HitResult.ImpactPoint.Z + FloorCollisionsOffset;
				}
			}
		}
	}

	CollisionResult.LandSpeed = InitialVelocityZ + GravityZ * CollisionResult.TimeToLand;
}





bool UAGLS_BlueprintFunctionsLibraryP2::ResolveLoadedPoseSearchDatabases(
	const TArray<TSoftObjectPtr<UPoseSearchDatabase>>& SoftDatabases, 
	TArray<UPoseSearchDatabase*>& AvaliableDatabases, 
	TArray<TSoftObjectPtr<UPoseSearchDatabase>>& NotLoadedDatabases)
{
	AvaliableDatabases.Reset();
	NotLoadedDatabases.Reset();

	bool bHasNotLoadedDatabases = false;

	for (const TSoftObjectPtr<UPoseSearchDatabase>& SoftDatabase : SoftDatabases)
	{
		UPoseSearchDatabase* Database = SoftDatabase.Get();

		if (IsValid(Database))
		{
			AvaliableDatabases.Add(Database);
		}
		else
		{
			NotLoadedDatabases.Add(SoftDatabase);
			bHasNotLoadedDatabases = true;
		}
	}

	return bHasNotLoadedDatabases;
}





FVector RemapVectorMagnitudeWithCurve(const FVector& Vector, bool bUseCurve, const FRuntimeFloatCurve& Curve)
{
	if (bUseCurve)
	{
		const float Length = Vector.Length();
		if (Length > UE_KINDA_SMALL_NUMBER)
		{
			const float RemappedLength = Curve.GetRichCurveConst()->Eval(Length);
			return Vector * (RemappedLength / Length);
		}
	}

	return Vector;
}


FQuat UAGLS_BlueprintFunctionsLibraryP2::RotateYawTowardsConstant(const FQuat& CurrentFacing, const FQuat& DesiredFacing, float DeltaSeconds, float RotationRateDegPerSecond)
{
	if (DeltaSeconds <= 0.0f || RotationRateDegPerSecond <= 0.0f)
	{
		return CurrentFacing;
	}

	const FRotator CurrentRot = CurrentFacing.Rotator();
	const FRotator DesiredRot = DesiredFacing.Rotator();

	const float CurrentYaw = CurrentRot.Yaw;
	const float DesiredYaw = DesiredRot.Yaw;

	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw);
	const float MaxStep = RotationRateDegPerSecond * DeltaSeconds;

	const float AppliedDeltaYaw = FMath::Clamp(DeltaYaw, -MaxStep, MaxStep);
	const float NewYaw = CurrentYaw + AppliedDeltaYaw;

	return FRotator(0.0f, NewYaw, 0.0f).Quaternion();
}



void UAGLS_BlueprintFunctionsLibraryP2::UpdateHistory_TransformHistoryWithZ(FTransformTrajectory& Trajectory, FVector CurrentPosition, FVector CurrentVelocity, const FPoseSearchTrajectoryData::FSampling& TrajectoryDataSampling, float DeltaTime)
{
	const int32 NumHistorySamples = TrajectoryDataSampling.NumHistorySamples;
	if (NumHistorySamples > 0)
	{
		const float SecondsPerHistorySample = TrajectoryDataSampling.SecondsPerHistorySample;

		// Trajectory should include room for history + current + future
		// So num history samples needs to be less than the total number
		check(NumHistorySamples < Trajectory.Samples.Num());

		// Trajectory.Samples[NumHistorySamples] is last frame position! (assuming this is called every frame)
		const FVector CurrentTranslationFromMover = CurrentVelocity * DeltaTime;
		const FVector TranslationSinceLastFrame = CurrentPosition - Trajectory.Samples[NumHistorySamples].Position;
		const FVector InferredGroundTranslation = TranslationSinceLastFrame - CurrentTranslationFromMover;

		// Shift history Samples when it's time to record a new one.
		if (SecondsPerHistorySample <= 0.f || FMath::Abs(Trajectory.Samples[NumHistorySamples - 1].TimeInSeconds) >= SecondsPerHistorySample)
		{
			for (int32 Index = 0; Index < NumHistorySamples - 1; ++Index)
			{
				Trajectory.Samples[Index].TimeInSeconds = Trajectory.Samples[Index + 1].TimeInSeconds - DeltaTime;
				Trajectory.Samples[Index].Position = Trajectory.Samples[Index + 1].Position + InferredGroundTranslation;
				Trajectory.Samples[Index].Facing = Trajectory.Samples[Index + 1].Facing;
			}

			// Adding a new history record
			// Copy over the last frame's current transform (stored at i == NumHistorySamples) into a sample at t = 0
			Trajectory.Samples[NumHistorySamples - 1].TimeInSeconds = 0.0f;
			Trajectory.Samples[NumHistorySamples - 1].Position = Trajectory.Samples[NumHistorySamples].Position;
			Trajectory.Samples[NumHistorySamples - 1].Facing = Trajectory.Samples[NumHistorySamples].Facing;
		}
		else
		{
			// Didn't record a new history position, update timers and shift by ground translation

			for (int32 Index = 0; Index < NumHistorySamples; ++Index)
			{
				Trajectory.Samples[Index].TimeInSeconds -= DeltaTime;
				Trajectory.Samples[Index].Position += InferredGroundTranslation;
			}
		}
	}
}



void UAGLS_BlueprintFunctionsLibraryP2::UpdatePrediction_SimulateCharacterMovement(FTransformTrajectory& Trajectory,
	const FPoseSearchTrajectoryData& TrajectoryData, const FPoseSearchTrajectoryData::FDerived& TrajectoryDataDerived,
	const FPoseSearchTrajectoryData::FSampling& TrajectoryDataSampling, float DeltaTime, const FPoseSearchTrajectoryFacingProperties& FacingValues)
{
	FVector CurrentPositionWS = TrajectoryDataDerived.Position;
	FVector CurrentVelocityWS = RemapVectorMagnitudeWithCurve(
		TrajectoryDataDerived.Velocity,
		TrajectoryData.bUseSpeedRemappingCurve,
		TrajectoryData.SpeedRemappingCurve);

	FVector CurrentAccelerationWS = RemapVectorMagnitudeWithCurve(
		TrajectoryDataDerived.Acceleration,
		TrajectoryData.bUseAccelerationRemappingCurve,
		TrajectoryData.AccelerationRemappingCurve);

	// Bending CurrentVelocityWS towards CurrentAccelerationWS
	if (TrajectoryData.BendVelocityTowardsAcceleration > UE_KINDA_SMALL_NUMBER && !CurrentAccelerationWS.IsNearlyZero())
	{
		const float CurrentSpeed = CurrentVelocityWS.Length();
		const FVector VelocityWSAlongAcceleration = CurrentAccelerationWS.GetUnsafeNormal() * CurrentSpeed;

		if (TrajectoryData.BendVelocityTowardsAcceleration < 1.f - UE_KINDA_SMALL_NUMBER)
		{
			CurrentVelocityWS = FMath::Lerp(
				CurrentVelocityWS,
				VelocityWSAlongAcceleration,
				TrajectoryData.BendVelocityTowardsAcceleration);

			const float NewLength = CurrentVelocityWS.Length();
			if (NewLength > UE_KINDA_SMALL_NUMBER)
			{
				CurrentVelocityWS *= CurrentSpeed / NewLength;
			}
			else
			{
				// @todo: consider setting the CurrentVelocityWS = VelocityWSAlongAcceleration if vel and acc are in opposite directions
			}
		}
		else
		{
			CurrentVelocityWS = VelocityWSAlongAcceleration;
		}
	}

	FQuat CurrentFacingWS = TrajectoryDataDerived.Facing;

	// Physical facing state.
	// U¿ywane tylko wtedy, gdy FacingValues.bUsePhysicalFacing == true.
	// Pierwszy sample predykcji ma dostaæ dok³adnie BeginingFacingToLerp.
	FQuat PhysicalFacingWS = FacingValues.BeginingFacingToLerp.GetNormalized();
	const FQuat PhysicalDesiredFacingWS = FacingValues.StaticDesiredFacing.GetNormalized();

	const int32 NumHistorySamples = TrajectoryDataSampling.NumHistorySamples;
	const float SecondsPerPredictionSample = TrajectoryDataSampling.SecondsPerPredictionSample;

	const FQuat ControllerRotationPerStep = FQuat::MakeFromEuler(
		FVector(0.f, 0.f, TrajectoryDataDerived.ControllerYawRate * SecondsPerPredictionSample));

	float AccumulatedSeconds = DeltaTime;

	const int32 LastIndex = Trajectory.Samples.Num() - 1;
	if (NumHistorySamples <= LastIndex)
	{
		for (int32 Index = NumHistorySamples; ; ++Index)
		{
			Trajectory.Samples[Index].Position = CurrentPositionWS;

			FQuat FinalFacingWS = CurrentFacingWS;

			if (FacingValues.bUsePhysicalFacing)
			{
				// Override domyœlnej predykcji facing.
				// Sample startowy bêdzie równy BeginingFacingToLerp.
				FinalFacingWS = PhysicalFacingWS;
			}

			Trajectory.Samples[Index].Facing = FinalFacingWS.GetNormalized();
			Trajectory.Samples[Index].TimeInSeconds = AccumulatedSeconds;

			// Lerping dzia³a tylko dla domyœlnej metody.
			// Jeœli bUsePhysicalFacing == true, to physical facing ma pe³ne pierwszeñstwo.
			if (!FacingValues.bUsePhysicalFacing && FacingValues.bUseLerpingMode)
			{
				const float LerpAlpha = FMath::GetMappedRangeValueClamped(
					FVector2D(
						NumHistorySamples * 1.0f,
						(NumHistorySamples + FacingValues.LerpingSamplesNumber) * 1.0f),
					FVector2D(0.0f, 1.0f),
					Index * 1.0f);

				FQuat OverridedFacing = Trajectory.Samples[Index].Facing;
				if (FacingValues.bUseLerpShortestPath)
				{
					OverridedFacing = UKismetMathLibrary::RLerp(FacingValues.BeginingFacingToLerp.Rotator(), CurrentFacingWS.Rotator(), LerpAlpha, true).Quaternion();
				}
				else
				{
					OverridedFacing = FMath::Lerp<FQuat>(FacingValues.BeginingFacingToLerp, CurrentFacingWS, LerpAlpha).GetNormalized();
				}
				Trajectory.Samples[Index].Facing = OverridedFacing;
			}

			if (Index == LastIndex)
			{
				break;
			}

			CurrentPositionWS += CurrentVelocityWS * SecondsPerPredictionSample;
			AccumulatedSeconds += SecondsPerPredictionSample;

			if (FacingValues.bUsePhysicalFacing)
			{
				PhysicalFacingWS = RotateYawTowardsConstant(
					PhysicalFacingWS,
					PhysicalDesiredFacingWS,
					SecondsPerPredictionSample,
					FacingValues.InRotationRate).GetNormalized();
			}

			if (TrajectoryDataDerived.bStepGroundPrediction)
			{
				CurrentAccelerationWS = RemapVectorMagnitudeWithCurve(
					ControllerRotationPerStep * CurrentAccelerationWS,
					TrajectoryData.bUseAccelerationRemappingCurve,
					TrajectoryData.AccelerationRemappingCurve);

				const FVector NewVelocityWS = TrajectoryData.StepCharacterMovementGroundPrediction(
					SecondsPerPredictionSample,
					CurrentVelocityWS,
					CurrentAccelerationWS,
					TrajectoryDataDerived);

				CurrentVelocityWS = RemapVectorMagnitudeWithCurve(
					NewVelocityWS,
					TrajectoryData.bUseSpeedRemappingCurve,
					TrajectoryData.SpeedRemappingCurve);

				// Account for the controller, e.g. the camera, rotating.
				CurrentFacingWS = ControllerRotationPerStep * CurrentFacingWS;

				if (TrajectoryDataDerived.bOrientRotationToMovement && !CurrentAccelerationWS.IsNearlyZero())
				{
					// Rotate towards acceleration.
					const FVector CurrentAccelerationCS =
						TrajectoryDataDerived.MeshCompRelativeRotation.RotateVector(CurrentAccelerationWS);

					CurrentFacingWS = FMath::QInterpConstantTo(
						CurrentFacingWS,
						CurrentAccelerationCS.ToOrientationQuat(),
						SecondsPerPredictionSample,
						TrajectoryData.RotateTowardsMovementSpeed);
				}
			}
		}
	}
}






void UAGLS_BlueprintFunctionsLibraryP2::CustomHandleTransformTrajectoryStairWorldCollisions(
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
	bool bIgnoreSelf,
	float MaxObstacleHeight,
	FLinearColor TraceColor,
	FLinearColor TraceHitColor,
	float DrawTime,
	float StairFloorTraceUpDistance,
	float StairFloorTraceDownDistance,
	float StairHorizontalTraceBackDistance,
	float StairHorizontalTraceForwardDistance,
	float StairHorizontalTraceZOffset,
	float StairEdgeSnapOffset,
	float StairMaxRiserNormalUpDot,
	float StairMinRiserFacingDot,
	float StairMinForwardSpacing,
	float StairMaxSnapDistanceAlongMove,
	bool bSnapOnlyToOuterStairEdges,
	bool bKeepTrajectoryMonotonic)
{
	OutTrajectory = InTrajectory;

	TArray<FTransformTrajectorySample>& Samples = OutTrajectory.Samples;
	const int32 NumSamples = Samples.Num();

	if (NumSamples <= 0)
	{
		CollisionResult.TimeToLand = 0.0f;
		CollisionResult.LandSpeed = 0.0f;
		return;
	}

	auto ProjectOnPlaneSafe = [](
		const FVector& Vector,
		const FVector& PlaneNormal) -> FVector
		{
			return Vector - PlaneNormal * FVector::DotProduct(Vector, PlaneNormal);
		};

	// ---------------------------------------------------------------------
	// Gravity / up axis setup.
	// ---------------------------------------------------------------------

	FVector GravityDirection = FVector::DownVector;
	float GravityZ = 0.0f;
	float InitialVelocityZ = StartingVelocity.Z;

	if (bApplyGravity && !GravityAccel.IsNearlyZero())
	{
		GravityAccel.ToDirectionAndLength(GravityDirection, GravityZ);
		GravityZ = -GravityZ;

		const FVector VelocityOnGravityAxis = StartingVelocity.ProjectOnTo(GravityDirection);

		InitialVelocityZ =
			VelocityOnGravityAxis.Length() *
			-FMath::Sign(GravityDirection.Dot(VelocityOnGravityAxis));
	}

	const FVector UpDirection = (-GravityDirection).GetSafeNormal();
	const FVector DownDirection = GravityDirection.GetSafeNormal();

	// ---------------------------------------------------------------------
	// Main trajectory direction.
	// For stairs it is better to use one stable horizontal direction for the
	// whole prediction instead of per-sample direction. This reduces zigzagging.
	// ---------------------------------------------------------------------

	FVector MainMoveDirection = ProjectOnPlaneSafe(StartingVelocity, UpDirection);

	if (!MainMoveDirection.Normalize())
	{
		for (int32 Index = 1; Index < NumSamples; ++Index)
		{
			MainMoveDirection =
				ProjectOnPlaneSafe(
					InTrajectory.Samples[Index].Position - InTrajectory.Samples[0].Position,
					UpDirection);

			if (MainMoveDirection.Normalize())
			{
				break;
			}
		}
	}

	if (MainMoveDirection.IsNearlyZero())
	{
		MainMoveDirection = FVector::ForwardVector;
	}

	// This is used to preserve point ordering along trajectory direction.
	float LastAcceptedDistanceAlongMove =
		FVector::DotProduct(Samples[0].Position, MainMoveDirection);

	CollisionResult.TimeToLand = Samples.Last().TimeInSeconds;
	CollisionResult.LandSpeed = InitialVelocityZ + GravityZ * CollisionResult.TimeToLand;

	auto IsOuterStairRiserHit =
		[&](
			const FHitResult& Hit,
			const FVector& MoveDirection) -> bool
		{
			const FVector HitNormal = Hit.Normal.GetSafeNormal();

			const float AbsUpDot =
				FMath::Abs(FVector::DotProduct(HitNormal, UpDirection));

			// A riser wall should be almost vertical, so its normal should be mostly horizontal.
			if (AbsUpDot > StairMaxRiserNormalUpDot)
			{
				return false;
			}

			const FVector PlanarNormal =
				ProjectOnPlaneSafe(HitNormal, UpDirection).GetSafeNormal();

			if (PlanarNormal.IsNearlyZero())
			{
				return false;
			}

			// When going up stairs, the visible / external riser normal usually points
			// opposite to movement direction.
			const float OpposesMovementDot =
				FVector::DotProduct(PlanarNormal, -MoveDirection);

			if (bSnapOnlyToOuterStairEdges && OpposesMovementDot < StairMinRiserFacingDot)
			{
				return false;
			}

			return true;
		};

	auto TryFindOuterStairEdge =
		[&](
			const FVector& FloorImpactPoint,
			const FVector& MoveDirection,
			FVector& OutEdgePoint,
			FVector& OutEdgeNormal) -> bool
		{
			// Important:
			// Horizontal trace is slightly below the vertical floor hit.
			// This avoids tracing exactly on the top face boundary and helps catch the riser.
			const FVector ProbeCenter =
				FloorImpactPoint + UpDirection * StairHorizontalTraceZOffset;

			const FVector TraceStart =
				ProbeCenter - MoveDirection * StairHorizontalTraceBackDistance;

			const FVector TraceEnd =
				ProbeCenter + MoveDirection * StairHorizontalTraceForwardDistance;

			FHitResult RiserHit;
			const bool bHit =
				UKismetSystemLibrary::LineTraceSingle(
					WorldContextObject,
					TraceStart,
					TraceEnd,
					TraceChannel,
					bTraceComplex,
					ActorsToIgnore,
					DrawDebugType,
					RiserHit,
					bIgnoreSelf,
					TraceColor,
					TraceHitColor,
					DrawTime);

			if (!bHit)
			{
				return false;
			}

			if (!IsOuterStairRiserHit(RiserHit, MoveDirection))
			{
				return false;
			}

			OutEdgePoint = RiserHit.ImpactPoint;
			OutEdgeNormal = RiserHit.Normal;
			return true;
		};

	auto ApplyMonotonicEdgeSnap =
		[&](
			const FVector& OriginalPredictedPosition,
			const FVector& EdgePoint,
			const FVector& MoveDirection,
			float& InOutLastAcceptedDistanceAlongMove,
			FVector& InOutCorrectedPosition) -> bool
		{
			const float OriginalAlongMove =
				FVector::DotProduct(OriginalPredictedPosition, MoveDirection);

			const float CurrentAlongMove =
				FVector::DotProduct(InOutCorrectedPosition, MoveDirection);

			const float EdgeAlongMove =
				FVector::DotProduct(EdgePoint, MoveDirection);

			const float DesiredAlongMove =
				EdgeAlongMove - StairEdgeSnapOffset;

			const float SnapDistanceFromOriginal =
				FMath::Abs(DesiredAlongMove - OriginalAlongMove);

			if (SnapDistanceFromOriginal > StairMaxSnapDistanceAlongMove)
			{
				return false;
			}

			float FinalAlongMove = DesiredAlongMove;

			if (bKeepTrajectoryMonotonic)
			{
				const float MinAllowedAlongMove =
					InOutLastAcceptedDistanceAlongMove + StairMinForwardSpacing;

				// Do not allow the next sample to go backwards along the main movement axis.
				// This is the main protection against zigzagging.
				if (FinalAlongMove < MinAllowedAlongMove)
				{
					FinalAlongMove = FMath::Max(OriginalAlongMove, MinAllowedAlongMove);
				}
			}

			const float DeltaAlongMove =
				FinalAlongMove - CurrentAlongMove;

			InOutCorrectedPosition += MoveDirection * DeltaAlongMove;

			InOutLastAcceptedDistanceAlongMove =
				FMath::Max(InOutLastAcceptedDistanceAlongMove, FinalAlongMove);

			return true;
		};

	auto UpdateMonotonicDistanceWithoutSnap =
		[&](
			const FVector& CorrectedPosition,
			const FVector& MoveDirection,
			float& InOutLastAcceptedDistanceAlongMove)
		{
			const float AlongMove =
				FVector::DotProduct(CorrectedPosition, MoveDirection);

			if (bKeepTrajectoryMonotonic)
			{
				InOutLastAcceptedDistanceAlongMove =
					FMath::Max(
						InOutLastAcceptedDistanceAlongMove + StairMinForwardSpacing,
						AlongMove);
			}
			else
			{
				InOutLastAcceptedDistanceAlongMove = AlongMove;
			}
		};

	auto FindFloorAndBuildCorrectedPosition =
		[&](
			int32 SampleIndex,
			float& InOutLastAcceptedDistanceAlongMove,
			FVector& InOutSamplePosition,
			FVector& OutImpactPoint,
			FVector& OutImpactNormal) -> bool
		{
			const FVector OriginalPredictedPosition =
				InTrajectory.Samples.IsValidIndex(SampleIndex)
				? InTrajectory.Samples[SampleIndex].Position
				: InOutSamplePosition;

			const FVector TraceStart =
				InOutSamplePosition + UpDirection * StairFloorTraceUpDistance;

			const FVector TraceEnd =
				InOutSamplePosition - UpDirection * StairFloorTraceDownDistance;

			FHitResult FloorHit;
			const bool bHitFloor =
				UKismetSystemLibrary::LineTraceSingle(
					WorldContextObject,
					TraceStart,
					TraceEnd,
					TraceChannel,
					bTraceComplex,
					ActorsToIgnore,
					DrawDebugType,
					FloorHit,
					bIgnoreSelf,
					TraceColor,
					TraceHitColor,
					DrawTime);

			if (!bHitFloor)
			{
				return false;
			}

			OutImpactPoint = FloorHit.ImpactPoint;
			OutImpactNormal = FloorHit.Normal;

			// First pass: height correction only.
			FVector CorrectedPosition = InOutSamplePosition;

			const float HeightDelta =
				FVector::DotProduct(FloorHit.ImpactPoint - CorrectedPosition, UpDirection);

			CorrectedPosition += UpDirection * (HeightDelta + FloorCollisionsOffset);

			// Second pass: try to normalize to external step edge.
			FVector EdgePoint = FVector::ZeroVector;
			FVector EdgeNormal = FVector::ZeroVector;

			const bool bFoundOuterEdge =
				TryFindOuterStairEdge(
					FloorHit.ImpactPoint,
					MainMoveDirection,
					EdgePoint,
					EdgeNormal);

			if (bFoundOuterEdge)
			{
				const bool bSnapped =
					ApplyMonotonicEdgeSnap(
						OriginalPredictedPosition,
						EdgePoint,
						MainMoveDirection,
						InOutLastAcceptedDistanceAlongMove,
						CorrectedPosition);

				if (!bSnapped)
				{
					UpdateMonotonicDistanceWithoutSnap(
						CorrectedPosition,
						MainMoveDirection,
						InOutLastAcceptedDistanceAlongMove);
				}
			}
			else
			{
				UpdateMonotonicDistanceWithoutSnap(
					CorrectedPosition,
					MainMoveDirection,
					InOutLastAcceptedDistanceAlongMove);
			}

			InOutSamplePosition = CorrectedPosition;
			return true;
		};

	// ---------------------------------------------------------------------
	// Prediction collision pass with gravity.
	// ---------------------------------------------------------------------

	if (!FMath::IsNearlyZero(GravityZ))
	{
		bool bIsFirstFall = true;
		bool bIsLastImpactValid = false;

		FVector LastImpactPoint = FVector::ZeroVector;
		FVector LastImpactNormal = FVector::ZeroVector;

		const FVector Gravity = GravityDirection * -GravityZ;
		float FreeFallAccumulatedSeconds = 0.0f;

		for (int32 SampleIndex = 1; SampleIndex < NumSamples; ++SampleIndex)
		{
			FTransformTrajectorySample& Sample = Samples[SampleIndex];

			if (Sample.TimeInSeconds <= 0.0f)
			{
				continue;
			}

			const int32 PrevSampleIndex = SampleIndex - 1;
			const FTransformTrajectorySample& PrevSample = Samples[PrevSampleIndex];

			const float DeltaSeconds =
				Sample.TimeInSeconds - PrevSample.TimeInSeconds;

			if (DeltaSeconds > 0.0f)
			{
				FreeFallAccumulatedSeconds += DeltaSeconds;
			}

			if (bIsLastImpactValid)
			{
				const FPlane GroundPlane = FPlane(PrevSample.Position, UpDirection);
				Sample.Position = FPlane::PointPlaneProject(Sample.Position, GroundPlane);
			}

			const FVector FreeFallOffset =
				Gravity *
				(0.5f * FreeFallAccumulatedSeconds * FreeFallAccumulatedSeconds);

			Sample.Position += FreeFallOffset;

			if (FloorCollisionsOffset > 0.0f)
			{
				FVector ImpactPoint = FVector::ZeroVector;
				FVector ImpactNormal = FVector::ZeroVector;

				const bool bHitFloor =
					FindFloorAndBuildCorrectedPosition(
						SampleIndex,
						LastAcceptedDistanceAlongMove,
						Sample.Position,
						ImpactPoint,
						ImpactNormal);

				if (bHitFloor)
				{
					LastImpactPoint = ImpactPoint;
					LastImpactNormal = ImpactNormal;
					bIsLastImpactValid = true;

					if (bIsFirstFall)
					{
						const float InitialHeight =
							OutTrajectory.GetSampleAtTime(0.0f).Position.Z;

						const float FinalHeight = Sample.Position.Z;
						const float FallHeight =
							FMath::Abs(FinalHeight - InitialHeight);

						bIsFirstFall = false;

						CollisionResult.TimeToLand =
							(InitialVelocityZ / -GravityZ) +
							(
								FMath::Sqrt(
									FMath::Square(InitialVelocityZ) +
									(2.0f * -GravityZ * FallHeight))
								/ -GravityZ
								);

						CollisionResult.LandSpeed =
							InitialVelocityZ + GravityZ * CollisionResult.TimeToLand;
					}

					FreeFallAccumulatedSeconds = 0.0f;
				}
			}
		}
	}
	// ---------------------------------------------------------------------
	// Prediction collision pass without gravity.
	// ---------------------------------------------------------------------
	else if (FloorCollisionsOffset > 0.0f)
	{
		for (int32 SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
		{
			FTransformTrajectorySample& Sample = Samples[SampleIndex];

			if (Sample.TimeInSeconds <= 0.0f)
			{
				continue;
			}

			FVector ImpactPoint = FVector::ZeroVector;
			FVector ImpactNormal = FVector::ZeroVector;

			FindFloorAndBuildCorrectedPosition(
				SampleIndex,
				LastAcceptedDistanceAlongMove,
				Sample.Position,
				ImpactPoint,
				ImpactNormal);
		}
	}

	CollisionResult.LandSpeed =
		InitialVelocityZ + GravityZ * CollisionResult.TimeToLand;
}


#undef PSTL 