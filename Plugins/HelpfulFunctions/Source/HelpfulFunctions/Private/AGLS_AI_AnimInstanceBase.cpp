
#include "AGLS_AI_AnimInstanceBase.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Vector.h"
#include "AGLS_BlueprintFunctionsLibraryP2.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"

#define KML UKismetMathLibrary
#define POSESEARCH_FUNCTIONS UAGLS_BlueprintFunctionsLibraryP2
#define POSESEARCH_TRAJ UPoseSearchTrajectoryLibrary

float UAGLS_AI_AnimInstanceBase::VectorLenghtXY(FVector In)
{
	const FVector VectorXY = FVector(In.X, In.Y, 0.0);
	return VectorXY.Length();
}


// INITIALIZE ANIM INSTANCE
void UAGLS_AI_AnimInstanceBase::NativeInitializeAnimation()
{
	CharacterC = Cast<ACharacter>(TryGetPawnOwner()); // Get Main Character Reference
	if (CharacterC)
	{
		MovementComp = CharacterC->GetCharacterMovement(); // Try Get Character Movement Component
	}
}

// TICK EVENT
void UAGLS_AI_AnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	CreateOverlayPosesModeState(); // Create Overlay Poses Modes
}


void UAGLS_AI_AnimInstanceBase::CreateOverlayPosesModeState()
{
	if (RotationModeC == CALS_RotationMode::Aiming)
	{
		OverlayPosesType = CALS_OverlayPosesType::Aiming;
	}
	else
	{
		if (OverlayPosesType == CALS_OverlayPosesType::Aiming)
		{
			OverlayPosesType = CALS_OverlayPosesType::Ready;
			timer1 = ReadyStateDuration;
		}

		if (OverlayPosesType == CALS_OverlayPosesType::Ready)
		{
			timer1 = FMath::Clamp<float>(timer1 - dt, 0.0, ReadyStateDuration);

			if (timer1 < 0.2 ||
				(timer1 < (ReadyStateDuration * 0.5) && IsMovingC) ||
				GaitC == CALS_Gait::Sprinting ||
				MovementActionC != CALS_MovementAction::None ||
				MovementStateC == CALS_MovementState::InAir)
			{
				OverlayPosesType = CALS_OverlayPosesType::Relaxed;
				timer1 = 0.0;
			}
		}
	}
}


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ THEAD SAFE LOGIC
#pragma region THREAD SAFE LOGIC

FTransformTrajectory UAGLS_AI_AnimInstanceBase::GenerateTrajectory_Implementation(float HistoryInterval, int HistoryCount, float InPredictionInterval, int PreditionCount, 
	bool UseDirectionStateCorrection, float DiectionCorrectionTreshold, int LerpingFacingSamplesNum)
{
	
	FPoseSearchTrajectoryData ConfigData = TrajectoryConfigMoving;
	if (SpeedC < 10 && !GetIsMovingValue())
	{ ConfigData = TrajectoryConfigIdle; }

	FTransformTrajectory OutTrajectory;
	if (!GetOwningComponent()) return OutTrajectory;

	bool OutPlayingRootAnim = false;
	FRotator OutMontageFacing = FRotator::ZeroRotator;

	POSESEARCH_FUNCTIONS::MakeAccelerationValueForTrajectory(OutPlayingRootAnim, OutMontageFacing, this, dt, AccelerationGenerated, true, 
		RootMotionTrajectorySolverConfig.RootExtractionConstTimeOffset, 
		RootMotionTrajectorySolverConfig.RootExtractionDynamicTimeOffset, 
		RootMotionTrajectorySolverConfig.RootRotationExtractionOffset,
		RootMotionTrajectorySolverConfig.RootRotationExtractionFramesNum
	);
	POSESEARCH_FUNCTIONS::MakeVelocityForTrajectory(this, dt, VelocityGenerated, PrevActorPosition, true, RootMotionTrajectorySolverConfig.VelocityScale);

	FVector InTrajAcceleration = AccelerationGenerated;
	FVector InTrajVelocity = VelocityGenerated;
	FQuat InTrajFacing = FQuat::Identity;
	FPoseSearchTrajectoryFacingProperties InTrajFacingProperties;

	if (GetIsColliding()) { InTrajAcceleration = KML::VLerp(AccelerationGenerated, AccelerationC, 0.4f);}


	Get_OverrideTrajectoryFacing_Implementation(InTrajFacingProperties.bUseStaticFacing, InTrajFacingProperties.StaticDesiredLookingYaw, InTrajFacingProperties.StaticDesiredFacing);


	InTrajFacingProperties.bUseLerpingMode = true;
	InTrajFacingProperties.bUseLerpShortestPath = true;
	InTrajFacingProperties.LerpingSamplesNumber = LerpingFacingSamplesNum;
	if(OutPlayingRootAnim)
	{
		InTrajFacingProperties.bUseStaticFacing = true;
		InTrajFacingProperties.StaticDesiredLookingYaw = KML::NormalizedDeltaRotator(OutMontageFacing, GetOwningComponent()->GetRelativeRotation()).Yaw;
		InTrajFacingProperties.StaticDesiredFacing = OutMontageFacing.Quaternion();
	}

	FRotator ActorRotSafe = FRotator::ZeroRotator;
	if (TryGetPawnOwner()) { ActorRotSafe = TryGetPawnOwner()->GetActorRotation(); }
	InTrajFacingProperties.BeginingFacingToLerp = KML::ComposeRotators(FRotator(0, ActorRotSafe.Yaw, 0), GetOwningComponent()->GetRelativeRotation()).Quaternion();


	POSESEARCH_FUNCTIONS::PoseSearchGenerateTransformTrajectoryExtend
	(
		this, 
		ConfigData, 
		dt, 
		TrajectoryWithoutCollision, 
		PreviousDesiredControllerYaw, 
		InTrajAcceleration, 
		InTrajVelocity, 
		true, 
		InTrajFacingProperties, 
		OutTrajectory, 
		HistoryInterval,
		HistoryCount,
		InPredictionInterval,
		PreditionCount
	);


	if (!TryGetPawnOwner()) return OutTrajectory;

	if (MovementComp->IsFalling() || abs(TryGetPawnOwner()->GetVelocity().Z) > 15 || MovementActionC == CALS_MovementAction::LowMantle || MovementActionC == CALS_MovementAction::HighMantle)
	{
		/*EXPERIMENTAL!: This function takes the generated trajectory and applies gravity over time, and also uses traces to predict collision, mainly used while in the air.
		Known Issues: collision checks are simple and often produce bad trajectories when near walls and ceilings, in the future we want collision checks to be more robust and handle more cases.*/
		TArray<AActor*> ActorsToIgnore;
		POSESEARCH_TRAJ::HandleTransformTrajectoryWorldCollisions(this, this, TrajectoryWithoutCollision, true, 0.01f, Trajectory, TrajectoryCollision,
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, true, 150, FColor::Black, FColor::Blue, 0.1f);

	}
	else
	{
		Trajectory = TrajectoryWithoutCollision;
	}

	/*Get predicted velocities at different points in time. Also get the trajectory Turn Angle, which is the angle between the current and future velocity direction, useful for detecting pivots.*/
	POSESEARCH_TRAJ::GetTransformTrajectoryVelocity(Trajectory, -0.15f, -0.05f, Trj_PastVelocity, true);
	POSESEARCH_TRAJ::GetTransformTrajectoryVelocity(Trajectory, 0.1f, 0.2f, Trj_NearFutureVelocity, false);
	Trj_PreviousFutureVelocity = Trj_FutureVelocity;
	POSESEARCH_TRAJ::GetTransformTrajectoryVelocity(Trajectory, 0.4f, 0.5f, Trj_FutureVelocity, false);
	Trj_TurnAngle = Get_TrajectoryTurnAngle();

	/*Get the future facing rotation, as well as the Future Facing Delta, which is the total amount of rotation delta between the root's current rotation and the trajectory's future facing rotation.*/
	FTransformTrajectorySample SampleFuture;
	POSESEARCH_TRAJ::GetTransformTrajectorySampleAtTime(Trajectory, 1.5f, SampleFuture, false); Trj_FutureFacing = SampleFuture.Facing.Rotator();
	Trj_FutureFacingDelta_LastFrame = Trj_FutureFacingDelta;
	const TArray<float> TimeSamples = { 0.0, 0.5, 0.75f, 1.2f };
	const FRotator RootRotation = GetRootTransformFromNodeRef().Rotator();
	Trj_FutureFacingDelta = Get_TrajectoryFacingDelta(TimeSamples, RootRotation);

	/*In certain strafe styles, the pawn can perform large spins without changing movement directions. In order to properly trigger 360 spins when this occurs, we force a direction change to happen ▓▓▓▓▓▓▓▓▓▓▓▓
	by setting the Movement Direction to B. It will still be set back to F later in this update, but the anim system will still trigger a reselection as if the direction changed normally.*/ //▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
	//if (abs(Trj_FutureFacingDelta - Trj_FutureFacingDelta_LastFrame) > 200 && RotationModeC != CALS_RotationMode::VelocityDirection) //▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
	//{
	//	MovementDirection = AGLS_MovementDirectionState::B;
	//	MovementDirection_Recent = AGLS_MovementDirectionState::B;
	//}																																													//OVERRIDED for AGLS v1.9.2


	/*Get the past and current angular velocity, which is useful in determining whether or not the character is turning continually in a circle. We call this "circling", and we use this value to 
	modify animation selection, such as blocking pivot animations.*/
	POSESEARCH_TRAJ::GetTransformTrajectoryAngularVelocity(Trajectory, -0.4f, -0.3f, Trj_PastAngularVelocity, true);
	POSESEARCH_TRAJ::GetTransformTrajectoryAngularVelocity(Trajectory, 0.0, 0.1f, Trj_CurrentAngularVelocity, true);

	FVector2D TresholdPast; FVector2D TresholdCurrent; float CircingAngle = 0.0;
	GetIsCirclingTresholds(TresholdPast, TresholdCurrent, CircingAngle);

	bool NewCircingValue = (Trj_PastAngularVelocity.Z < TresholdPast.X && Trj_CurrentAngularVelocity.Z < TresholdCurrent.X) ||
		(Trj_PastAngularVelocity.Z > TresholdPast.Y && Trj_CurrentAngularVelocity.Z > TresholdCurrent.Y);
	NewCircingValue = NewCircingValue && abs(Trj_TurnAngle) > CircingAngle;

	//Trj_IsCircling = NewCircingValue; Edited in AGLS v1.9.2
	if (NewCircingValue)
	{ Trj_CirclingTime += dt; }
	else
	{ Trj_CirclingTime = KML::FClamp(Trj_CirclingTime - dt, 0.0, 0.4f); }

	Trj_IsCircling = Trj_CirclingTime > 0.1f;

	//IGNORING
	Trj_IsPivotingInCircleShape = false;


	//MOVEMENT DIRECTION STATE CORRECTION
	if (UseDirectionStateCorrection)
	{
		const float RotDelta = KML::NormalizedDeltaRotator(CharacterTransformC.Rotator(), RootTransformC.Rotator()).Yaw;
		if (Trj_IsCircling && abs(RotDelta) > DiectionCorrectionTreshold && Trj_FutureFacingDelta > 45)
		{
			if (MovementDirection == AGLS_MovementDirectionState::B)
			{
				MovementDirection = AGLS_MovementDirectionState::F;
				MovementDirection_Recent = AGLS_MovementDirectionState::F;
			}
			else
			{
				MovementDirection = AGLS_MovementDirectionState::B;
				MovementDirection_Recent = AGLS_MovementDirectionState::B;
			}
		}
	}

	return OutTrajectory;
}



