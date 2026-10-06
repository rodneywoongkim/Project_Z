// AGLS v1.9 | JakubW 2026


#include "MovementParamsControlComponent.h"

#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"

#define STATE_LOOKING CALS_RotationMode::LookingDirection
#define STATE_FACING CALS_RotationMode::VelocityDirection
#define STATE_AIMING CALS_RotationMode::Aiming
#define S_STANDING CALS_Stance::Standing
#define S_CROUCHING CALS_Stance::Crouching
#define KML UKismetMathLibrary


// Sets default values for this component's properties
UMovementParamsControlComponent::UMovementParamsControlComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}


// Called when the game starts
void UMovementParamsControlComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterRef = Cast<ACharacter>(GetOwner());
	if (CharacterRef)
	{
		CMC = CharacterRef->GetCharacterMovement();
		if (CMC)
		{
			CMC->AddTickPrerequisiteComponent(this);
		}
	}

	SetNewMovementPreset(DefaultMovementPreset, false);
}



// Called every frame
void UMovementParamsControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableComponent) return;
	if (!CurrentMovementPreset) return;

	if (CMC) SpeedValue2D = CMC->Velocity.Size2D();

	CollectStatesValues(CollectedMovement, CollectedStance, CollectedRotationMode, CollectedDesiredGait, CollectedCurrentGait);

	bool bShouldSkip = false;
	if (bSkipUpdateWhenNotOnGround)
	{
		bShouldSkip = CollectedMovement == CALS_MovementState::None || CollectedMovement == CALS_MovementState::InAir || 
					  CollectedMovement == CALS_MovementState::Ragdoll || CollectedMovement == CALS_MovementState::Mantling;
	}

	if (CalculateSimpleOnStairsValue)
	{
		WalkingOnStairsAlpha = UpdateWalkOnStairsAlpha();
	}

	CALS_Gait AllowedGaitLocal = GetAllowedGait();
	CALS_Gait ActualGaitLocal = GetActualGait(AllowedGaitLocal);
	ActualGaitState = ActualGaitLocal;
	if (CollectedCurrentGait != ActualGaitLocal)
	{
		SetNewGaitState(ActualGaitLocal);
	}

	if (bTransitionIsRunning)
	{
		TransitionBlendAlpha = KML::MapRangeClamped(CurrentPresetTransitionTimer, 0.0, CurrentPresetsTransitionDuration, 1.0, 0.0);
		CurrentPresetTransitionTimer = CurrentPresetTransitionTimer - DeltaTime;

		if (CurrentPresetTransitionTimer <= 0.0)
		{
			bTransitionIsRunning = false;
			TransitionBlendAlpha = 1.0;
			CurrentPresetTransitionTimer = -1;
		}
	}

	if (!bShouldSkip && GetCustomUpdatingCondition())
	{
		if (UseInterpModeToCalculateGaitMap) UpdateMappedSpeedInterpMode(1.0, ActualGaitLocal, DeltaTime);
		UpdateDynamicMovementSettings(ActualGaitLocal);
	}

	OrientationIntent = GetOrientationIntent(OrientationIntent);
	CalculateAndUpdateMovementDirectionState(FVector::ZeroVector, OrientationIntent, ActualGaitLocal, CurrentMovementPreset->SprintDiectionStateAlwaysForward);


	if (AnyStateChangedTimer > 0.0)
	{ AnyStateChangedTimer = FMath::Clamp<float>(AnyStateChangedTimer - DeltaTime, 0.0, 10.0); }
	else if (AnyStateChangedTimer == 0.0)
	{ AnyStateChangedTimer = -1; }

	//Debug Section
	if (bDisplayDebugInformations) RenderDebugInformations(DeltaTime, ActualGaitLocal);

	TickBeforeMovementComponent.Broadcast(DeltaTime); //CALL TICK DELEGATE (SHOULD BE EXECUTER BEFORE Character Movement Component)

}


