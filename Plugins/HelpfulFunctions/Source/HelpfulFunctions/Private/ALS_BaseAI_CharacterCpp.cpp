


#include "ALS_BaseAI_CharacterCpp.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Animation/AnimInstance.h"



// Sets default values
AALS_BaseAI_CharacterCpp::AALS_BaseAI_CharacterCpp()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AALS_BaseAI_CharacterCpp::BeginPlay()
{
	Super::BeginPlay();
	Self = this;
}


// Called every frame
void AALS_BaseAI_CharacterCpp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// ------------ PODSTAWOWA DEFINICJA ROTACJI --------------------
//                 FRotator(Pitch,Yaw,Roll)
// --------------------------------------------------------------


/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
void AALS_BaseAI_CharacterCpp::SmoothedCharRotation(FRotator Target, float TargetInterpSpeedConst, float ActorInterpSpeedSmooth, bool UpdateControl)
{
	TargetRotationC = UKismetMathLibrary::RInterpTo_Constant(TargetRotationC, Target, UGameplayStatics::GetWorldDeltaSeconds(Self), TargetInterpSpeedConst);
	const FQuat NewActorRot = UKismetMathLibrary::Conv_RotatorToQuaternion(UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRotationC, UGameplayStatics::GetWorldDeltaSeconds(Self), ActorInterpSpeedSmooth));
	SetActorRotation(NewActorRot, ETeleportType::None);
}


/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
void AALS_BaseAI_CharacterCpp::LimitRotationFast(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
	if (UKismetMathLibrary::InRange_FloatFloat(DeltaRot.Yaw, AimYawMin, AimYawMax, true, true) == false)
	{
		FRotator TargetRot;
		if (DeltaRot.Yaw > 0)
		{
			TargetRot = FRotator(0, GetControlRotation().Yaw + AimYawMin, 0);
		}
		else
		{
			TargetRot = FRotator(0, GetControlRotation().Yaw + AimYawMax, 0);
		}
		SmoothedCharRotation(TargetRot, 0, InterpSpeed, true);
		return;
	}
	return;
}


/*𝐌𝐎𝐕𝐄𝐌𝐄𝐍𝐓 𝐑𝐎𝐓𝐀𝐓𝐈𝐎𝐍 𝐂𝐎𝐍𝐓𝐑𝐎𝐋*/
bool AALS_BaseAI_CharacterCpp::CanUpdateMovingRotation()
{
	bool CanByMove = IsMovingC && HasMovementInputC;
	CanByMove = CanByMove || SpeedC > 150;
	if (CanByMove == true && !Self->HasAnyRootMotion() == true)
	{
		return true;
	}
	return false;
}



/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//void AALS_BaseAI_CharacterCpp::CalculateGroundedRotation()
//{
//}

/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//void AALS_BaseAI_CharacterCpp::UpdateMovementSettings(CALS_Gait AllowedGait, FCALSMovementSettings CurrentMovement)
//{
//	FCALSMovementSettings LocMoveSettings = CurrentMovement;
//	CurrentMovementSettingsC = LocMoveSettings;
//	if (AllowedGait == CALS_Gait::Running)
//	{
//		Self->GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettingsC.RunSpeed;
//	}
//	else if (AllowedGait == CALS_Gait::Sprinting)
//	{
//		Self->GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettingsC.SprintSpeed;
//	}
//	else
//	{
//		Self->GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettingsC.WalkSpeed;
//	}
//
//	Self->GetCharacterMovement()->MaxWalkSpeedCrouched = Self->GetCharacterMovement()->MaxWalkSpeed;
//	FVector CurveValue = CurrentMovementSettingsC.MovementCurve->GetVectorValue(GetMappedSpeedFast());
//	Self->GetCharacterMovement()->MaxAcceleration = CurveValue.X;
//	Self->GetCharacterMovement()->BrakingDecelerationWalking = CurveValue.Y;
//	Self->GetCharacterMovement()->GroundFriction = CurveValue.Z;
//}

/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//float AALS_BaseAI_CharacterCpp::GetMappedSpeedFast()
//{
//	float LocWalkSpeed = CurrentMovementSettingsC.WalkSpeed;
//	float LocRunSpeed = CurrentMovementSettingsC.RunSpeed;
//	float LocSprintSpeed = CurrentMovementSettingsC.SprintSpeed;
//	if (SpeedC > LocRunSpeed)
//	{
//		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocRunSpeed, LocSprintSpeed, 2, 3);
//	}
//	else if (SpeedC > LocWalkSpeed)
//	{
//		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocWalkSpeed, LocRunSpeed, 1, 2);
//	}
//	else
//	{
//		return UKismetMathLibrary::MapRangeClamped(SpeedC, 0, LocWalkSpeed, 0, 1);
//	}
//}