void UAGLS_AI_AnimInstanceBase::Get_OverrideTrajectoryFacing_Implementation(bool& ShoundOverride, float& ReturnLookingYaw, FQuat& ReturnFacing)
{
	ShoundOverride = false;
	ReturnLookingYaw = 0.0;
	ReturnFacing = FQuat::Identity;
	return;
}



void UAGLS_AI_AnimInstanceBase::UpdateEssentialValues()
{
	if (!TryGetPawnOwner()) return;
	if (!MovementComp) return;
	//Cache the actor (capsule) transform. This is mainly for convenience and to ensure thread safety with Property Access.
	CharacterTransformLastFrame = CharacterTransformC;
	CharacterTransformC = TryGetPawnOwner()->GetTransform();

	//Caches the root bone transform from the offset root bone node if possible. Storing the offset this way instead of using the Get Socket Transform function 
	// ensures a more accurate value. Since Skeletal Meshes are rotated -90 degrees by default in editor, an offset is added to make angle comparisons against other transforms easier.
	if (OffsetRootBoneEnabledC)
	{
		const FTransform NodeRootTransform = GetRootTransformFromNodeRef();

		RootTransformC = FTransform(
			FRotator(NodeRootTransform.Rotator().Pitch, NodeRootTransform.Rotator().Yaw + 90, NodeRootTransform.Rotator().Roll), 
			NodeRootTransform.GetLocation(), 
			NodeRootTransform.GetScale3D()
		);
	}
	else
	{
		RootTransformC = CharacterTransformC;
	}

	//Caches important information about the Character’s Acceleration, which is the input acceleration applied by the movement component, not the physical acceleration. Although not 
	// all of these values are currently used in the graph, they are important to have on hand for additional features.
	AccelerationLastFrame = MovementAcceleration;
	MovementAcceleration = MovementComp->GetCurrentAcceleration();
	if (bHasAnyRootMotion) { MovementAcceleration = AccelerationGenerated; }

	const float RotationAmout = KML::SafeDivide(MovementAcceleration.Length(), MovementComp->GetMaxAcceleration());

	//Caches important information about the Character’s Velocity. Although not all of these values are currently used in the graph, they are important to have on hand for additional features.
	VelocityLastFrameC = VelocityC;
	VelocityC = MovementComp->Velocity;
	if (bHasAnyRootMotion) { VelocityC = VelocityGenerated; }
	SpeedC = VelocityC.Size2D();
	const bool HasVelocity = SpeedC > 5.0;
	if (HasVelocity) LastNonZeroVelocityC = VelocityC;

	AccelerationC = (VelocityC - VelocityLastFrameC) / KML::FMax(this->GetDeltaSeconds(), 0.0005f);
	RelativeAccelerationAmout = KML::Quat_UnrotateVector(RootTransformC.GetRotation(), AccelerationC);

	//Caches the tags from the currently selected Motion matching database. This allows us to make additional selection choices based on the current database.
	if (CurrentSelectedDatabase) { CurrentDatabaseTags = CurrentSelectedDatabase->Tags; }

	//Create is Moving value, but using character movement component values, not trajectory
	PrevIsMovingC = IsMovingC;
	IsMovingC = !MovementComp->GetCurrentAcceleration().Equals(FVector::ZeroVector, 0.1f) && SpeedC > 2;

	const bool CalculateAimingValue = true;
	if (CalculateAimingValue)
	{
		AimingRotationC = CharacterC->GetControlRotation();
		AimYawRateC = KML::SafeDivide(KML::NormalizedDeltaRotator(AimingRotationC, PrevAimingRotationC).Yaw, this->GetDeltaSeconds());
		PrevAimingRotationC = AimingRotationC;
	}

	//Calculate Root Rotation Speed
	RootYawChangeSpeedC = UKismetMathLibrary::FInterpTo(RootYawChangeSpeedC, UKismetMathLibrary::NormalizedDeltaRotator(RootTransformC.Rotator(), PrevRootTransform.Rotator()).Yaw / dt, dt, 6.0);

}