FCALSMovementSettingsStrafeExtend UMovementParamsControlComponent::BlendTwoModels(FCALSMovementSettingsStrafeExtend PrevModel, FCALSMovementSettingsStrafeExtend CurrentModel)
{
	if (bTransitionIsRunning)
	{
		FCALSMovementSettingsStrafeExtend TransitionModel = PrevModel;
		TransitionModel.WalkSpeed = KML::VLerp(PrevModel.WalkSpeed, CurrentModel.WalkSpeed, TransitionBlendAlpha);
		TransitionModel.RunSpeed = KML::VLerp(PrevModel.RunSpeed, CurrentModel.RunSpeed, TransitionBlendAlpha);
		TransitionModel.SprintSpeed = KML::VLerp(PrevModel.SprintSpeed, CurrentModel.SprintSpeed, TransitionBlendAlpha);

		if (TransitionBlendAlpha > 0.5)
		{
			TransitionModel.MatchingWalkDatabasesToSpeed = CurrentModel.MatchingWalkDatabasesToSpeed;
			TransitionModel.MatchingRunDatabasesToSpeed = CurrentModel.MatchingRunDatabasesToSpeed;
			TransitionModel.MatchingSprintDatabasesToSpeed = CurrentModel.MatchingSprintDatabasesToSpeed;
			TransitionModel.MovementCurve = CurrentModel.MovementCurve;
			TransitionModel.RotationRateCurve = CurrentModel.RotationRateCurve;
		}
		return TransitionModel;
	}

	return CurrentModel;
}


void UMovementParamsControlComponent::CollectStatesValues_Implementation(CALS_MovementState& InMovementState, CALS_Stance& InStance, CALS_RotationMode& InRotationMode, CALS_Gait& InDesiredGait, CALS_Gait& InCurrentGait)
{
	if (InMovementState != CollectedMovement || InStance != CollectedStance || InRotationMode != CollectedRotationMode || InDesiredGait != CollectedDesiredGait)
	{
		AnyStateChangedTimer = 1.0;
	}
}


CALS_Gait UMovementParamsControlComponent::GetAllowedGait_Implementation()
{
	const CALS_Gait DesiredGaitA = CollectedDesiredGait;
	const bool bIsPushingObject = false;

	if (CollectedStance == CALS_Stance::Standing)
	{
		if (CollectedRotationMode != STATE_AIMING)
		{
			if (bIsPushingObject) return CALS_Gait::Walking;

			switch (DesiredGaitA)
			{
			case CALS_Gait::Walking:
				return CALS_Gait::Walking;
			case CALS_Gait::Running:
				return CALS_Gait::Running;
			case CALS_Gait::Sprinting:
				if (WalkingOnStairsAlpha > 0.5) { return CALS_Gait::Running; }
				else { return CALS_Gait::Sprinting; }
			}
		}
		else
		{
			switch (DesiredGaitA)
			{
			case CALS_Gait::Walking:
				return CALS_Gait::Walking;
			case CALS_Gait::Running:
				return CALS_Gait::Running;
			case CALS_Gait::Sprinting:
				return CALS_Gait::Running;
			}
		}
	}
	else
	{
		switch (DesiredGaitA)
		{
		case CALS_Gait::Walking:
			return CALS_Gait::Walking;
		case CALS_Gait::Running:
			return CALS_Gait::Running;
		case CALS_Gait::Sprinting:
			return CALS_Gait::Running;
		}
	}
	return CALS_Gait::Walking;
}


CALS_Gait UMovementParamsControlComponent::GetActualGait_Implementation(CALS_Gait InAllowedGait)
{
	if (!CMC) return InAllowedGait;

	float Speed2D = CMC->Velocity.Size2D();

	if (CurrentMovementPreset)
	{
		if (CurrentMovementPreset->CanSprintOnlyInForwardMove && InAllowedGait == CALS_Gait::Sprinting)
		{
			const float DotTo = KML::Dot_VectorVector(GetMovementInput(), OrientationIntent);

			if (GetMovementInput().Equals(FVector::ZeroVector, 0.05) && CurrentMovementPreset->CanSetSprintGaitWhenNoInputs && Speed2D < 5.0)
			{
				return InAllowedGait;
			}

			if (DotTo < CurrentMovementPreset->GaitSprintingTreshold)
			{
				return CALS_Gait::Running;
			}
		}

		if(CurrentMovementPreset->CanSetSprintGaitWhenNoInputs) return InAllowedGait;
		if (!CMC->GetPendingInputVector().Equals(FVector::ZeroVector, 0.01) || Speed2D > 20.0)
		{
			return InAllowedGait;
		}
		else
		{
			if (InAllowedGait == CALS_Gait::Sprinting) return CALS_Gait::Running;
			return InAllowedGait;
		}
	}
	return InAllowedGait;
}


void UMovementParamsControlComponent::SetNewGaitState_Implementation(CALS_Gait NewState)
{
	CallOnNewGaitDeleage(NewState, CollectedDesiredGait);
}



bool UMovementParamsControlComponent::GetCustomUpdatingCondition_Implementation()
{
	return true;
}