/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//bool AALS_BaseAI_CharacterCpp::CalcCanSprint_Implementation()
//{
//	if (HasMovementInputC == false)
//	{
//		return false;
//	}
//	if (RotationModeC == CALS_RotationMode::Aiming)
//	{
//		return false;
//	}
//	else if (RotationModeC == CALS_RotationMode::VelocityDirection)
//	{
//		return MovementInputAmountC > 0.9;
//	}
//	else
//	{
//		FRotator AccRotation = UKismetMathLibrary::MakeRotFromX(Self->GetCharacterMovement()->GetCurrentAcceleration());
//		AccRotation = UKismetMathLibrary::NormalizedDeltaRotator(AccRotation, Self->GetControlRotation());
//		AccRotation = FRotator(0, abs(AccRotation.Yaw), 0);
//		return MovementInputAmountC > 0.9 && AccRotation.Yaw < 50;
//	}
//}

/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//CALS_Gait AALS_BaseAI_CharacterCpp::GetAllowedGaitFast_Implementation()
//{
//	if (StanceC == CALS_Stance::Standing)
//	{
//		if (RotationModeC == CALS_RotationMode::LookingDirection || RotationModeC == CALS_RotationMode::VelocityDirection)
//		{
//			// For Normal Rotation
//			if (DesiredGaitC == CALS_Gait::Walking)
//			{
//				return CALS_Gait::Walking;
//			}
//			else if (DesiredGaitC == CALS_Gait::Running)
//			{
//				return CALS_Gait::Running;
//			}
//			else if (CalcCanSprint() == true)
//			{
//				return CALS_Gait::Sprinting;
//			}
//			else
//			{
//				return CALS_Gait::Running;
//			}
//		}
//		else
//		{
//			// For Aiming
//			if (DesiredGaitC == CALS_Gait::Walking)
//			{
//				return CALS_Gait::Walking;
//			}
//			else
//			{
//				return CALS_Gait::Running;
//			}
//		}
//	}
//	else
//	{
//		// For Crouch
//		if (DesiredGaitC == CALS_Gait::Walking)
//		{
//			return CALS_Gait::Walking;
//		}
//		else
//		{
//			return CALS_Gait::Running;
//		}
//	}
//}

/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
//CALS_Gait AALS_BaseAI_CharacterCpp::GetActualGaitFast_Implementation(CALS_Gait AllowedGait)
//{
//	float LocWalkSpeed = CurrentMovementSettingsC.WalkSpeed;
//	float LocRunSpeed = CurrentMovementSettingsC.RunSpeed;
//	float LocSprintSpeed = CurrentMovementSettingsC.SprintSpeed;
//	if (SpeedC >= LocRunSpeed + 10)
//	{
//		if (AllowedGait == CALS_Gait::Sprinting)
//		{
//			return CALS_Gait::Sprinting;
//		}
//		else
//		{
//			return CALS_Gait::Running;
//		}
//	}
//	else if (SpeedC >= LocWalkSpeed + 10)
//	{
//		return CALS_Gait::Running;
//	}
//	else
//	{
//		return CALS_Gait::Walking;
//	}
//}



// Called to bind functionality to input
void AALS_BaseAI_CharacterCpp::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}




void AALS_BaseAI_CharacterCpp::UpdateEssentialMovementValues(float InDelta)
{
	float SafeDelta = InDelta;
	if (SafeDelta == 0.0) { SafeDelta = 0.01f; }

	AccelerationC = (GetVelocity() - PrevVelocityC) / SafeDelta;
	//MovementSpeedDifferenceC = FVector(GetVelocity().X, GetVelocity().Y, 0.0).Length() - FVector(PreviousVelocityC.X, PreviousVelocityC.Y, 0.0).Length();
	SpeedC = FVector(GetVelocity().X, GetVelocity().Y, 0.0).Length();
	if (SpeedC > 1.0) { IsMovingC = true; }
	else { IsMovingC = false; }

	FVector AccelerationXY = this->GetCharacterMovement()->GetCurrentAcceleration();
	AccelerationXY.Z = 0.0;
	MovementInputAmountC = AccelerationXY.Length() / this->GetCharacterMovement()->GetMaxAcceleration();
	if (MovementInputAmountC > 0.0) { HasMovementInputC = true; }
	else { HasMovementInputC = false; }

	AimYawRateC = abs((GetControlRotation().Yaw - PrevAimYawC) / SafeDelta);

	//Update Base Rotation Values
	if (HasMovementInputC == true)
	{
		LastMovementInputRotationC = UKismetMathLibrary::MakeRotFromX(this->GetCharacterMovement()->GetCurrentAcceleration());
	}
	if (IsMovingC == true)
	{
		LastVelocityRotationC = UKismetMathLibrary::MakeRotFromX(GetVelocity());
	}

	//Update Cached Variables
	PrevVelocityC = GetVelocity();
	PrevAimYawC = GetControlRotation().Yaw;
}