// RootTransform is not be able to set by using C++, becouse we need reference to Anim Graph Node 'Root Bone Offset'
FTransform UAGLS_AI_AnimInstanceBase::GetRootTransformFromNodeRef_Implementation()
{
	if (TryGetPawnOwner())
	{
		return FTransform(TryGetPawnOwner()->GetActorRotation() - FRotator(0, 90, 0), TryGetPawnOwner()->GetActorLocation());
	}
	return FTransform::Identity;
}


void UAGLS_AI_AnimInstanceBase::UpdateRotationValues_Implementation(float SmoothingTime, int NumOfSpineBones, bool UseSmoothOnSpineRotation, float SpineRotSmoothMinSpeed, bool UseRootBoneAsRotationRef)
{
	const FRotator RotationDeltaBase = FRotator(0, KML::SelectFloat(RootTransformC.Rotator().Yaw, CharacterTransformC.Rotator().Yaw, UseRootBoneAsRotationRef) + GetCurveValue("AimingRotationOffset"), 0);

	//Interp the Aiming Rotation value to achieve smooth aiming rotation changes. Interpolating the rotation before calculating the angle ensures the value is not affected by 
	// changes in actor rotation, allowing slow aiming rotation changes with fast actor rotation changes.
	SmoothedAimingRotationC = KML::RInterpTo(SmoothedAimingRotationC, AimingRotationC, this->GetDeltaSeconds(), SmoothingTime);

	//Calculate the Aiming angle and Smoothed Aiming Angle by getting the delta between the aiming rotation and the actor rotation.
	const FRotator DeltaRot = KML::NormalizedDeltaRotator(AimingRotationC, RotationDeltaBase);
	const FRotator DeltaRotSmooth = KML::NormalizedDeltaRotator(SmoothedAimingRotationC, RotationDeltaBase);
	AimingAngleC = FVector2D(DeltaRot.Yaw, DeltaRot.Pitch);
	SmoothedAimingAngleC = FVector2D(DeltaRotSmooth.Yaw, DeltaRotSmooth.Pitch);

	//Clamp the Aiming Pitch Angle to a range of 1 to 0 for use in the vertical aim sweeps.
	const float DetectedEnemyTimeValue = DetectedEnemyTime;
	AimSweepTimeC = KML::MapRangeClamped(AimingAngleC.Y, -90, 90, 1.0, 0.0);
	//Use the Aiming Yaw Angle divided by the number of spine+pelvis bones to get the amount of spine rotation needed to remain facing the camera direction.
	const float R = KML::ClampAngle(KML::Lerp(SmoothedAimingAngleC.X, AimingAngleC.X, DetectedEnemyTimeValue), -89, 89);
	
	if (UseSmoothOnSpineRotation)
	{
		const float SpineRotDelta = abs((R / NumOfSpineBones) - SpineRotationC.Yaw);
		SpineRotationC = KML::RInterpTo_Constant(SpineRotationC, FRotator(0, R / NumOfSpineBones, 0), GetDeltaSeconds(), KML::MapRangeClamped(SpineRotDelta, 30, 80, 300, SpineRotSmoothMinSpeed));
	}
	else
	{
		SpineRotationC.Yaw = R / NumOfSpineBones;
	}

}


void UAGLS_AI_AnimInstanceBase::UpdateLayeringValues(bool MakeBasePoseAlphaFromState, float BasePoseInterpSpeed)
{
	if (MakeBasePoseAlphaFromState)
	{
		float StanceAsFloat = 0.0; if (StanceC == CALS_Stance::Standing) StanceAsFloat = 1.0;
		const float OutInterpTime = KML::FClamp(BasePoseInterpSpeed, 0.01, 1000);

		BasePoseN = KML::FClamp(KML::FInterpTo_Constant(BasePoseN, StanceAsFloat, this->GetDeltaSeconds(), OutInterpTime), 0.0, 1.0);
		BasePoseCLF = 1 - BasePoseN;
	}
	else
	{
		BasePoseN = GetCurveValue("BasePose_N");
		BasePoseCLF = GetCurveValue("BasePose_CLF");
	}


	Hand_R = GetCurveValue(TEXT("Layering_Hand_R"));
	Hand_L = GetCurveValue(TEXT("Layering_Hand_L"));

	ArmR_LS = GetCurveValue(TEXT("Layering_Arm_R_LS"));
	ArmL_LS = GetCurveValue(TEXT("Layering_Arm_L_LS"));

	ArmR_MS = (1 - UKismetMathLibrary::FFloor(ArmR_LS)) * 1.0;
	ArmL_MS = (1 - UKismetMathLibrary::FFloor(ArmL_LS)) * 1.0;
}


void UAGLS_AI_AnimInstanceBase::UpdateMainStatesValues_Implementation(float RecentTimeLimit)
{
	//Save Prev Values and new collected from Character
	PrevMovementStateC = MovementStateC;
	MovementStateC = CollectedCharacterParams.MovementState;

	PrevMovementActionC = MovementActionC;
	MovementActionC = CollectedCharacterParams.MovementAction;

	PrevRotationMode = RotationModeC;
	RotationModeC = CollectedCharacterParams.RotationMode;

	PrevGait = GaitC;
	GaitC = CollectedCharacterParams.Gait;

	PrevStance = StanceC;
	StanceC = CollectedCharacterParams.Stance;

	PrevLocomotionModeIndex = LocomotionModeIndex;
	LocomotionModeIndex = CollectedCharacterParams.LocomotionModeIndex;

	LOD_State = CollectedCharacterParams.LOD_State;

	bPrevCapsuleCollidingC = CapsuleCollidingC;
	CapsuleCollidingC = CollectedCharacterParams.IsColliding;

	bPrevHasAnyRootMotion = bHasAnyRootMotion;
	bHasAnyRootMotion = CollectedCharacterParams.HasRootMotion;


	//Caches the Movement Direction and saves the last frame's value. The Movement Direction Enum describes which kind of directional animation to play.
	if (MovementParamsControlComponent)
	{
		const AGLS_MovementDirectionState CollectedDirection = MovementParamsControlComponent->CurrentMovementDirection;
		UpdateStateValuesMacro
		(
			CollectedDirection, 
			MovementDirection, 
			MovementDirection_LastFrame, 
			MovementDirection_Recent, 
			MovementDirection_Time, 
			MovementDirection_LastStateTime, 
			RecentTimeLimit, 
			this->GetDeltaSeconds()
		);
	}
	

	//Enable Stance Transition
	if (StanceC != PrevStance) StanceTransitionC = true;
	if (MovementStateC != PrevMovementStateC)
	{
		if (MovementStateC == CALS_MovementState::Crawl || PrevMovementStateC == CALS_MovementState::Crawl)
		{
			StanceTransitionC = true;
		}
	}

	//Convert bIsMoving value as Enum state. Before value update save current value as prev. This value is required for Motion Matching databases search interrupt


	//Set LastMovement State
	if (PrevMovementStateC != MovementStateC) LastMovementStateC = PrevMovementStateC;
}