FCALSMovementSettingsStrafeExtend UMovementParamsControlComponent::GetTargetMovementSettings()
{
	if (!CurrentMovementPreset)
	{
		//Not Valid
		return FCALSMovementSettingsStrafeExtend();
	}

	if (WalkingOnStairsAlpha > 0.5 && CurrentMovementPreset->bIncludeSettingsForStairsWalk)
	{
		if (CollectedStance == S_STANDING) return BlendTwoModels(PrevMovementPreset->OnStairsSettings_Stand, CurrentMovementPreset->OnStairsSettings_Stand);
		else BlendTwoModels(PrevMovementPreset->OnStairsSettings_Crouch, CurrentMovementPreset->OnStairsSettings_Crouch);
	}

	switch (CollectedRotationMode)
	{
	case CALS_RotationMode::VelocityDirection:
		if (CollectedStance == S_STANDING) return BlendTwoModels(PrevMovementPreset->VelocityDirectionSettings_Stand, CurrentMovementPreset->VelocityDirectionSettings_Stand);
		else return BlendTwoModels(PrevMovementPreset->VelocityDirectionSettings_Crouch, CurrentMovementPreset->VelocityDirectionSettings_Crouch);
	case CALS_RotationMode::LookingDirection:
		if (CollectedStance == S_STANDING) return BlendTwoModels(PrevMovementPreset->LookingDirectionSettings_Stand, CurrentMovementPreset->LookingDirectionSettings_Stand);
		else return BlendTwoModels(PrevMovementPreset->LookingDirectionSettings_Crouch, CurrentMovementPreset->LookingDirectionSettings_Crouch);
	case CALS_RotationMode::Aiming:
		if (CollectedStance == S_STANDING) return BlendTwoModels(PrevMovementPreset->AimingDirectionSettings_Stand, CurrentMovementPreset->AimingDirectionSettings_Stand);
		else return BlendTwoModels(PrevMovementPreset->AimingDirectionSettings_Crouch, CurrentMovementPreset->AimingDirectionSettings_Crouch);
	default:
		return CurrentMovementPreset->LookingDirectionSettings_Stand;
	}
}


float UMovementParamsControlComponent::CalculateMaxSpeed(CALS_Gait AllowedGait)
{
	FVector SpeedVector = FVector(200, 200, 200);

	switch (AllowedGait)
	{
	case CALS_Gait::Walking:
		SpeedVector = CurrentMovementModel.WalkSpeed;
		break;
	case CALS_Gait::Running:
		SpeedVector = CurrentMovementModel.RunSpeed;
		break;
	case CALS_Gait::Sprinting:
		SpeedVector = CurrentMovementModel.SprintSpeed;
		break;
	}

	return GetTargetSpeedWithStrafe(SpeedVector);
}


float UMovementParamsControlComponent::GetMappedSpeed(float Scale, CALS_Gait AllowedGait)
{
	const float LocWalkSpeed = GetTargetSpeedWithStrafe(CurrentMovementModel.WalkSpeed);
	const float LocRunSpeed = GetTargetSpeedWithStrafe(CurrentMovementModel.RunSpeed);
	const float LocSprintSpeed = GetTargetSpeedWithStrafe(CurrentMovementModel.SprintSpeed);


	if (SpeedValue2D >= LocRunSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedValue2D, LocRunSpeed, LocSprintSpeed, 2.0, 3.0);
	}
	else if (SpeedValue2D >= LocWalkSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedValue2D, LocWalkSpeed, LocRunSpeed, 1.0, 2.0);
	}
	else
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedValue2D, 0, LocWalkSpeed, 0.0, 1.0);
	}
}


float UMovementParamsControlComponent::UpdateMappedSpeedInterpMode(float Scale, CALS_Gait AllowedGait, float dt)
{
	float DesiredGaitValue = 0.0;

	if (SpeedValue2D > -5)
	{
		switch (AllowedGait)
		{
		case CALS_Gait::Walking:
			DesiredGaitValue = 1.0; break;
		case CALS_Gait::Running:
			DesiredGaitValue = 2.0; break;
		case CALS_Gait::Sprinting:
			DesiredGaitValue = 3.0; break;
		}
	}
	float Delta = abs(DesiredGaitValue - SmoothGaitTransition);

	SmoothGaitTransition = KML::FInterpTo_Constant(SmoothGaitTransition, DesiredGaitValue, dt, GaitMapInterpSpeed + (Delta * 2));
	return SmoothGaitTransition;
}