void AALS_BaseAI_CharacterCpp::UpdateGroundedRotation_Implementation()
{
	if (LockRotationUpdatingC == true)
	{
		TargetRotationC = Self->GetActorRotation();
		return;
	}

	if (MovementActionC == CALS_MovementAction::None)
	{
		// sprawdz czy postac sie porusza
		if (CanUpdateMovingRotation() == true)
		{
			//jesli sie porusza wykonaj to:
			if (RotationModeC == CALS_RotationMode::VelocityDirection)
			{
				// IF Moving==True [AND] RotationMode==Velocity
				SmoothedCharRotation(FRotator(0, LastVelocityRotationC.Yaw, 0), 800, GetGroundedRotationUpdateRate(), true);
			}
			else if (RotationModeC == CALS_RotationMode::LookingDirection)
			{
				if (GaitC == CALS_Gait::Walking || GaitC == CALS_Gait::Running)
				{
					// IF Moving==True [AND] RotationMode==Looking [AND] Gait==Walking Or Running
					SmoothedCharRotation(FRotator(0, Self->GetControlRotation().Yaw, 0), 400, GetGroundedRotationUpdateRate(), true);
				}
				else
				{
					// IF Moving==True [AND] RotationMode==Looking [AND] Gait==Sprint
					SmoothedCharRotation(FRotator(0, LastVelocityRotationC.Yaw, 0), 500, GetGroundedRotationUpdateRate(), true);
				}
			}
			else if (RotationModeC == CALS_RotationMode::Aiming)
			{
				SmoothedCharRotation(FRotator(0, Self->GetControlRotation().Yaw, 0), 1200, GetGroundedRotationUpdateRate(), true);
			}
		}
		else
		{
			if (Self->GetMesh()->GetAnimInstance())
			{
				FRotator AddRotation;
				AActor* CharActor = (AActor*)Self;
				AddRotation = FRotator(0, Self->GetMesh()->GetAnimInstance()->GetCurveValue(FName(TEXT("RotationAmount"))) *
					UKismetMathLibrary::SafeDivide(UGameplayStatics::GetWorldDeltaSeconds(Self), 0.0333f), 0);
				//jesli sie NIE porusza to wykonaj to:
				if (RotationModeC == CALS_RotationMode::Aiming)
				{
					//IF Moving==FALSE [AND] RotationMode==Aiming (Limit Rotation)
					LimitRotationFast(-100, 100, 20);
					if (abs(Self->GetMesh()->GetAnimInstance()->GetCurveValue(FName(TEXT("RotationAmount")))) > 0.001)
					{
						CharActor->AddActorWorldRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(AddRotation), false);
						TargetRotationC = Self->GetActorRotation();
					}
				}
				else
				{
					if (abs(Self->GetMesh()->GetAnimInstance()->GetCurveValue(FName(TEXT("RotationAmount")))) > 0.001)
					{
						CharActor->AddActorWorldRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(AddRotation), false);
						TargetRotationC = Self->GetActorRotation();
						//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, AddRotation.ToString());
					}
				}
			}
		}
	}
	else if (MovementActionC == CALS_MovementAction::Rolling)
	{
		if (HasMovementInputC == true)
		{
			SmoothedCharRotation(FRotator(0, LastMovementInputRotationC.Yaw, 0), 0, 2, true);
		}
	}
}


void AALS_BaseAI_CharacterCpp::UpdateInAirRotation_Implementation()
{
	if (RotationModeC == CALS_RotationMode::VelocityDirection)
	{
		SmoothedCharRotation(LastVelocityRotationC, 50, 10, true);
	}
	else
	{
		SmoothedCharRotation(LastMovementInputRotationC, 50, 10, true);
	}
}


float AALS_BaseAI_CharacterCpp::GetGroundedRotationUpdateRate()
{
	if (IsValid(CurrentMovementSettingsC.RotationRateCurve) == true)
	{
		float CurveValue = CurrentMovementSettingsC.RotationRateCurve->GetFloatValue(GetMappedSpeedFast());
		CurveValue = CurveValue * UKismetMathLibrary::MapRangeClamped(AimYawRateC, 0, 300, 1, 3);
		return CurveValue;
	}
	return 1;
}


float AALS_BaseAI_CharacterCpp::GetMappedSpeedFast()
{
	float LocWalkSpeed = CurrentMovementSettingsC.WalkSpeed;
	float LocRunSpeed = CurrentMovementSettingsC.RunSpeed;
	float LocSprintSpeed = CurrentMovementSettingsC.SprintSpeed;
	if (SpeedC > LocRunSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocRunSpeed, LocSprintSpeed, 2, 3);
	}
	else if (SpeedC > LocWalkSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocWalkSpeed, LocRunSpeed, 1, 2);
	}
	else
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, 0, LocWalkSpeed, 0, 1);
	}
}