void UAGLS_AI_AnimInstanceBase::UpdateCoverValues_Implementation()
{
	if (CharacterC && MovementComp)
	{
		bool PrevCoverDirection = false;

		const FVector AccelerationXY = FVector(MovementComp->GetCurrentAcceleration().X, MovementComp->GetCurrentAcceleration().Y, 0.0);
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
				}
			}
		}
	}
	if (CoverDirectionChangedC)
	{
		timer2 = FMath::Clamp<float>(timer2 - dt, 0.0, 0.25);
		if (timer2 <= 0.02)
		{
			CoverDirectionChangedC = false;
			timer2 = -1.0;
		}
	}
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ FOOTS IK Core Functions
#pragma region FOOTS INVERSE KINEMATIC FUNCTIONS

void UAGLS_AI_AnimInstanceBase::UpdateFootsIK_Implementation(ECollisionChannel TraceChannel, bool UseFootsLock, float TraceAboveFoot, float TraceBelowFoot, float FootHeight, int ActiveOnLOD, int TraceDebugIndex, float DebugTime)
{
	const FVector ZeroV = FVector(0, 0, 0);
	const FRotator ZeroR = FRotator(0, 0, 0);

	FVector FootOffset_L_Target = FVector(0, 0, 0);
	FVector FootOffset_R_Target = FVector(0, 0, 0);

	int LOD_Index = static_cast<int>(LOD_State);
	if (LOD_Index < ActiveOnLOD)
	{
		if (MovementStateC == CALS_MovementState::InAir)
		{
			SetPelvisIK_Offset(ZeroV, ZeroV);
			ResetIK_Offsets(15.0);
		}
		else if (MovementStateC != CALS_MovementState::Ragdoll)
		{
			SetFootOffsets(
				FootIK_L_CurveName,
				TEXT("ik_foot_l"),
				TEXT("root"),
				FootOffset_L_Target,
				FootOffset_L_LocC,
				FootOffset_L_RotC,
				TraceChannel,
				TraceAboveFoot,
				TraceBelowFoot,
				FootHeight,
				TraceDebugIndex,
				DebugTime
			);

			SetFootOffsets(
				FootIK_R_CurveName,
				TEXT("ik_foot_r"),
				TEXT("root"),
				FootOffset_R_Target,
				FootOffset_R_LocC,
				FootOffset_R_RotC,
				TraceChannel,
				TraceAboveFoot,
				TraceBelowFoot,
				FootHeight,
				TraceDebugIndex,
				DebugTime
			);

			SetPelvisIK_Offset(FootOffset_L_Target, FootOffset_R_Target);
		}
	}
	else
	{
		//Clear All IK Offsets
		FootOffset_L_LocC = ZeroV;
		FootLock_L_Location = ZeroV;
		FootOffset_L_RotC = ZeroR;
		//Clear All IK Offsets
		FootOffset_R_LocC = ZeroV;
		FootLock_R_Location = ZeroV;
		FootOffset_R_RotC = ZeroR;
	}
}

bool UAGLS_AI_AnimInstanceBase::SetFootOffsets(FName Enable_FootIK_Curve, FName IKFootBone, FName RootBone,
	UPARAM(ref)FVector& CurrentLocationTarget, UPARAM(ref)FVector& CurrentLocationOffset, UPARAM(ref)FRotator& CurrentRotationOffset,
	ECollisionChannel TraceChannel, float TraceAboveFoot, float TraceBelowFoot, float FootHeight, int TraceDebugIndex, float DebugTime)
{
	//Only update Foot IK offset values if the Foot IK curve has a weight. If it equals 0, clear the offset values.
	float FootCurveValue = GetCurveValue(Enable_FootIK_Curve);
	if (bFootEnableCurvesAsDisableMode)
	{ FootCurveValue = KML::FClamp(1 - FootCurveValue, 0.0, 1.0); }

	if (FootCurveValue > 0.0 && IsValid(MovementComp))
	{
		FVector IK_Foot_FloorLocation = GetOwningComponent()->GetSocketLocation(IKFootBone);
		IK_Foot_FloorLocation.Z = GetOwningComponent()->GetSocketLocation(RootBone).Z;
		FRotator TargetRotationOffset = FRotator(0, 0, 0);

		ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(TraceChannel);
		TArray<AActor*> ActorsToIgnore = {};
		EDrawDebugTrace::Type DebugTrace = EDrawDebugTrace::None;
		if (TraceDebugIndex == 1) { DebugTrace = EDrawDebugTrace::ForOneFrame; }
		if (TraceDebugIndex == 2) { DebugTrace = EDrawDebugTrace::ForDuration; }
		FHitResult TraceResult;

		//Step 1: Trace downward from the foot location to find the geometry. If the surface is walkable, save the Impact Location and Normal.
		bool TraceValid = UKismetSystemLibrary::SphereTraceSingle(
			TryGetPawnOwner(),
			IK_Foot_FloorLocation + FVector(0, 0, TraceAboveFoot),
			IK_Foot_FloorLocation - FVector(0, 0, TraceBelowFoot),
			FootTraceRadius,
			TraceTypeQuery,
			false,
			ActorsToIgnore,
			DebugTrace,
			TraceResult,
			true,
			FColor::Blue,
			FColor::Green,
			DebugTime
		);

		if (TraceValid && MovementComp->IsWalkable(TraceResult))
		{
			FVector ImpactPoint = TraceResult.ImpactPoint;
			FVector ImpactNormal = TraceResult.ImpactNormal;

			//Step 1.1: Find the difference in location from the Impact point and the expected (flat) floor location. 
			// These values are offset by the nomrmal multiplied by the foot height to get better behavior on angled surfaces.
			CurrentLocationTarget = (ImpactPoint + (ImpactNormal * FootHeight)) - (IK_Foot_FloorLocation + (FVector(0, 0, 1) * FootHeight));

			//Step 1.2: Calculate the Rotation offset by getting the Atan2 of the Impact Normal.
			TargetRotationOffset = FRotator(KML::DegAtan2(ImpactNormal.X, ImpactNormal.Z) * -1.0, 0.0, KML::DegAtan2(ImpactNormal.Y, ImpactNormal.Z));
		}

		//Step 2: Interp the Current Location Offset to the new target value. Interpolate at different speeds based on whether the new target is above or below the current one.
		if (CurrentLocationOffset.Z > CurrentLocationTarget.Z)
		{
			CurrentLocationOffset = KML::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, dt, 30.0);
		}
		else
		{
			CurrentLocationOffset = KML::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, dt, 15.0);
		}
		//Step 3: Interp the Current Rotation Offset to the new target value.
		CurrentRotationOffset = KML::RInterpTo(CurrentRotationOffset, TargetRotationOffset, dt, 30.0);
		return true;
	}
	else
	{
		CurrentLocationOffset = FVector(0, 0, 0);
		CurrentRotationOffset = FRotator(0, 0, 0);
		return false;
	}
}

