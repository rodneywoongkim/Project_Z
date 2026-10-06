// Fill out your copyright notice in the Description page of Project Settings.


#include "AGLS_Player_AnimInstanceCore.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

#define KML UKismetMathLibrary
#define KSL UKismetSystemLibrary


bool UAGLS_Player_AnimInstanceCore::JustTeleport()
{
	return FVector::DistSquared(CharacterTransform_LastFrame.GetLocation(), CharacterTransformC.GetLocation()) > (TeleportTreshold * TeleportTreshold);
}


bool UAGLS_Player_AnimInstanceCore::EnableSteering_Implementation(const FAnimNodeReference& Node)
{
	return false;
}


FQuat UAGLS_Player_AnimInstanceCore::GetDesiredFacing_Implementation(FAnimNodeReference Node)
{
	FTransformTrajectorySample Trj_Sample;
	UPoseSearchTrajectoryLibrary::GetTransformTrajectorySampleAtTime(Trajectory, 0.1f, Trj_Sample, false);
	return Trj_Sample.Facing;
}


float UAGLS_Player_AnimInstanceCore::GetProceduralTargetTime_Implementation(FAnimNodeReference Node)
{
	return 0.1f;
}


void UAGLS_Player_AnimInstanceCore::UpdateMotionMatching_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
}


void UAGLS_Player_AnimInstanceCore::UpdateMotionMatching_PostSelection_Implementation(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
}


float UAGLS_Player_AnimInstanceCore::Get_MMBlendTime_Implementation()
{
	return 0.0f;
}


bool UAGLS_Player_AnimInstanceCore::GetIsMoving_Implementation()
{
	return UKismetMathLibrary::NotEqual_VectorVector(Trj_FutureVelocity, FVector::ZeroVector, 10) && 
		UKismetMathLibrary::NotEqual_VectorVector(ActorAcceleration, FVector::ZeroVector, 10);
}


bool UAGLS_Player_AnimInstanceCore::GetIsStarting_Implementation()
{
	bool StartingValue = GetIsMoving() && 
		!(SpeedC > 100) && 
		(Trj_FutureVelocity.Size2D() >= VelocityC.Size2D() + 100) && 
		!CurrentDatabaseTags.Contains(TEXT("Pivots"));
	
	return StartingValue;
}



bool UAGLS_Player_AnimInstanceCore::GetIsStoping_Implementation()
{
	return !IsMovingC && FutureVelocityC.Size2D() < 10 && VelocityC.Size2D() > 10 && !CurrentDatabaseTags.Contains(TEXT("Pivots"));
}


bool UAGLS_Player_AnimInstanceCore::GetIsPivoting_Implementation()
{
	if (CurrentStance == CALS_Stance::Standing)
	{
		float Tollerance = 75;
		switch (CurrentRotationMode)
		{
		case CALS_RotationMode::VelocityDirection:
			Tollerance = 30.0; break;
		case CALS_RotationMode::LookingDirection:
			Tollerance = 60.0; break;
		case CALS_RotationMode::Aiming:
			Tollerance = 60.0; break;
		}
		return abs(Traj_TurnAngle) >= Tollerance && !Traj_IsCircling;
	}
	else
	{
		return abs(Traj_TurnAngle) >= 35.0;
	}
}

bool UAGLS_Player_AnimInstanceCore::GetIsPivotingSmallDelta_Implementation()
{
	const float Tollerance = 35;
	return abs(Traj_TurnAngle) >= Tollerance && !Traj_IsCircling;
}


bool UAGLS_Player_AnimInstanceCore::GetShouldTurnInPlace_Implementation()
{
	if (CurrentRotationMode == CALS_RotationMode::Aiming) return false;
	if (abs(FutureFacingDelta) >= 50 && SpeedC < 25.0)
	{
		if (MovementStateC == CALS_MovementState::Grounded &&
			PickUpLootItemC &&
			GetCurveValue("Enable_Transition") > 0.5
			&& abs(AimYawRate) < 12
			)
		{
			return true;
		}
	}
	return false;
}


bool UAGLS_Player_AnimInstanceCore::GetShouldSpinTransition_Implementation()
{
	return abs(FutureFacingDelta) > 130.0 && SpeedC > 150.0 && !CurrentDatabaseTags.Contains(TEXT("Pivots"));
}