float UMovementParamsControlComponent::GetTargetSpeedWithStrafe(FVector InSpeeds)
{
	if (StrafeSpeedMapCurve)
	{
		float StrafeSpeedMap = StrafeSpeedMapCurve->GetFloatValue(abs(UKismetAnimationLibrary::CalculateDirection(CharacterRef->GetCharacterMovement()->Velocity, CharacterRef->GetActorRotation())));
		if (StrafeSpeedMap < 1.0)
		{
			return UKismetMathLibrary::MapRangeClamped(StrafeSpeedMap, 0.0, 1.0, InSpeeds.X, InSpeeds.Y);
		}
		else
		{
			return UKismetMathLibrary::MapRangeClamped(StrafeSpeedMap, 1.0, 2.0, InSpeeds.Y, InSpeeds.Z);
		}
	}
	return 300.0;
}


FVector UMovementParamsControlComponent::GetMovementInput_Implementation()
{
	if (!CMC) return FVector::ZeroVector;
	return CMC->GetPendingInputVector();
}


FVector4d UMovementParamsControlComponent::GetMovementDirectionTreshold()
{
	switch (CollectedRotationMode)
	{
	case CALS_RotationMode::VelocityDirection:
		//When the Strafe or Aim style is 2, ony the forward direction will be used. This means the character will always play the forward loco anims regardless of the strafe direction.
		return FVector4d(-180, 180, -180, 180);
	case CALS_RotationMode::LookingDirection:
		//When the Strafe or Aim style is 0, the character will use all 4 directional animations, which is how many games handle strafing.
		if (CurrentMovementDirection == AGLS_MovementDirectionState::F || CurrentMovementDirection == AGLS_MovementDirectionState::B)
		{ return FVector4d(-60, 60, -120, 120); }
		else
		{ return FVector4d(-40, 40, -140, 140); }
	case CALS_RotationMode::Aiming:
		//When the Strafe or Aim style is 0, the character will use all 4 directional animations, which is how many games handle strafing.
		if (CurrentMovementDirection == AGLS_MovementDirectionState::F || CurrentMovementDirection == AGLS_MovementDirectionState::B)
		{ return FVector4d(-60, 60, -120, 120); }
		else
		{ return FVector4d(-40, 40, -140, 140); }
	}
	return FVector4d(-180, 180, -180, 180);
}


AGLS_MovementDirectionState UMovementParamsControlComponent::GetMovementDirectionStateFromTresholds(FVector4d InTresholds, float Direction)
{
	if (KML::InRange_FloatFloat(Direction, InTresholds.X, InTresholds.Y))
	{
		return AGLS_MovementDirectionState::F;
	}
	else if (KML::InRange_FloatFloat(Direction, InTresholds.Z, InTresholds.X))
	{
		return AGLS_MovementDirectionState::LL;
	}
	else if (KML::InRange_FloatFloat(Direction, InTresholds.Y, InTresholds.W))
	{
		return AGLS_MovementDirectionState::RL;
	}
	else
	{
		return AGLS_MovementDirectionState::B;
	}
}