void UAGLS_AI_AnimInstanceBase::SetPelvisIK_Offset(FVector FootOffset_L_Target, FVector FootOffset_R_Target)
{
	const FVector ZeroV = FVector(0, 0, 0);

	float FootCurveValue_L = GetCurveValue(FootIK_L_CurveName);
	if (bFootEnableCurvesAsDisableMode) FootCurveValue_L = KML::FClamp(1 - FootCurveValue_L, 0.0, 1.0);

	float FootCurveValue_R = GetCurveValue(FootIK_L_CurveName);
	if (bFootEnableCurvesAsDisableMode) FootCurveValue_R = KML::FClamp(1 - FootCurveValue_R, 0.0, 1.0);

	//Calculate the Pelvis Alpha by finding the average Foot IK weight. If the alpha is 0, clear the offset.
	PelvisOffsetAlphaC = (FootCurveValue_L + FootCurveValue_R) / 2.0;
	FVector PelvisTarget = FVector(0, 0, 0);

	if (PelvisOffsetAlphaC > 0.02)
	{
		//Step 1: Set the new Pelvis Target to be the lowest Foot Offset
		if (FootOffset_L_Target.Z < FootOffset_R_Target.Z)
		{
			PelvisTarget = FootOffset_L_Target;
		}
		else
		{
			PelvisTarget = FootOffset_R_Target;
		}
		//Step 2: Interp the Current Pelvis Offset to the new target value. 
		// Interpolate at different speeds based on whether the new target is above or below the current one.
		if (PelvisTarget.Z > PelvisOffsetC.Z)
		{
			PelvisOffsetC = KML::VInterpTo(PelvisOffsetC, PelvisTarget, dt, 10.0);
		}
		else
		{
			PelvisOffsetC = KML::VInterpTo(PelvisOffsetC, PelvisTarget, dt, 15.0);
		}
	}
	else
	{
		PelvisOffsetC = ZeroV;
	}
}

//Interp Foot IK offsets back to 0
void UAGLS_AI_AnimInstanceBase::ResetIK_Offsets(float InterpSpeed)
{
	FootOffset_L_LocC = KML::VInterpTo(FootOffset_L_LocC, FVector(0, 0, 0), dt, InterpSpeed);
	FootOffset_R_LocC = KML::VInterpTo(FootOffset_R_LocC, FVector(0, 0, 0), dt, InterpSpeed);

	FootOffset_L_RotC = KML::RInterpTo(FootOffset_L_RotC, FRotator(0, 0, 0), dt, InterpSpeed);
	FootOffset_R_RotC = KML::RInterpTo(FootOffset_R_RotC, FRotator(0, 0, 0), dt, InterpSpeed);
}


void UAGLS_AI_AnimInstanceBase::SetFootLocking_Implementation(FName FootEnableCurve, FName FootLockCurve, FName IKFootBone,
	UPARAM(ref) float& CurrentFootLockAlpha, UPARAM(ref)FVector& CurrentFootLockLocation, UPARAM(ref)FRotator& CurrentFOotLockRotation)
{
}


void UAGLS_AI_AnimInstanceBase::SetFootLockOffset_Implementation(float Alpha, UPARAM(ref)FVector& LocalLot, UPARAM(ref)FRotator& LocalRot)
{
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
#pragma region NOT THREAD SAFE SECTION
//An event that initiates an attempt to load PoseSearchDatabases, which are not yet available.This option is highly EXPERIMENTAL.
void UAGLS_AI_AnimInstanceBase::TryAsyncLoadDatabases_Implementation(){}


void UAGLS_AI_AnimInstanceBase::OnStanceTransitionEnded_Implementation()
{
	StanceTransitionC = false;
}


void UAGLS_AI_AnimInstanceBase::CollectAndUpdateDataFromCharacter_Implementation()
{
	if (!CharacterC) return;

	const CALS_OverlayState PrevOverlayState = CollectedCharacterParams.OverlayState;

	//⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻ UAGLS_AI_CharacterInterface ⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻
	if (CharacterC->GetClass()->ImplementsInterface(UAGLS_AI_CharacterInterface::StaticClass()))
	{
		IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_CurrentStates
		(
			CharacterC,
			IgnoreOut<TEnumAsByte<EMovementMode>>(),
			CollectedCharacterParams.MovementState,
			IgnoreOut<CALS_MovementState>(),
			CollectedCharacterParams.MovementAction,
			CollectedCharacterParams.RotationMode,
			CollectedCharacterParams.Gait,
			CollectedCharacterParams.Stance,
			CollectedCharacterParams.OverlayState,
			IgnoreOut<CALS_GroundedMoveMode>()
		);

		IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_LocomotionModeIndex
		(
			CharacterC, 
			CollectedCharacterParams.LocomotionModeIndex, 
			IgnoreOut<uint8>(), 
			CollectedCharacterParams.LocomotionModeName
		);

		IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_LOD_State(CharacterC, CollectedCharacterParams.LOD_State);

		IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_EssentialValues
		(
			CharacterC,
			IgnoreOut<FVector>(),
			IgnoreOut<FVector>(),
			IgnoreOut<FVector>(),
			IgnoreOut<bool>(),
			IgnoreOut<bool>(),
			IgnoreOut<float>(),
			AimingRotationC, //<------------ Update only this value
			IgnoreOut<float>()
		);

		IAGLS_AI_CharacterInterface::Execute_BPI_AI_Get_MainTagsContainerData(CharacterC, OwnerTagsContainer);
	}

	//⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻ UALS_HumanAI_InterfaceCpp ⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻
	if (CharacterC->GetClass()->ImplementsInterface(UALS_HumanAI_InterfaceCpp::StaticClass()))
	{
		float EnemyTimeFromInterface = 0.0;
		IALS_HumanAI_InterfaceCpp::Execute_HAI_GetControllerSmallValues(CharacterC, IgnoreOut<bool>(), EnemyTimeFromInterface, IgnoreOut<ACharacter*>());
		DetectedEnemyTime = KML::MapRangeClamped(EnemyTimeFromInterface, 0.1, 0.9, 0.0, 1.0);
	}

	//⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻ UAGLS_AI_HumanCharInterface ⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻⿻
	if (CharacterC->GetClass()->ImplementsInterface(UAGLS_AI_HumanCharInterface::StaticClass()))
	{
		IAGLS_AI_HumanCharInterface::Execute_BPI_HCAI_Get_StartedCoverMode(CharacterC, IsCoveringC);
	}
	else
	{
		IsCoveringC = CollectedCharacterParams.LocomotionModeIndex == 1;
	}

	CollectedCharacterParams.HasRootMotion = CharacterC->HasAnyRootMotion() && SpeedC > 10.0;


	//Warość Alpha przeznaczona dla Overlay State wpływająca na to czy charakter powinien się lekko pochlać/ przykucać
	BendDownAlphaC = KML::FInterpTo(BendDownAlphaC, DesiredBendDownAlpha, this->GetDeltaSeconds(), 4.0);

	//Odmierzanie czasu od ostatniej zmiany OverlayState
	OverlayStateC = CollectedCharacterParams.OverlayState; //<----------------------- IMPORTANT
	if (PrevOverlayState != CollectedCharacterParams.OverlayState)
	{
		OverlayStateElapsedTime = 0.0;
	}
	else
	{
		const float NexTime = OverlayStateElapsedTime + this->GetDeltaSeconds();
		if (NexTime < 500)
		{
			OverlayStateElapsedTime = NexTime;
		}
	}

	//Pobranie z MovementParamsControlComponent wartość potrzebnych dla Chooser
	if (MovementParamsControlComponent)
	{
		MovementParamsControlComponent->GetDesiredMovementsTypeStates(WalkingDatabasesType, RunningDatabasesType, SprintingDatabasesType);
	}

	//⚠️ Check Required any asyncloading PoseSearch Databases
	if (RequiredToLoadDatabases.Num() > 0 && !CorrentyAnyDatabaseIsLoading)
	{
		TryAsyncLoadDatabases();
	}

	//Activate timer when stance Transition is Enabled
	if (StanceTransitionC)
	{
		if (GetWorld()->GetTimerManager().GetTimerElapsed(TimerHandle_StanceTransition) <= -1.0)
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_StanceTransition,
				this,
				&UAGLS_AI_AnimInstanceBase::OnStanceTransitionEnded_Implementation,
				0.2f,
				false);
		}
	}

}