bool UAGLS_Player_AnimInstanceCore::GetIsPivotingInCircleShape_Implementation()
{
	if (GetIsMoving() == false) return false;

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

		Trj_VelocityA.Z = 0.0;
		Trj_VelocityB.Z = 0.0;

		//Make sure can normalize velocity
		if (KML::EqualEqual_VectorVector(Trj_VelocityA, FVector::ZeroVector, 0.1f) == true || KML::EqualEqual_VectorVector(Trj_VelocityB, FVector::ZeroVector, 0.1f) == true)
		{ return false; }

		const FVector SampleA_PositionXY = FVector(Trj_SampleA.Position.X, Trj_SampleA.Position.Y, 0.0);
		const FVector SampleB_PositionXY = FVector(Trj_SampleB.Position.X, Trj_SampleB.Position.Y, 0.0);

		if ((SampleB_PositionXY - SampleA_PositionXY).Length() < (10.0 / PointsToCheckNumber)) return false;

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


bool UAGLS_Player_AnimInstanceCore::JustTraversed()
{
	return GetCurveValue(TEXT("MovingTraversal")) > 0.0 && abs(Get_TrajectoryTurnAngle()) <= 50.0 && !IsSlotActive(TEXT("BaseLayer"));
}


bool UAGLS_Player_AnimInstanceCore::JustLanded_Light()
{
	if (JustLandedC && LandVelocityC.Z > HeavyLandSpeedThreshold) return true;
	return MovementStateC == CALS_MovementState::Grounded && 
		PrevMovementStateC == CALS_MovementState::InAir && 
		VelocityLastFrameC.Z > HeavyLandSpeedThreshold;
}


bool UAGLS_Player_AnimInstanceCore::JustLanded_Heavy()
{
	if (JustLandedC && LandVelocityC.Z <= HeavyLandSpeedThreshold) return true;
	return MovementStateC == CALS_MovementState::Grounded &&
		PrevMovementStateC == CALS_MovementState::InAir &&
		VelocityLastFrameC.Z <= HeavyLandSpeedThreshold;
}


float UAGLS_Player_AnimInstanceCore::GetLandVelocity()
{
	return LandVelocityC.Z;
}


bool UAGLS_Player_AnimInstanceCore::GetIsColliding()
{
	return CapsuleCollidingC;
}




float UAGLS_Player_AnimInstanceCore::Get_TrajectoryTurnAngle_Implementation()
{
	const FRotator Delta = KML::NormalizedDeltaRotator(KML::MakeRotFromX(Trj_FutureVelocity), KML::MakeRotFromX(VelocityC));
	return Delta.Yaw;
}


float UAGLS_Player_AnimInstanceCore::Get_TrajectoryFacingDelta_Implementation(const TArray<float>& Times, FRotator RootRotation)
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



void UAGLS_Player_AnimInstanceCore::GenerateTrajectory_Implementation()
{
}


void UAGLS_Player_AnimInstanceCore::OnInstanceInitializeNotSafe_Implementation()
{
}


void UAGLS_Player_AnimInstanceCore::UpdateEssentialValuesSafe_Implementation()
{
}


void UAGLS_Player_AnimInstanceCore::UpdateLayeringValues_Implementation(int32 InLocomotionIndex, bool MakeBasePoseAlphaFromState, float BasePoseInterpSpeed, float BendDownAlphaStrength)
{
	//Set the Base Pose weights
	if (MakeBasePoseAlphaFromState)
	{
		float StanceAsFloat = 0.0; if (CurrentStance == CALS_Stance::Standing) StanceAsFloat = 1.0;
		const float OutInterpTime = KML::FClamp(BasePoseInterpSpeed, 0.01, 1000);

		BasePose_N = KML::FClamp(KML::FInterpTo_Constant(BasePose_N, StanceAsFloat, DeltaTimeX, OutInterpTime), 0.0, 1.0);
		BasePose_CLF = 1 - BasePose_N;
	}
	else
	{
		BasePose_N = GetCurveValue("BasePose_N");
		BasePose_CLF = GetCurveValue("BasePose_CLF");
	}

	//Set the Hand Override weights
	Layering_HandR = GetCurveValue("Layering_Hand_R");
	Layering_HandL = GetCurveValue("Layering_Hand_L");

	//Set whether the arms should blend in mesh space or local space. The Mesh space weight will always be 1 unless the Local Space (LS) curve is fully weighted.
	Layering_ArmL_LS = GetCurveValue("Layering_Arm_L_LS");
	Layering_ArmL_MS = (1 - KML::FFloor(Layering_ArmL_LS)) * 1.0;

	Layering_ArmR_LS = GetCurveValue("Layering_Arm_R_LS");
	Layering_ArmR_MS = (1 - KML::FFloor(Layering_ArmR_LS)) * 1.0;


	SecondaryMotionMaskC = GetCurveValue("BasePose_Crawl") + GetCurveValue("BasePose_Ladder") + GetCurveValue("BasePose_CMC_Climbing");
	SecondaryMotionMaskC = SecondaryMotionMaskC + (bStartCovering * 0.65);
	const FGameplayTag InspectNoteTag = FGameplayTag::RequestGameplayTag(TEXT("Interaction.InspectNote"));
	SecondaryMotionMaskC = SecondaryMotionMaskC + (OwnerTagContainer.HasTag(InspectNoteTag) * 1.0);
	SecondaryMotionMaskC = KML::FClamp(SecondaryMotionMaskC, 0.2, 1.0);


	const bool LocomotionInRange = KML::InRange_IntInt(InLocomotionIndex, 6, 8) || KML::InRange_IntInt(InLocomotionIndex, 10, 15) || InLocomotionIndex == 2;
	const bool NoAlphaBias = MovementStateC == CALS_MovementState::Crawl || JustLandedC || bStartCovering || LocomotionInRange;

	float DesiredBendAlpha = DistanceToNearestOpponent; if (NoAlphaBias) DesiredBendAlpha = 0.0;
	DesiredBendAlpha = DesiredBendAlpha * BendDownAlphaStrength;

	BlendOverlayWithCoverModeC = KML::Lerp(KML::FInterpTo(BlendOverlayWithCoverModeC, DesiredBendAlpha, DeltaTimeX, 1.2), 0.0, GetCurveValue("Mask_OverlayCrouchPosture"));

	/*
	Some actions, such as reloading, have animations prepared only for Stance == Standing, and additionally, during these sequences, it is possible to switch to Crouching. 
	In such a case, Layering_Legs and Layering_Pelvis should be set to 0.0. Therefore, if the 'OverrideBlendPelvis' variable is greater than 0.0, the value of these curves 
	will be modified.
	
	Niektóre akcje takie jak np. prze³adowanie maj¹ przygotowanie animacje tylko dla Stance == Standing, a dodatkowo w trakcie tych sekwencji mo¿liwoœæ przejœcia na Crouching 
	jest mo¿liwa. W takim przypadku Layering_Legs, oraz Layering_Pelvis powinno byæ ustawione na 0.0. Dlatego te¿ je¿eli Zmienna 'OverrideBlendPelvis' bêdzie wiêksza od 0.0 to 
	wartoœæ tych krzywych bêdzie modyfikowana. */
	//const FGameplayTag ActionTagA = FGameplayTag::RequestGameplayTag(TEXT("Action.Reload"));
	//const FGameplayTag ActionTagB = FGameplayTag::RequestGameplayTag(TEXT("Action.SwitchPistol"));
	const bool HasActionTag = OwnerTagContainer.HasAny(OverridePelvisBlendForThisTags);
	OverridePelvisBlending = KML::FInterpTo_Constant(OverridePelvisBlending, BasePose_CLF * HasActionTag, DeltaTimeX, KML::SelectFloat(50, 0.5, HasActionTag));
	
}


void UAGLS_Player_AnimInstanceCore::UpdateAimingValues_Implementation(float SmoothingTime, int NumOfSpineBones, bool UseSpineRotSmoothing, bool UseRootRotationAsRef)
{
	FRotator RotationDeltaBase = FRotator::ZeroRotator;

	if (UseRootRotationAsRef)
	{
		RotationDeltaBase = KML::RLerp(
			FRotator(0, RootTransformC.Rotator().Yaw + GetCurveValue("AimingRotationOffset"), 0),
			FRotator(0, RootTransformC.Rotator().Yaw + 180, 0),
			GetCurveValue("Aiming_Offset_Type"), true);
	}
	else
	{
		RotationDeltaBase = KML::RLerp(
			FRotator(0, CharacterTransformC.Rotator().Yaw + GetCurveValue("AimingRotationOffset"), 0),
			FRotator(0, CharacterTransformC.Rotator().Yaw + 180, 0),
			GetCurveValue("Aiming_Offset_Type"), true);
	}

	//Interp the Aiming Rotation value to achieve smooth aiming rotation changes. Interpolating the rotation before calculating the 
	// angle ensures the value is not affected by changes in actor rotation, allowing slow aiming rotation changes with fast actor rotation changes.

	if (!TryGetPawnOwner()) return;
	SmoothedAimingRotationC = KML::RInterpTo(SmoothedAimingRotationC, TryGetPawnOwner()->GetControlRotation(), DeltaTimeX, SmoothingTime);

	//Calculate the Aiming angle and Smoothed Aiming Angle by getting the delta between the aiming rotation and the actor rotation.
	const FRotator RotDeltaA = KML::NormalizedDeltaRotator(TryGetPawnOwner()->GetControlRotation(), RotationDeltaBase);
	const FRotator RotDeltaB = KML::NormalizedDeltaRotator(SmoothedAimingRotationC, RotationDeltaBase);
	AimingAngleC = FVector2D(RotDeltaA.Yaw, RotDeltaA.Pitch);
	SmoothedAimingAngleC = FVector2D(RotDeltaB.Yaw, RotDeltaB.Pitch);

	//Clamp the Aiming Pitch Angle to a range of 1 to 0 for use in the vertical aim sweeps.
	if (CurrentRotationMode != CALS_RotationMode::VelocityDirection)
	{
		AimSweepTimeC = KML::MapRangeClamped(AimingAngleC.Y, -90, 90, 1.0, 0.0);

		//Use the Aiming Yaw Angle divided by the number of spine+pelvis bones to get the amount of spine rotation needed to remain facing the camera direction.
		if (UseSpineRotSmoothing)
		{
			const float SpineRotDelta = abs((AimingAngleC.X / NumOfSpineBones) - SpineRotationC.Yaw);
			float InterpMin = 20.0; if (CurrentRotationMode == CALS_RotationMode::Aiming) { InterpMin = 120.0; }

			SpineRotationC = KML::RInterpTo_Constant(SpineRotationC, FRotator(0, AimingAngleC.X / NumOfSpineBones, 0), GetDeltaSeconds(), KML::MapRangeClamped(SpineRotDelta, 30, 80, 300, InterpMin));
		}
		else
		{
			SpineRotationC.Yaw = AimingAngleC.X / (NumOfSpineBones * 1.0);
		}

	}

}


void UAGLS_Player_AnimInstanceCore::AsyncLoadOverlayEvent_Implementation(CALS_OverlayState NewState)
{
}


void UAGLS_Player_AnimInstanceCore::AsyncLoadSecondOverlayEvent_Implementation()
{
}


bool UAGLS_Player_AnimInstanceCore::CalculateIsCollidingValue_Implementation(float SpeedScaleTollerance, float TimeDilatation, bool bDrawTrace)
{
	if (!Character) return false;

	const float DilatationTime = TimeDilatation;

	bool CapCollideResult = false;

	const FVector MainLocation = RootTransformC.GetLocation() + (KML::GetUpVector(RootTransformC.Rotator()) * Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	const FVector VelocityDeltaForce = VelocityC * KML::SafeDivide(DeltaTimeX * 2.0, DilatationTime);
	const FVector AccelerationDeltaForce = Character->GetCharacterMovement()->GetCurrentAcceleration() * (KML::SafeDivide((DeltaTimeX * DeltaTimeX) * 2, DilatationTime));

	const FVector TraceStart = MainLocation + VelocityDeltaForce + AccelerationDeltaForce + FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere() * 0.98f);
	const FVector TraceEnd = MainLocation + VelocityDeltaForce + AccelerationDeltaForce - FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere() * 0.98f);
	const float CapRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();

	const ETraceTypeQuery TraceType = ETraceTypeQuery::TraceTypeQuery1;
	TArray<AActor*> ActorsToIgnore;
	EDrawDebugTrace::Type DebugTrace = EDrawDebugTrace::None; if (bDrawTrace) DebugTrace = EDrawDebugTrace::ForOneFrame;
	FHitResult HitResult;

	const bool HitValid = KSL::SphereTraceSingle(Character, TraceStart, TraceEnd, CapRadius, TraceType, false, ActorsToIgnore, DebugTrace, HitResult, true, FLinearColor::Black, FLinearColor::Red, 0.05f);
	if (HitValid)
	{
		CapCollideResult = (HitResult.ImpactPoint.Z - RootTransformC.GetLocation().Z) > Character->GetCharacterMovement()->MaxStepHeight;
	}
	else
	{
		CapCollideResult = false;
	}

	return CapCollideResult && (Character->GetCharacterMovement()->MaxWalkSpeed - SpeedC) > (Character->GetCharacterMovement()->MaxWalkSpeed * SpeedScaleTollerance);
}


#undef KML 
#undef KSL 