void UMovementParamsControlComponent::CalculateAndUpdateMovementDirectionState(FVector MoveInput, FVector InOrientationIntent, CALS_Gait InGait, bool AlwaysForwardWhenSprint)
{
	AGLS_MovementDirectionState SavedDirection = CurrentMovementDirection;
	float DebugAngle = 0.0;

	//If the character is in the OrientToMovement Rotation Mode, then set the returned Movement Direction to F with no offset.
	if (CollectedRotationMode == CALS_RotationMode::VelocityDirection)
	{
		PreviousMovementDirection = SavedDirection;
		CurrentMovementDirection = AGLS_MovementDirectionState::F;
		return;
	}

	//First we cache the Direction Of Movement which is essentially the direction the pawn will be moving in. Since the InAir and Sliding modes have low friction, we use the velocity.
	FVector DirectionOfMovement = GetCurrentMovementDirection(); // AGLS v1.9.1 !!!!


	if (KML::EqualEqual_VectorVector(DirectionOfMovement, FVector::ZeroVector, 0.001f) == true)
	{
		DebugAngle = 0.0;
		PreviousMovementDirection = SavedDirection;
		CurrentMovementDirection = AGLS_MovementDirectionState::F;
		return;
	}

	//Next we find the delta between the Direction of Movement and the Orientation Intent, cached as a value between -180 and 180.
	float MovementDirectionAngle = KML::NormalizedDeltaRotator(KML::MakeRotFromX(DirectionOfMovement), KML::MakeRotFromX(InOrientationIntent)).Yaw;
	DebugAngle = MovementDirectionAngle;

	//ClampAngleToPreventConstantFlippingAt180
	if (PreviousMovementDirection == AGLS_MovementDirectionState::F)
	{
		bool AngleCondition = false;
		if (RotationOffsetPreSim == 0.0)
		{
			const float ActorDeltaTo = KML::NormalizedDeltaRotator(CharacterRef->GetActorRotation(), KML::MakeRotFromX(InOrientationIntent)).Yaw;
			AngleCondition = ActorDeltaTo > 0.0;
		}
		else
		{
			AngleCondition = RotationOffsetPreSim > 0.0;
		}

		if (AngleCondition)
		{
			MovementDirectionAngle = KML::SelectFloat(179, MovementDirectionAngle, KML::InRange_FloatFloat(MovementDirectionAngle, -180, -170));
		}
		else
		{
			MovementDirectionAngle = KML::SelectFloat(-179, MovementDirectionAngle, KML::InRange_FloatFloat(MovementDirectionAngle, 170, 180));
		}
		DebugAngle = MovementDirectionAngle;
	}

	//Based on the Movement Direction Angle and the Movement Direction Thresholds, we get the desired Movement Direction.
	AGLS_MovementDirectionState NewMovementDirection = GetMovementDirectionStateFromTresholds(GetMovementDirectionTreshold(), MovementDirectionAngle);

	//This fallback ensures that the movement direction is always F when sprinting, since we have no strafe sprint coverage.
	if (InGait == CALS_Gait::Sprinting && AlwaysForwardWhenSprint) NewMovementDirection = AGLS_MovementDirectionState::F;


	//Now that we have the Movement Direction Angle and the desired Movement Direction, we can determine how to offset the pawn's rotation. We do 
	// this via curve assets for easy control, since each direction may need slightly different offset behavior. These curve assets are stored in a 
	// chooser and are selected based on the MovementDirection and MovementMode, and the Movement Direction Angle is used as the InTime for the curves. 
	// The curve value (y axis) at the InTime (x axis) determines the Rotation Offset angle passed into mover.


	//Return
	PreviousMovementDirection = SavedDirection;
	CurrentMovementDirection = NewMovementDirection;
}


FVector UMovementParamsControlComponent::GetOrientationIntent_Implementation(FVector CurrentOrientationIntent)
{
	FVector ControlDirection = KML::GetForwardVector(FRotator(0.0, GetAimingRotation().Yaw, 0.0));

	if (CollectedMovement == CALS_MovementState::Grounded || CollectedMovement == CALS_MovementState::Crawl)
	{
		if (KML::NotEqual_VectorVector(GetMovementInput(), FVector::ZeroVector) == true)
		{
			//When on the ground WITH movement input applied, the Orientation Intent will be the movement input direction when in OrientToMovement, or the Aiming Rotation when in Strafe or Aim.
			if (CollectedRotationMode == CALS_RotationMode::VelocityDirection)
			{
				return GetMovementInput();
			}
			else
			{
				return ControlDirection;
			}
		}
		else
		{
			if (CollectedRotationMode != CALS_RotationMode::Aiming)
			{
				//When on the ground WITHOUT movement input applied while in OrientToMovement or Strafe, we simply return last frames values, meaning the OrientationIntent will not change.
				return CurrentOrientationIntent;
			}
			else
			{
				//When on the ground WITHOUT movement input applies while in the Aim mode, then update the OrientationIntent to be the Aiming 
				// Rotation whenever the character is rotated 60 degrees or more away from the AimingRotation. This effectively creates a basic turn in place behavior.
				const FRotator DeltaRot = KML::NormalizedDeltaRotator(CharacterRef->GetActorRotation(), GetAimingRotation());
				if (abs(DeltaRot.Yaw) > 60)
				{
					return KML::GetForwardVector(FRotator(0.0, GetAimingRotation().Yaw, 0.0));
				}
				else
				{
					return CurrentOrientationIntent;
				}
				//GEngine->AddOnScreenDebugMessage(-1, 0.1, FColor::Red, TEXT("Aiming Idle"));
			}
		}
	}
	else if(CollectedMovement == CALS_MovementState::InAir)
	{
		if (CollectedRotationMode == CALS_RotationMode::VelocityDirection)
		{
			//When in the air while NOT in Strafe or Aim, dont change the OrientationIntent
			return CurrentOrientationIntent;
		}
		else
		{
			//When in the air while in Strafe or Aim, use the AimingRotation as the OrientationIntent
			return KML::GetForwardVector(FRotator(0.0, GetAimingRotation().Yaw, 0.0));
		}
	}
	else if (CollectedMovement == CALS_MovementState::Mantling || CollectedMovement == CALS_MovementState::None || CollectedMovement == CALS_MovementState::Ragdoll)
	{
		//When in the Traversing movement mode (active during traversal montages) set the OrientationIntent to be the actors forward vector, meaning it will not try to rotate.
		return CharacterRef->GetActorForwardVector();
	}

	return CharacterRef->GetActorForwardVector();
}