void UAGLS_AI_AnimInstanceBase::MakeIsCollideValue(TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects, int DebugIndex, float DebugTime, bool IgnoreCharacters)
{
	if (IsValid(MovementComp) == false) { return; }

	const FVector MainLoc = TryGetPawnOwner()->GetActorLocation();

	const FVector VelocityDeltaForce = VelocityC * ((dt * 2) / TimeDilatationC);
	const FVector AccelerationDeltaForce = MovementComp->GetCurrentAcceleration() * ((dt * dt * 2) / TimeDilatationC);
	const FVector CapsuleSizeOffset = FVector(0, 0, CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
	const FVector MeshPosition = MainLoc - (CharacterC->GetActorUpVector() * CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	TArray<AActor*> ActorsToIgnore = {};
	ActorsToIgnore.Add(TryGetPawnOwner());
	EDrawDebugTrace::Type DebugTrace = EDrawDebugTrace::None;
	if (DebugIndex == 1) { DebugTrace = EDrawDebugTrace::ForOneFrame; }
	if (DebugIndex == 2) { DebugTrace = EDrawDebugTrace::ForDuration; }
	FHitResult TraceResult;

	const bool HitWall = UKismetSystemLibrary::SphereTraceSingleForObjects(CharacterC, MainLoc + VelocityDeltaForce + AccelerationDeltaForce + CapsuleSizeOffset, MainLoc + VelocityDeltaForce + AccelerationDeltaForce -
		CapsuleSizeOffset, CharacterC->GetCapsuleComponent()->GetScaledCapsuleRadius(), TraceObjects, false, ActorsToIgnore, DebugTrace, TraceResult, true, FColor::Cyan, FColor::Red, DebugTime);

	if (HitWall == true)
	{
		if (DebugIndex > 0)
		{
			int32 MesKey = KML::Round((TryGetPawnOwner()->GetActorLocation().X + TryGetPawnOwner()->GetActorLocation().Y) * 0.1);
			GEngine->AddOnScreenDebugMessage(MesKey, DebugTime, FColor::Red, TraceResult.GetComponent()->GetName());
		}

		if (IgnoreCharacters == true)
		{
			ACharacter* HitChar = Cast<ACharacter>(TraceResult.GetActor());
			if (HitChar) { CapsuleCollidingC = false; return; }
		}
		if (TraceResult.ImpactPoint.Z - MeshPosition.Z > MovementComp->MaxStepHeight && (MovementComp->MaxWalkSpeed - SpeedC) > (MovementComp->MaxWalkSpeed * 0.3))
		{
			CapsuleCollidingC = true; return;
		}
	}

	CapsuleCollidingC = false;
	return;
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ Trajectory functions
#pragma region TRAJECTORY MOVEMENT ANALYZE
bool UAGLS_AI_AnimInstanceBase::GetIsMovingValue_Implementation() // <--------------------------------------------------- AGLS v1.7
{
	return !Trj_FutureVelocity.Equals(FVector::ZeroVector, 10) && !MovementAcceleration.Equals(FVector::ZeroVector, 2.0);
}


bool UAGLS_AI_AnimInstanceBase::IsStarting()
{
	if (CurrentDatabaseTags.Contains("Pivots") || !GetIsMovingValue()) return false;

	if (IsCoveringC)
	{
		return Trj_FutureVelocity.Size2D() >= (VelocityC.Size2D() + 8.0) && Trj_PastVelocity.Size2D() < 30;
	}
	else if(StanceC == CALS_Stance::Standing)
	{
		float VelocityBias = 100;
		float PastVelocityMax = 100;
		switch (GaitC)
		{
		case CALS_Gait::Walking:
			VelocityBias = IsStartingVelocityBiasPerGait.X;
			PastVelocityMax = IsStartingPastVelocityMax.X;
			break;
		case CALS_Gait::Running:
			VelocityBias = IsStartingVelocityBiasPerGait.Y;
			PastVelocityMax = IsStartingPastVelocityMax.Y;
			break;
		case CALS_Gait::Sprinting:
			VelocityBias = IsStartingVelocityBiasPerGait.Z;
			PastVelocityMax = IsStartingPastVelocityMax.Z;
			break;
		}
		return Trj_FutureVelocity.Size2D() >= (VelocityC.Size2D() + VelocityBias) && Trj_PastVelocity.Size2D() < PastVelocityMax;
	}
	else
	{
		return Trj_FutureVelocity.Size2D() >= (VelocityC.Size2D() + 20) && Trj_PastVelocity.Size2D() < 100;
	}
}


bool UAGLS_AI_AnimInstanceBase::IsStopping()
{
	return !IsMovingC && FutureVelocityC.Size2D() < 10 && VelocityC.Size2D() > 10 && !CurrentDatabaseTags.Contains(TEXT("Pivots"));
}


bool UAGLS_AI_AnimInstanceBase::IsPivoting()
{
	if (bHasAnyRootMotion) return abs(Trj_TurnAngle) > 20.0 && !Trj_IsCircling;

	if (StanceC == CALS_Stance::Standing)
	{
		float Treshold = IsPivotingDeltaTrigger.X;
		if (GaitC == CALS_Gait::Running)
		{ Treshold = IsPivotingDeltaTrigger.Y; }
		else if (GaitC == CALS_Gait::Sprinting)
		{ Treshold = IsPivotingDeltaTrigger.Z; }

		if (GaitC == CALS_Gait::Walking) Treshold = 30;
		return abs(Trj_TurnAngle) > Treshold && !Trj_IsCircling;
	}
	else if (MovementStateC == CALS_MovementState::Crawl)
	{
		return abs(Trj_TurnAngle) > 20 && !Trj_IsCircling;
	}
	else
	{
		return abs(Trj_TurnAngle) > 35;
	}
}


bool UAGLS_AI_AnimInstanceBase::ShouldSpinTransition_Implementation()
{
	return abs(Trj_FutureFacingDelta) > IsSpinningDeltaTrigger && SpeedC > 80.0 && !CurrentDatabaseTags.Contains(TEXT("Pivots"));
}


bool UAGLS_AI_AnimInstanceBase::JustLanedLight()
{
	if (JustLandedC && LandVelocityC.Z > HeavyLandSpeedThreshold) return true;
	return MovementStateC == CALS_MovementState::Grounded &&
		PrevMovementStateC == CALS_MovementState::InAir &&
		VelocityLastFrameC.Z > HeavyLandSpeedThreshold;
}


bool UAGLS_AI_AnimInstanceBase::JustLanedHeavy()
{
	if (JustLandedC && LandVelocityC.Z <= HeavyLandSpeedThreshold) return true;
	return MovementStateC == CALS_MovementState::Grounded &&
		PrevMovementStateC == CALS_MovementState::InAir &&
		VelocityLastFrameC.Z <= HeavyLandSpeedThreshold;
}


bool UAGLS_AI_AnimInstanceBase::JustLandedNeutral()
{
	return JustLandedC;
}


bool UAGLS_AI_AnimInstanceBase::ShouldTurnInPlace_Implementation()
{
	switch (RotationModeC)
	{
	case CALS_RotationMode::VelocityDirection:
		return false;
	case CALS_RotationMode::LookingDirection:
		if (abs(Trj_FutureFacingDelta) >= 80 && SpeedC < 25.0 && !bHasAnyRootMotion && abs(AimYawRateC) < 40)
		{
			return GetCurveValue("Enable_Transition") > 0.8;
		}
		return false;
	case CALS_RotationMode::Aiming:

		if (bUseTurnsForAimingAsMontages) { return false; }
		if (abs(Trj_FutureFacingDelta) >= 50 && SpeedC < 25.0 && !bHasAnyRootMotion && abs(AimYawRateC) < 400)
		{
			return GetCurveValue("Enable_Transition") > KML::SelectFloat(-1, 0.1, CurrentDatabaseTags.Contains("Stops"));
		}
		return false;
	}
	return false;
}


bool UAGLS_AI_AnimInstanceBase::ShouldUseRootMotionData_Implementation()
{
	if (CharacterC)
	{
		return CharacterC->HasAnyRootMotion();
	}
	return false;
}


bool UAGLS_AI_AnimInstanceBase::GetIsWalkOnSlope()
{
	FVector OutVelocity;
	POSESEARCH_TRAJ::GetTransformTrajectoryVelocity(Trajectory, -0.15f, -0.05f, OutVelocity, false);
	if (OutVelocity.Equals(FVector::ZeroVector, 0.1)) return false;

	OutVelocity.Normalize();
	return abs(OutVelocity.Z) > 0.1;
}


bool UAGLS_AI_AnimInstanceBase::GetIsPivotingInCircleShape_Implementation()
{
	if (GetIsMovingValue() == false) return false;

	const int PointsToCheckNumber = 4;
	const FVector2D TimesRange = FVector2D(-0.5, -0.1);

	float FirstOffset = 0.0;
	FRotator FirstFacing = FRotator::ZeroRotator;

	for (int i = 0; i < PointsToCheckNumber; i++)
	{
		if (i == 0) continue;

		const float TimeA = KML::MapRangeClamped(i - 1.0, 0.0, PointsToCheckNumber - 1.0, TimesRange.X, TimesRange.Y);
		const float TimeB = KML::MapRangeClamped((float)i, 0.0, PointsToCheckNumber - 1.0, TimesRange.X, TimesRange.Y);

		FTransformTrajectorySample Trj_SampleA;
		FTransformTrajectorySample Trj_SampleB;
		FVector Trj_VelocityA = FVector::ZeroVector;
		FVector Trj_VelocityB = FVector::ZeroVector;

		UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(Trajectory, TimeA, Trj_SampleA, false);
		UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, TimeA, TimeA + 0.1, Trj_VelocityA);

		UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(Trajectory, TimeB, Trj_SampleB, false);
		UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, TimeB, TimeB + 0.1, Trj_VelocityB);

		//Make sure can normalize velocity
		if (KML::EqualEqual_VectorVector(Trj_VelocityA, FVector::ZeroVector, 0.1f) == true || KML::EqualEqual_VectorVector(Trj_VelocityB, FVector::ZeroVector, 0.1f) == true)
		{
			return false;
		}

		if ((Trj_SampleB.Position - Trj_SampleA.Position).Length() < (10.0 / PointsToCheckNumber)) return false;

		//Make direction from Velocity
		FVector DirectionA = Trj_VelocityA; DirectionA.Normalize();
		FVector DirectionB = Trj_VelocityB; DirectionB.Normalize();

		const FVector UpVec = FVector(0, 0, 1);
		const float DotBetweenSamples = KML::Dot_VectorVector(KML::Cross_VectorVector(DirectionA, UpVec), KML::Cross_VectorVector(DirectionB, UpVec));
		if (DotBetweenSamples >= 0.8) return false;


		const FRotator DirectionAsRot = FRotator(0.0, KML::MakeRotFromX(DirectionA).Yaw, 0.0);
		const FTransform RelT = KML::MakeRelativeTransform(FTransform(FRotator::ZeroRotator, Trj_SampleB.Position), FTransform(DirectionAsRot, Trj_SampleB.Position));

		if (i == 1)
		{
			FirstOffset = RelT.GetLocation().Y;
			FirstFacing = Trj_SampleA.Facing.Rotator();
		}
		else
		{
			if (FirstOffset < 0)
			{
				if (RelT.GetLocation().Y * 10 > 0.0) return false;
			}
			else
			{
				if (RelT.GetLocation().Y * 10 < 0.0) return false;
			}
		}

		if (!KML::NearlyEqual_FloatFloat(FirstFacing.Yaw, Trj_SampleB.Facing.Rotator().Yaw, 10)) return false;

	}
	return true;
}


bool UAGLS_AI_AnimInstanceBase::GetIsColliding()
{
	return CapsuleCollidingC;
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ Trajectory functions
#pragma region TRAJECTORY ANALYZE PART2

float UAGLS_AI_AnimInstanceBase::Get_TrajectoryTurnAngle_Implementation()
{
	const FRotator Delta = KML::NormalizedDeltaRotator(KML::MakeRotFromX(Trj_FutureVelocity), KML::MakeRotFromX(VelocityC));
	return Delta.Yaw;
}


float UAGLS_AI_AnimInstanceBase::Get_TrajectoryFacingDelta_Implementation(const TArray<float>& Times, FRotator RootRotation)
{
	if (Times.Num() == 0) return 0.0;

	TArray<FRotator> Rotations;
	for (int i = 0; i < Times.Num(); i++)
	{
		FTransformTrajectorySample Trj_Sample;
		UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(Trajectory, Times[i], Trj_Sample, false);
		Rotations.Add(Trj_Sample.Facing.Rotator());
	}
	float Angle = KML::NormalizedDeltaRotator(Rotations[0], RootRotation).Yaw;

	for (int ii = 0; ii < Rotations.Num(); ii++)
	{
		if (ii < Rotations.Num() - 1)
		{
			float a = KML::NormalizedDeltaRotator(Rotations[ii + 1], Rotations[ii]).Yaw;
			Angle = a + Angle;
		}
	}

	return Angle;
}


void UAGLS_AI_AnimInstanceBase::GetIsCirclingTresholds_Implementation(FVector2D& ReturnPastAngularRange, FVector2D& ReturnCurrentAngularRange, float& ReturnAngle)
{
	if (StanceC == CALS_Stance::Standing)
	{
		if (GaitC == CALS_Gait::Walking)
		{
			ReturnPastAngularRange = FVector2D(-160, 160);
			ReturnCurrentAngularRange = FVector2D(-20, 20);
			ReturnAngle = 30;
			return;
		}
		else
		{
			ReturnPastAngularRange = FVector2D(-160, 160);
			ReturnCurrentAngularRange = FVector2D(-20, 20);
			ReturnAngle = 30;
			return;
		}
	}
	else
	{
		ReturnPastAngularRange = FVector2D(-140, 140);
		ReturnCurrentAngularRange = FVector2D(-10, 10);
		ReturnAngle = 25;
		return;
	}
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
#pragma region ROOT OFFSET

int UAGLS_AI_AnimInstanceBase::GetOffsetRootRotationMode_Implementation()
{
	return 0;
}

int UAGLS_AI_AnimInstanceBase::GetOffsetRootLocationMode_Implementation()
{
	if (IsSlotActive(TEXT("BaseLayer")) || IsSlotActive(TEXT("BaseLayer-LowerPiority")) || CapsuleCollidingC) { return 3; }

	if (MovementStateC == CALS_MovementState::Grounded || MovementStateC == CALS_MovementState::Crawl)
	{
		if (IsMovingC) { return 1; }
		else return 3;
	}
	return 3;
}

float UAGLS_AI_AnimInstanceBase::GetOffsetRootTranslationHalfLife()
{
	if (IsMovingC)
	{
		return 0.3 * RootOffsetInterpSpeedMultiply;
	}
	else
	{
		return 0.1 * RootOffsetInterpSpeedMultiply;
	}
}

#pragma endregion


//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
#pragma region MOTION MATCHING / BLEND STACK / STEERING

float UAGLS_AI_AnimInstanceBase::Get_MMBlendTime()
{
	if (MovementStateC == CALS_MovementState::InAir)
	{
		if (VelocityC.Z > 100.0) { return 0.15 * BlendStackTimeBlendMultiply; }
		else { return 0.5 * BlendStackTimeBlendMultiply; }
	}
	return 0.45 * BlendStackTimeBlendMultiply;
}


EPoseSearchInterruptMode UAGLS_AI_AnimInstanceBase::Get_MMInterruptMode_Implementation()
{
	const EPoseSearchInterruptMode AsInterruptValue = EPoseSearchInterruptMode::InterruptOnDatabaseChange;

	if (bInterruptOnDatabasesLoadEnd || (bPrevCapsuleCollidingC != CapsuleCollidingC)) return AsInterruptValue;

	if
	(
		PrevIsMovingC != IsMovingC ||
		PrevMovementStateC != MovementStateC ||
		(PrevGait != GaitC && !CurrentDatabaseTags.Contains("Stops")) ||
		(PrevLocomotionModeIndex != LocomotionModeIndex && LocomotionModeIndex == 1) ||
		PrevStance != StanceC ||
		bPrevHasAnyRootMotion != bHasAnyRootMotion
	)
	{
		return AsInterruptValue;
	}
	return EPoseSearchInterruptMode::DoNotInterrupt;
}


void UAGLS_AI_AnimInstanceBase::UpdateMotionMatching_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

}


void UAGLS_AI_AnimInstanceBase::UpdateMotionMatchingPostSelection_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FMotionMatchingAnimNodeReference MM_Node;
	FPoseSearchBlueprintResult SearchResult;
	bool CastResult = false;
	bool SearchValid = true;

	UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNodePure(Node, MM_Node, CastResult); // Get Node Reference

	if (CastResult == true)
	{
		UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MM_Node, SearchResult, SearchValid);
		CurrentSelectedDatabase = const_cast<UPoseSearchDatabase*>(SearchResult.SelectedDatabase.Get());
	}

}