FVector UMovementParamsControlComponent::GetCurrentMovementDirection_Implementation()
{
	//First we cache the Direction Of Movement which is essentially the direction the pawn will be moving in. Since the InAir and Sliding modes have low friction, we use the velocity.
	FVector VelocityDir = FVector(CMC->Velocity.X, CMC->Velocity.Y, 0.0);
	if (VelocityDir.Length() > KINDA_SMALL_NUMBER) VelocityDir.Normalize();

	if (CharacterRef->HasAnyRootMotion())
	{
		return VelocityDir;
	}
	else if (CollectedMovement == CALS_MovementState::InAir || CollectedMovement == CALS_MovementState::Ragdoll)
	{
		return VelocityDir;
	}
	else if (CollectedMovement == CALS_MovementState::Grounded || CollectedMovement == CALS_MovementState::Crawl)
	{
		return GetMovementInput();
	}
	return CharacterRef->GetActorForwardVector();
}


FRotator UMovementParamsControlComponent::GetAimingRotation()
{
	if (!CharacterRef) return FRotator::ZeroRotator;
	return CharacterRef->GetControlRotation();
}


void UMovementParamsControlComponent::UpdateDynamicMovementSettings(CALS_Gait AllowedGait)
{
	UpdateWalkOnStairsAlpha();

	CurrentMovementModel = GetTargetMovementSettings();

	if (!CMC) return;

	CMC->MaxWalkSpeed = CalculateMaxSpeed(AllowedGait);
	CMC->MaxWalkSpeedCrouched = CMC->MaxWalkSpeed;

	float SpeedMap = 1.0;
	if (UseInterpModeToCalculateGaitMap) SpeedMap = SmoothGaitTransition;
	else SpeedMap = GetMappedSpeed(1.0, AllowedGait);

	//GEngine->AddOnScreenDebugMessage(0, 0.2, FColor::Red, FString::SanitizeFloat(SmoothGaitTransition));

	if (CurrentMovementModel.MovementCurve)
	{
		FVector CurveValue = CurrentMovementModel.MovementCurve->GetVectorValue(SpeedMap);

		if (CurrentMovementPreset->UseDynamicParamsForSprintGait && SpeedMap >= 3)
		{
			const float TimeBiasForAcc = KML::MapRangeClamped(SpeedValue2D, CurrentMovementPreset->DynamicRangeInForAcceleration.X, CurrentMovementPreset->DynamicRangeInForAcceleration.Y, 0.0, 1.0);
			CurveValue.X = CurrentMovementModel.MovementCurve->GetVectorValue(SpeedMap + TimeBiasForAcc).X;

			const float TimeBiasForFric = KML::MapRangeClamped(SpeedValue2D, CurrentMovementPreset->DynamicRangeInForFriction.X, CurrentMovementPreset->DynamicRangeInForFriction.Y, 0.0, 1.0);
			CurveValue.Z = CurrentMovementModel.MovementCurve->GetVectorValue(SpeedMap + TimeBiasForFric).Z;
		}

		if (UAnimInstance* AnimInst = CharacterRef->GetMesh()->GetAnimInstance())
		{
			CurveValue.Y = CurveValue.Y + AnimInst->GetCurveValue(ModifyDecelerationCurveName);
			CurveValue.Z = CurveValue.Z + AnimInst->GetCurveValue(ModifyGroundFrictionCurveName);
		}

		CMC->MaxAcceleration = CurveValue.X;
		CMC->BrakingDecelerationWalking = CurveValue.Y;
		CMC->GroundFriction = CurveValue.Z;
	}
}


float UMovementParamsControlComponent::UpdateWalkOnStairsAlpha_Implementation()
{
	return 0.0;
}


bool UMovementParamsControlComponent::SetNewMovementPreset(UCMC_MovementPreset* NewPreset, bool UseTransition, float TransitionDurationScale)
{
	if (!NewPreset) return false;

	UCMC_MovementPreset* LocalPreset = CurrentMovementPreset;
	if (!CurrentMovementPreset) LocalPreset = DefaultMovementPreset;

	PrevMovementPreset = LocalPreset;
	CurrentMovementPreset = NewPreset;

	if (UseTransition && CanUseTransitionsBetweenPresets && IsValid(PrevMovementPreset))
	{
		bTransitionIsRunning = true;
		TransitionBlendAlpha = 0.0;
		CurrentPresetsTransitionDuration = FMath::Clamp<float>(DefaultTransitionsDuration * TransitionDurationScale, 0.05, 10.0);
		CurrentPresetTransitionTimer = CurrentPresetsTransitionDuration;
	}

	return true;
}


bool UMovementParamsControlComponent::SetDefaultMovementPreset(bool UseTransition, float TransitionDurationScale)
{
	if (!DefaultMovementPreset) return false;

	UCMC_MovementPreset* LocalPreset = CurrentMovementPreset;
	if (!CurrentMovementPreset) LocalPreset = DefaultMovementPreset;

	PrevMovementPreset = LocalPreset;
	CurrentMovementPreset = DefaultMovementPreset;

	if (UseTransition && CanUseTransitionsBetweenPresets && IsValid(PrevMovementPreset))
	{
		bTransitionIsRunning = true;
		TransitionBlendAlpha = 0.0;
		CurrentPresetsTransitionDuration = FMath::Clamp<float>(DefaultTransitionsDuration * TransitionDurationScale, 0.05, 10.0);
		CurrentPresetTransitionTimer = CurrentPresetsTransitionDuration;
	}
	return true;
}


UCMC_MovementPreset* UMovementParamsControlComponent::GetCurrentMovementPreset() const
{
	return CurrentMovementPreset;
}


UCMC_MovementPreset* UMovementParamsControlComponent::GetPreviousMovementPreset() const
{
	return PrevMovementPreset;
}


void UMovementParamsControlComponent::GetDesiredMovementsTypeStates(AGLS_WalkingType& OutWalkType, AGLS_RunningType& OutRunType, AGLS_SprintingType& OutSprintType) const
{
	OutWalkType = CurrentMovementModel.MatchingWalkDatabasesToSpeed;
	OutRunType = CurrentMovementModel.MatchingRunDatabasesToSpeed;
	OutSprintType = CurrentMovementModel.MatchingSprintDatabasesToSpeed;
}


UMovementParamsControlComponent* UMovementParamsControlComponent::GryGetMovementControlComponent(UObject* WorldContextObject, ACharacter* Target)
{
	if (Target)
	{
		UMovementParamsControlComponent* TargetComp = Cast< UMovementParamsControlComponent>(Target->GetComponentByClass(UMovementParamsControlComponent::StaticClass()));
		return TargetComp;
	}
	else
	{
		ACharacter* TargetChar = Cast<ACharacter>(WorldContextObject);
		if (TargetChar)
		{
			UMovementParamsControlComponent* TargetComp = Cast< UMovementParamsControlComponent>(TargetChar->GetComponentByClass(UMovementParamsControlComponent::StaticClass()));
			return TargetComp;
		}
		return nullptr;
	}
}


void UMovementParamsControlComponent::GetCollectedStates(CALS_MovementState& ReturnMovementState, CALS_RotationMode& ReturnRotationMode, CALS_Gait& ReturnDesiredGait, CALS_Gait& ReturnCurrentGait)
{
	ReturnMovementState = CollectedMovement;
	ReturnRotationMode = CollectedRotationMode;
	ReturnDesiredGait = CollectedDesiredGait;
	ReturnCurrentGait = CollectedCurrentGait;
}


float UMovementParamsControlComponent::GetOnStairsWalkingAlpha() const
{
	return WalkingOnStairsAlpha;
}