bool UAGLS_AI_AnimInstanceBase::EnableSteering_Implementation(const FAnimNodeReference& Node)
{
	const bool Alive = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimIsActive(Node);
	return MovementStateC == CALS_MovementState::InAir || (Alive && GetIsMovingValue());
}


bool UAGLS_AI_AnimInstanceBase::GetEnableSteeringForTurns_Implementation(const FAnimNodeReference& Node, float RotationAmoutTreshold)
{
	if (!CurrentDatabaseTags.Contains("TurnInPlace")) return false;

	const UAnimationAsset* CurrentAnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	const UAnimSequence* CurrentAnimSequence = Cast<UAnimSequence>(CurrentAnimAsset);

	if (!CurrentAnimSequence) { return false; }

	const float CurrentAnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	const FAnimExtractContext ExtractContext(static_cast<double>(KML::FClamp(CurrentAnimTime - 0.1, 0.05, 10)), false);
	const float CurveValue = CurrentAnimSequence->EvaluateCurveData(FName(TEXT("RotationAmount")), ExtractContext, false);

	return abs(CurveValue) > RotationAmoutTreshold;
}


FVector UAGLS_AI_AnimInstanceBase::GetOrientationForWarping()
{
	return FMath::Lerp<FVector>(LastNonZeroVelocityC, Trj_NearFutureVelocity, FMath::GetMappedRangeValueClamped(FVector2D(20, 120), FVector2D(0.0, 1.0), abs(Trj_CurrentAngularVelocity.Z)));
}