void UMovementParamsControlComponent::RenderDebugInformations(float DeltaX, CALS_Gait FinalGait)
{
#if WITH_EDITOR

	if (!CharacterRef) return;

	const float TextOffet = -7;
	const FVector UpPosition = KML::GetUpVector(CharacterRef->GetControlRotation());
	const FVector TextCenter = CharacterRef->GetActorLocation() + (KML::GetRightVector(CharacterRef->GetControlRotation()) * -90) + (UpPosition * 80);
	
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 0)), "| MOVEMENT CONTROL COMPONENT |", nullptr, FColor::Black, 0.0, true, 0.95f);

	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 1)), "1) WalkSpeed: " + CurrentMovementModel.WalkSpeed.ToCompactString(), nullptr, FColor::Yellow, 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 2)), "2) JogSpeed: " + CurrentMovementModel.RunSpeed.ToCompactString(), nullptr, FColor::Yellow, 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 3)), "3) RunSpeed: " + CurrentMovementModel.SprintSpeed.ToCompactString(), nullptr, FColor::Yellow, 0.0, true, 1);

	FString MM_Database = "Unspecified";
	//CurrentMovementModel.MatchingRunDatabasesToSpeed
	if (FinalGait == CALS_Gait::Walking)
	{ const UEnum* EnumPtr = StaticEnum<AGLS_WalkingType>(); MM_Database = EnumPtr->GetNameStringByValue((int64)CurrentMovementModel.MatchingWalkDatabasesToSpeed); }
	else if (FinalGait == CALS_Gait::Running)
	{ const UEnum* EnumPtr = StaticEnum<AGLS_RunningType>(); MM_Database = EnumPtr->GetNameStringByValue((int64)CurrentMovementModel.MatchingRunDatabasesToSpeed); }
	else 
	{ const UEnum* EnumPtr = StaticEnum<AGLS_SprintingType>(); MM_Database = EnumPtr->GetNameStringByValue((int64)CurrentMovementModel.MatchingSprintDatabasesToSpeed); }

	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 4)), "Current MM Databases Type: " + MM_Database, nullptr, FColor::Magenta, 0.0, true, 1);

	FString TransitionValue = "False";
	if (bTransitionIsRunning) TransitionValue = "TRUE";
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 6)), "Presets Transition Is Running: " + TransitionValue, nullptr, 
		KML::SelectColor(FColor::Red, FColor::Black, bTransitionIsRunning).ToFColor(false), 0.0, true, 1);

	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 7)), "Transition Blend Alpha: " + FString::SanitizeFloat(TransitionBlendAlpha), nullptr,
		KML::SelectColor(FColor::Emerald, FColor::Black, bTransitionIsRunning).ToFColor(false), 0.0, true, 1);

	if (!CMC) return;
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 9)), "CMC Max Acceleration: " + FString::SanitizeFloat(CMC->MaxAcceleration), nullptr, FColor::Cyan, 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 10)), "CMC Walk Deceleration: " + FString::SanitizeFloat(CMC->BrakingDecelerationWalking), nullptr, FColor::Cyan, 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 11)), "CMC Max Walk Speed: " + FString::SanitizeFloat(CMC->MaxWalkSpeed), nullptr, FColor::Cyan, 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 12)), "CMC Ground Friction: " + FString::SanitizeFloat(CMC->GroundFriction), nullptr, FColor(0, 100, 150), 0.0, true, 1);

	const FString DesiredGaitName = StaticEnum<CALS_Gait>()->GetNameStringByValue((int64)CollectedDesiredGait);
	const FString AllowedGaitName = StaticEnum<CALS_Gait>()->GetNameStringByValue((int64)ActualGaitState);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 14)), "Collected Desired Gait: " + DesiredGaitName, nullptr, FColor(150,150,150,255), 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 15)), "Actual Gait: " + AllowedGaitName, nullptr, FColor(150, 150, 150, 255), 0.0, true, 1);

	const FString DirectionStateName = StaticEnum<AGLS_MovementDirectionState>()->GetNameStringByValue((int64)CurrentMovementDirection);
	const FString DirectionStateName2 = StaticEnum<AGLS_MovementDirectionState>()->GetNameStringByValue((int64)PreviousMovementDirection);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 17)), "Current Direction State: " + DirectionStateName, nullptr, FColor(0, 100, 200, 255), 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 18)), "Previous Direction State: " + DirectionStateName2, nullptr, FColor(0, 85, 170, 255), 0.0, true, 1);
	DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 19)), "Orientation Intent: " + OrientationIntent.ToCompactString(), nullptr, FColor(60, 20, 150, 255), 0.0, true, 1);

	if (CalculateSimpleOnStairsValue)
	{
		DrawDebugString(GetWorld(), TextCenter + (UpPosition * (TextOffet * 21)), "Walking Stairs Alpha: " + FString::SanitizeFloat(WalkingOnStairsAlpha), nullptr, FColor(150, 200, 0, 255), 0.0, true, 1);
	}
#endif
}


#undef STATE_LOOKING
#undef STATE_FACING
#undef STATE_AIMING
#undef S_STANDING
#undef S_CROUCHING
#undef KML