FQuat UAGLS_AI_AnimInstanceBase::GetDesiredFacing_Implementation(const FAnimNodeReference& Node, FVector2D FacingSampleTimeRange, FName SteringTimeCurve)
{
	const UAnimationAsset* CurrentAnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	const UAnimSequence* CurrentAnimSequence = Cast<UAnimSequence>(CurrentAnimAsset);

	if (!CurrentAnimSequence)
	{
		return FQuat::Identity;
	}

	const float CurrentAnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	const FAnimExtractContext ExtractContext(static_cast<double>(CurrentAnimTime), false);
	const float CurveValue = CurrentAnimSequence->EvaluateCurveData(SteringTimeCurve, ExtractContext, false);

	const float TargetTime = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 1.0f), FacingSampleTimeRange, CurveValue);

	FTransformTrajectorySample Sample;
	UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(Trajectory, TargetTime, Sample, false);

	return Sample.Facing;
}


float UAGLS_AI_AnimInstanceBase::GetProceduralTargetTime_Implementation(const FAnimNodeReference& Node, FVector2D DefaultTargetTimeRage, float MaxTargetTime, float ForSpinsTargetTime)
{
	const UAnimationAsset* CurrentAnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);

	const UAnimSequence* CurrentAnimSequence = Cast<UAnimSequence>(CurrentAnimAsset);

	if (!CurrentAnimSequence)
	{ return 0.1f; }

	const float CurrentAnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	const FAnimExtractContext ExtractContext( static_cast<double>(CurrentAnimTime), false );
	const float CurveValue = CurrentAnimSequence->EvaluateCurveData(FName(TEXT("SteeringTargetTime")), ExtractContext, false);

	float TargetTimeA = FMath::GetMappedRangeValueClamped(FVector2D(35, 55), DefaultTargetTimeRage, abs(Trj_FutureFacingDelta));
	if (CurrentDatabaseTags.Contains("Spins") == true) TargetTimeA = ForSpinsTargetTime;
	return FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 1.0f), FVector2D(TargetTimeA, MaxTargetTime), CurveValue);
}


float UAGLS_AI_AnimInstanceBase::Get_SpeedDifferenceDelta()
{
	if (!CharacterC) return 0.0;
	const float MaxSpeed = CharacterC->GetCharacterMovement()->GetMaxSpeed();
	return abs(MaxSpeed - SpeedC);
}


#pragma endregion



#undef KML
#undef POSESEARCH_FUNCTIONS 
#undef POSESEARCH_TRAJ 