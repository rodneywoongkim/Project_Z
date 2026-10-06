// Copyright Jakub W, All Rights Reserved. 

#include "Cpp_DynamicClimbingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Curves/CurveVector.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "Animation/AnimInstance.h"
#include <tuple>
#include <cmath>

#define SAFEDELTATIME GetSafeDeltaTime()
#define KSL UKismetSystemLibrary
#define KML UKismetMathLibrary
#define CLASSTOIGNORESAFE ClassToIgnoreSafe

// Sets default values for this component's properties
UCpp_DynamicClimbingComponent::UCpp_DynamicClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UCpp_DynamicClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	CharacterC = Cast<ACharacter>(this->GetOwner());
	if (IsValid(CharacterC) == true)
	{
		DefCapsuleSizeC = FVector2D(CharacterC->GetCapsuleComponent()->GetScaledCapsuleRadius(), CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}

}

// Called every frame
void UCpp_DynamicClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	dt = DeltaTime;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UCpp_DynamicClimbingComponent::GetSafeDeltaTime()
{
	if (dt == KINDA_SMALL_NUMBER)
	{
		return UGameplayStatics::GetWorldDeltaSeconds(this);
	}
	else { return dt; }
}


bool UCpp_DynamicClimbingComponent::GetHitIsBrushComponent(UPrimitiveComponent* InHitComponent)
{
	if (!InHitComponent) return false;
	if (InHitComponent->StaticClass() == GeometryBrushCompoentClass)
	{
		return true;
	}
	return false;
}

bool UCpp_DynamicClimbingComponent::ClassToIgnoreSafe(FHitResult InHit, TArray<UClass*> ToIgnore, UPrimitiveComponent* HitComponent)
{
	if (HitComponent)
	{
		bool Brush = GetHitIsBrushComponent(HitComponent);
		if (Brush) return true;
	}
	if (!InHit.GetActor()) return false;
	return !ToIgnore.Contains(InHit.GetActor()->StaticClass());
}


#pragma region Functions to override in Blueprint (C++ Code is empty)

bool UCpp_DynamicClimbingComponent::CheckNormalForPointC_Implementation(FExposedCableParticle& InParticle)
{
	if (HookActorC && HookActorC->Implements<UALS_HookActorInterface>())
	{
		IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(HookActorC); //Get Interface

		float AngleValue = 0.0;
		HookInterface->Execute_HAFSI_Get_ParticleNormalValidation(HookActorC, AngleValue);

		if (AngleValue <= 0.0) { return true; }

		EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
		if (SwingDebugIndexC == 1) TraceType = EDrawDebugTrace::ForOneFrame;
		if (SwingDebugIndexC == 2) TraceType = EDrawDebugTrace::ForDuration;
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		TEnumAsByte<ETraceTypeQuery> Channel = ETraceTypeQuery::TraceTypeQuery1;

		FHitResult R1, R2;
		const bool HitValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, InParticle.Position, InParticle.OldPosition + FVector(0, 0, 0.2f), CableSimC->CableWidth * 1.1f, 
			Channel, false, ToIgnore, TraceType, R1, true, FLinearColor::Black, FLinearColor::White, 0.2f);
		if (HitValid == false) { return true; }

		const bool HitValid2 = UKismetSystemLibrary::SphereTraceSingle(CharacterC, R1.ImpactPoint + (R1.Normal * -6.0) + FVector(0, 0, 12), R1.ImpactPoint + (R1.Normal * -6.0) + FVector(0, 0, -12), 
			CableSimC->CableWidth * 1.1, Channel, false, ToIgnore, TraceType, R2, true, FLinearColor::Gray, FLinearColor::Blue, 0.3f);
		if (HitValid2 == false) { return true; }

		const float Dot = UKismetMathLibrary::Dot_VectorVector(R2.Normal, FVector(0, 0, 1));

		return Dot > AngleValue;
	}
	return true;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::RopeHookedConditionC_Implementation()
{
	return true;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::DetachRopeOrEndSwingC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::StartedZiplineC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::StartedPickaxeClimbC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::FinishRopeSwingC_Implementation()
{
	return false;
}

#pragma endregion


#pragma region DYNAMIC LEDGE CONSTRUCTION CORE

//MAIN LEDGE CREATION FUNCTION (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::TryCreateLedgeStructureC_Implementation(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, 
	FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid, FVector TraceOrigin, FVector TraceDirection, float Z_Offset, float ForwardTraceLength, bool UseWallCondition)
{
	if (LedgeSolverAlgoritmType == AGLS_DynamicLedgeSolverType::ComplexTracesMode)
	{
		//int QueriesResult = 0;
		Valid = TryCreateLedgeUsingComplexTracesMethod(
			LeftPoint, 
			RightPoint, 
			OriginPoint,
			SingleLedgeFindTotalTracesResult,
			ClassToIgnoreByLedgeC, //Inputs begin
			CharacterC, 
			TraceOrigin + FVector(0,0, Z_Offset),
			TraceDirection, 
			DefCapsuleSizeC.X, 
			DefCapsuleSizeC.Y, 
			0.0f, 
			(CapsuleUpOffsetC + ComplexTracesLedgeSolverConfig.CapsuleFreeSpaceCheckOriginOffsetUp) * -1.0f,
			ForClimbingChannelC, 
			ComplexTracesLedgeSolverConfig,
			DebugTraceIndexC > 0,
			DebugTraceIndexC > 0
		);
		FirstNotValid = Valid;

		return;
	}
	else if(LedgeSolverAlgoritmType == AGLS_DynamicLedgeSolverType::Default)
	{
		TryCreateLedgeUsingDefaultSolver(
			Valid, 
			LeftPoint, 
			RightPoint, 
			OriginPoint, 
			FirstNotValid, 
			TraceOrigin, //Inputs begin
			TraceDirection, 
			FVector2D(0.0f, 0.0f),
			Z_Offset,
			DefaultLedgeSolverConfig,
			UseWallCondition
		);
		return;
	}
	else
	{
		FAGLSLedgeResult AstraLedgeResult;
		Valid = UAGLS_AdvancedLedgesFinder::TryFindLedgeForClimbing(CharacterC, TraceOrigin, TraceDirection, FVector2D(0.9f, 0.9f), 1.0f, CapsuleUpOffsetC, AstraLedgeSolverConfig, AstraLedgeResult);

		FCMC_SingleClimbPointC AstraPointLeft;
		AstraPointLeft.Location = AstraLedgeResult.LeftTransform.GetLocation();
		AstraPointLeft.Normal = KML::GetForwardVector(AstraLedgeResult.LeftTransform.Rotator());
		AstraPointLeft.ValidPoint = true;
		AstraPointLeft.Component = AstraLedgeResult.SurfaceComponent;

		FCMC_SingleClimbPointC AstraPointRight;
		AstraPointRight.Location = AstraLedgeResult.RightTransform.GetLocation();
		AstraPointRight.Normal = KML::GetForwardVector(AstraLedgeResult.RightTransform.Rotator());
		AstraPointRight.ValidPoint = true;
		AstraPointRight.Component = AstraLedgeResult.SurfaceComponent;
		
		OriginPoint.Location = KML::VLerp(AstraPointLeft.Location, AstraPointRight.Location, 0.5f);
		OriginPoint.Normal = KML::Vector_SlerpNormals(AstraPointLeft.Normal, AstraPointRight.Normal, 0.5f);
		OriginPoint.ValidPoint = IsValid(AstraLedgeResult.SurfaceComponent);
		OriginPoint.Component = AstraLedgeResult.SurfaceComponent;
		FirstNotValid = false;
		LeftPoint = AstraPointLeft;
		RightPoint = AstraPointRight;
		return;
	}
}


void UCpp_DynamicClimbingComponent::TryCreateLedgeUsingDefaultSolver(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid, 
	FVector TraceOrigin, FVector TraceDirection, FVector2D AxisNormals, float TracingOriginOffsetZ, FAGLS_LedgeFinderConfig_DefaultSolver SolverSettings, bool UseWallCondition)
{
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	bool Continue = false;
	FCMC_SingleClimbPointC RightLedgeStruct = {};
	FCMC_SingleClimbPointC LeftLedgeStruct = {};
	FCMC_SingleClimbPointC CenterLedgeStruct = {};
	bool RightLedgeValid = false;
	bool LeftLedgeValid = false;
	int TraceDebugIndex = DebugTraceIndexC;
	if (DebugTraceIndexC > 0) { TraceType = EDrawDebugTrace::ForOneFrame; }

	float TraceForward = SolverSettings.ForwardTraceLength;
	if (CurrentModifyVolume)
	{
		if (CurrentModifyVolume->LedgeSearchForwardRange >= 0.0 && CharacterC->GetVelocity().Z > CurrentModifyVolume->VerticalVelocityTollerance)
		{
			TraceForward = CurrentModifyVolume->LedgeSearchForwardRange;
		}
	}

	if (UseWallCondition == true)
	{
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		FHitResult WalleHitResult;
		const bool WallHitValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TraceOrigin, TraceOrigin + (TraceDirection * TraceForward),
			18.0, Channel, false, ToIgnore, TraceType, WalleHitResult, true);
		Continue = WallHitValid;
	}
	else { Continue = true; }

	if (Continue == false) { Valid = false; FirstNotValid = true; return; }

	if (CurrentModifyVolume)
	{
		int LedgeIterationsCount = CurrentModifyVolume->LedgeUpOffsetIterations - 1;
		if (CharacterC->GetVelocity().Z <= CurrentModifyVolume->VerticalVelocityTollerance) { LedgeIterationsCount = 0; }

		for (int i = 0; i <= LedgeIterationsCount; i++)
		{
			float NewOffset = TracingOriginOffsetZ + ((float)i * CurrentModifyVolume->UpOffsetDeltaValue);

			// RIGHT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
			UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, RightLedgeValid, RightLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, NewOffset,
				TraceForward, SolverSettings.RightOffsetScale, false, SolverSettings.InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2f);
			if (RightLedgeValid == false) { Valid = false; FirstNotValid = false; }

			// LEFT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
			UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, LeftLedgeValid, LeftLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, NewOffset,
				TraceForward, SolverSettings.RightOffsetScale, true, SolverSettings.InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2f);
			if (LeftLedgeValid == false) { Valid = false; FirstNotValid = false; }

			if (RightLedgeValid && LeftLedgeValid) { break; }
		}
	}
	else
	{
		// RIGHT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
		UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, RightLedgeValid, RightLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, TracingOriginOffsetZ,
			SolverSettings.ForwardTraceLength, SolverSettings.RightOffsetScale, false, SolverSettings.InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2f);
		if (RightLedgeValid == false)
		{ Valid = false; FirstNotValid = false; return; }

		// LEFT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
		UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, LeftLedgeValid, LeftLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, TracingOriginOffsetZ,
			SolverSettings.ForwardTraceLength, SolverSettings.RightOffsetScale, true, SolverSettings.InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2f);
		if (LeftLedgeValid == false)
		{ Valid = false; FirstNotValid = false; return; }
	}

	if (SolverSettings.bUseAdvancedCapsuleFreeSpaceFinder)
	{
		FTransform LeftLedgeTransform = FTransform(KML::MakeRotFromX(LeftLedgeStruct.Normal), LeftLedgeStruct.Location);
		FTransform RightLedgeTransform = FTransform(KML::MakeRotFromX(RightLedgeStruct.Normal), RightLedgeStruct.Location);
		FTransform LedgeCenterTransform; UPrimitiveComponent* LedgeCenterComponent;
		int OutQueries = 0;
		FAGLS_LedgeFinderConfig_ComplexTracigMethod CapFreeSpaceFindSettings;
		CapFreeSpaceFindSettings.CapsuleFreeSpaceMaxTopReduction = SolverSettings.CapsuleFreeSpaceMaxTopReduction;
		CapFreeSpaceFindSettings.CapsuleFreeSpaceMaxBottomReduction = SolverSettings.CapsuleFreeSpaceMaxBottomReduction;
		CapFreeSpaceFindSettings.MinValidCapsuleOutHalfHeight = SolverSettings.MinValidCapsuleOutHalfHeight;
		CapFreeSpaceFindSettings.MinValidCapsuleOutRadius = SolverSettings.MinValidCapsuleOutRadius;
		CapFreeSpaceFindSettings.MaxValidCapsuleReducedSize = SolverSettings.MaxValidCapsuleReducedSize;
		CapFreeSpaceFindSettings.MaxSingleCapsuleFreeSpaceTracesQueries = SolverSettings.MaxSingleCapsuleFreeSpaceTracesQueries;

		const bool CapValid = VerifyLedgeGeneratedUsingComplexTraces
		(
			LeftLedgeTransform, 
			RightLedgeTransform, 
			OutQueries, 
			LeftLedgeTransform, 
			RightLedgeTransform, 
			LedgeCenterTransform, 
			LedgeCenterTransform, 
			LedgeCenterComponent, 
			ForClimbingChannelC, 
			DefCapsuleSizeC.X, 
			DefCapsuleSizeC.Y, 
			0, 
			CapsuleUpOffsetC * -1.0f,
			CapFreeSpaceFindSettings,
			DebugTraceIndexC,
			DebugTraceIndexC
		);
		if(!CapValid) { Valid = false; FirstNotValid = false; return; }
	}
	else
	{
		if (UHelpfulFunctionsBPLibrary::ClimbingLedgeValidP1(CharacterC, true, LeftLedgeStruct, RightLedgeStruct, ForClimbingChannelC, 25.0) == false
			|| LedgeValidationPart2C(true, LeftLedgeStruct, RightLedgeStruct, 14.0, 0.4f, -50.0, DefCapsuleSizeC) == false)
		{ Valid = false; FirstNotValid = false; return; }
	}

	RightLedgeStruct.Component = LeftLedgeStruct.Component;
	CenterLedgeStruct = LeftLedgeStruct;
	CenterLedgeStruct.Location = UKismetMathLibrary::VLerp(LeftLedgeStruct.Location, RightLedgeStruct.Location, 0.5f);
	CenterLedgeStruct.Normal = UKismetMathLibrary::VLerp(LeftLedgeStruct.Normal, RightLedgeStruct.Normal, 0.5f);
	//SET RETURN VALUES:
	Valid = LeftLedgeValid;
	LeftPoint = LeftLedgeStruct;
	RightPoint = RightLedgeStruct;
	FirstNotValid = false;
	OriginPoint = CenterLedgeStruct;
	return;
}


//MAKE SURE THE DETECTED LEDGE IS VALID
bool UCpp_DynamicClimbingComponent::LedgeValidationPart2C(bool Valid, FCMC_SingleClimbPointC LeftStruct, FCMC_SingleClimbPointC RightStruct,
	float MinDistanceBetweenPoints, float RotationTollerance, float CapsuleUpOffset, FVector2D CapsuleChecking)
{
	if (Valid == false)
	{
		return false;
	}

	FVector TraceStart = FVector(0, 0, 0);
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;

	if (LeftStruct.Location != RightStruct.Location && LeftStruct.Location != FVector(0, 0, 0)
		&& RightStruct.Location != FVector(0, 0, 0)
		&& UKismetMathLibrary::Vector_Distance(LeftStruct.Location, RightStruct.Location) > MinDistanceBetweenPoints
		&& UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(LeftStruct.Normal),
			UHelpfulFunctionsBPLibrary::NormalToVector(RightStruct.Normal)) > RotationTollerance)
	{
		// SelectVector(Wartosc jezeli Prawda, Wartosc jezeli Falsz, Warunek Bool)
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		FHitResult CapsuleHitResult;

		TraceStart = UKismetMathLibrary::VLerp(LeftStruct.Location, RightStruct.Location, 0.5) +
			(UHelpfulFunctionsBPLibrary::NormalToVector(UKismetMathLibrary::VLerp(LeftStruct.Normal, RightStruct.Normal, 0.5)) * CapsuleChecking.X);
		TraceStart = TraceStart + FVector(0, 0, CapsuleUpOffset);

		const bool HitValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TraceStart, TraceStart, CapsuleChecking.X * 0.6,
			UKismetMathLibrary::SelectFloat(NarrowFloorCapRadiusC * 0.8, CapsuleChecking.Y * 0.7, StartNarrowFloorMovementC), Channel, false, ToIgnore, TraceType, CapsuleHitResult, true);
		if (HitValid == false)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

#pragma endregion


#pragma region UNSORTED FUNCTIONS

//GET CHARACTER AXIS (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::GetCharacterAxisC_Implementation(float& Forward, float& Right)
{
	Forward = AxisValuesC.X;
	Right = AxisValuesC.Y;
}

// CHECK CAN START CORNER (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::CheckCanStartCornerC_Implementation(bool& DetectedCorner, bool& OuterType, FCMC_LedgeC& TargetLedgeStruct, bool Valid, bool InputLock)
{
	const bool MainCondition = AxisValuesC.X == 0.0 && AxisValuesC.Y != 0.0 && StartNarrowFloorMovementC == false && ActionC == CMC_ActionTypeC::None;
	if (MainCondition == false || InputLock == true)
	{ DetectedCorner = false; OuterType = true; return; }

	//Main Local Variables
	FCMC_LedgeC ReturnStruct = {};
	FCMC_LedgeC LWS = {};
	FCALS_ComponentAndTransform T = {};
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	float OffsetF = 5.0;
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	//Step 1) Convert Local Ledge Structure To Global Space
	T.Component = LedgePointsLS_C.Component;
	LWS.Component = LedgePointsLS_C.Component;
	T.Transform = LedgePointsLS_C.LeftPoint;
	LWS.LeftPoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;
	T.Transform = LedgePointsLS_C.RightPoint;
	LWS.RightPoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;
	T.Transform = LedgePointsLS_C.Origin;
	LWS.Origin = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;

	//Step 2) Run First Trace
	FVector TOrigin = CharacterC->GetActorLocation() + FVector(0, 0, 16) + (CharacterC->GetActorForwardVector() * 0.5);
	FHitResult TResult;
	const bool TValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (CharacterC->GetActorRightVector() * AxisValuesC.Y * 1), 
	TOrigin + (CharacterC->GetActorRightVector() * AxisValuesC.Y * 50.0), 18, 38, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), 
	false, ToIgnore, TraceType, TResult, true, FLinearColor::Blue, FLinearColor::White, 0.1f);

	//Step 3) Select Finding Type - Outer/Inner
	FHitResult SecondResult;
	bool SecondValid = false;
	bool LedgeValid = false;
	if (!TValid == true)
	{
		for (int i = 0; i <= 2; i++)
		{
			TOrigin = UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).GetLocation() + 
			(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * OffsetF);
			TOrigin = TOrigin + FVector(0, 0, -6);
			OffsetF = OffsetF + 10.0;
			SecondValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint,
			LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * 14.0 * AxisValuesC.Y), TOrigin + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint,
			LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * -2.0 * AxisValuesC.Y), 12, 25, Channel, false, ToIgnore, TraceType, SecondResult, true);
			if (SecondValid == true)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, TOrigin.ToString());
				if (abs(UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()))) < 0.45 
				&& UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()),CharacterC->GetActorForwardVector()))<20.0)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TOrigin.ToString());
					TOrigin = FVector(SecondResult.ImpactPoint.X, SecondResult.ImpactPoint.Y, UKismetMathLibrary::Lerp(SecondResult.ImpactPoint.Z, TOrigin.Z + 6.0, 0.5));
					TOrigin = TOrigin + (UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal) * -20.0) + FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(6.0, 12.0));
					//Create Result Structures
					FCMC_SingleClimbPointC LP, RP, OP;
					//Execute Ledge Finding Function
					TryCreateLedgeStructureC_Implementation(LedgeValid, LP, RP, OP, SecondValid, TOrigin, UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), 0, 45, false);

					if (LedgeValid == true)
					{
						ReturnStruct.LeftPoint = ConvertLedgeStructToWS(LP).Transform;
						ReturnStruct.RightPoint = ConvertLedgeStructToWS(RP).Transform;
						ReturnStruct.Origin = ConvertLedgeStructToWS(OP).Transform;
						ReturnStruct.Component = OP.Component;
						DetectedCorner = IsValid(ReturnStruct.Component);
						OuterType = true;
						TargetLedgeStruct = ReturnStruct;
						return;
					}
				}
			}
		}
		DetectedCorner = false; OuterType = true; return;
	}
	else
	{
		OffsetF = -30;
		for (int i = 0; i <= 2; i++)
		{
			TOrigin = UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).GetLocation() +
				(UKismetMathLibrary::GetForwardVector(CharacterC->GetActorRotation()) * OffsetF);
			TOrigin = TOrigin - FVector(0, 0, 6);
			OffsetF = OffsetF - 10.0;
			SecondValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (UKismetMathLibrary::GetRightVector(CharacterC->GetActorRotation()) * 1 * AxisValuesC.Y), 
			TOrigin + (UKismetMathLibrary::GetRightVector(CharacterC->GetActorRotation()) * 40 * AxisValuesC.Y), 12, 25, Channel, false, ToIgnore, TraceType, SecondResult, true);
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, TOrigin.ToString());
			if (SecondValid == true)
			{
				if (abs(UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()))) <= 1
					&& UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()), CharacterC->GetActorForwardVector())) < 20.0)
				{
					TOrigin = FVector(SecondResult.ImpactPoint.X, SecondResult.ImpactPoint.Y, UKismetMathLibrary::Lerp(SecondResult.ImpactPoint.Z, TOrigin.Z + 6.0, 0.5));
					TOrigin = TOrigin + (UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal) * -20.0) + FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(6.0, 12.0));
					//Create Result Structures
					FCMC_SingleClimbPointC LP, RP, OP;
					//Execute Ledge Finding Function
					TryCreateLedgeStructureC_Implementation(LedgeValid, LP, RP, OP, SecondValid, TOrigin, UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), 0, 45, false);
					if (LedgeValid == true)
					{
						ReturnStruct.LeftPoint = ConvertLedgeStructToWS(LP).Transform;
						ReturnStruct.RightPoint = ConvertLedgeStructToWS(RP).Transform;
						ReturnStruct.Origin = ConvertLedgeStructToWS(OP).Transform;
						ReturnStruct.Component = OP.Component;
						DetectedCorner = IsValid(ReturnStruct.Component);
						OuterType = false;
						TargetLedgeStruct = ReturnStruct;
						return;
					}
				}
			}
		}
		DetectedCorner = false; OuterType = false; return;
	}
}

// CHECK CAN JUMP BACK (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::CheckCanJumpBackC_Implementation(bool& ReturnValue, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint,
	FCMC_SingleClimbPointC& OriginPoint, bool UseCameraCondition, float JumpMaxDistance)
{
	bool CamCondition = false;
	if (UseCameraCondition == true)
	{
		APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (!PCM) { ReturnValue = false; return; }
		const float CameraDot = UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::GetForwardVector(FRotator(0, PCM->GetCameraRotation().Yaw, 0)), CharacterC->GetActorForwardVector());

		CamCondition = CameraDot < 0.22f;
		//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, FVector(0, 0, CameraDot).ToCompactString()); //DEBUG

		if (CamCondition == false)
		{
			JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), SAFEDELTATIME, 8);
			ReturnValue = false; return; 
		}
	}

	if (ActionC == CMC_ActionTypeC::None && StartNarrowFloorMovementC == false && AxisValuesC.X != 0 && AxisValuesC.Y == 0)
	{
		bool LedgeOutputValid = false;
		float DownScale = UKismetMathLibrary::SelectFloat(1.2, 1, AxisValuesC.X < 0);
		float DistanceToWall = 0;
		float UpOffsetValue = 60;
		float ForwardOffsetValue = 0.8f;
		float JumpMaxDist = JumpMaxDistance; //Input Parameter
		FVector LedgeLoc, LedgeDir = FVector(0, 0, 0);
		FHitResult FTHR = {};
		ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
		EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
		if (DebugTraceIndexC > 0)
		{TraceType = EDrawDebugTrace::ForOneFrame;}
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);

		ChooseLedgeFindingTransformC(false, LedgeLoc, LedgeDir);
		const bool FirstTraceValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, LedgeLoc + (LedgeDir * -1 * 3), LedgeLoc + (LedgeDir * -1 * JumpMaxDist) + 
		FVector(0, 0, AxisValuesC.X * 50), 25, 60, Channel, false, ToIgnore, TraceType, FTHR, true, FLinearColor::Blue, FLinearColor::Green, 0.1f);
		if (FirstTraceValid == false)
		{   JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), SAFEDELTATIME, 8);
			ReturnValue = false; return; }
		DistanceToWall = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(FTHR.Location, FTHR.TraceStart), 0, UKismetMathLibrary::Vector_Distance(FTHR.TraceEnd, FTHR.TraceStart), 0, 1);
		DistanceToWall = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::FClamp(DistanceToWall, 0, 0.9), 0.3, 0.9, 1.4, 0.7);
		//Update Animation Alpha
		JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(1, UKismetMathLibrary::MapRangeClamped(AxisValuesInterpC.X,-1,1,0,1)), SAFEDELTATIME, 8);
		if (SpaceBarImpulseC == true) // Continue Function Only When Player Pressed Space Bar
		{
			FHitResult STHR = {};
			ChooseLedgeFindingTransformC(false, LedgeLoc, LedgeDir);
			for (int i = 0; i <= 3; i++)
			{
				const bool SphereTraceValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, LedgeLoc + (LedgeDir * -3), LedgeLoc + (LedgeDir * -1 * JumpMaxDist * ForwardOffsetValue) +
				FVector(0, 0, 1.25 * DownScale * DistanceToWall * AxisValuesC.X * UpOffsetValue), 25, Channel, false, ToIgnore, TraceType, STHR, true, FLinearColor(0.35f, 0.0f, 0.1f, 1.0f), 
				FLinearColor(1.0f, 0.1f, 0.2f, 1.0f), 0.05f);
				if (SphereTraceValid == true)
				{
					//{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}
					for (int j = 0; j <= 1; j++)
					{
						bool LineTraceValid = UKismetSystemLibrary::LineTraceSingle(CharacterC, STHR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * 3) +
						FVector(0, 0, (j + 1) * 24.0), STHR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * 3) + FVector(0, 0, -12), 
						Channel, false, ToIgnore, TraceType, FTHR, true);
						if (LineTraceValid == true)
						{
							TryCreateLedgeStructureC_Implementation(LedgeOutputValid, LeftPoint, RightPoint, OriginPoint, LineTraceValid, UKismetMathLibrary::SelectVector((FTHR.TraceStart + FTHR.TraceEnd) / 2,
							FTHR.ImpactPoint, FTHR.bStartPenetrating) + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * -20), UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal), 0, 45, false);
							if (LedgeOutputValid == true && UKismetMathLibrary::Dot_VectorVector(OriginPoint.Normal,CharacterC->GetActorForwardVector())<-0.55)
							{
								ReturnValue = LedgeOutputValid;
								return; //finish function with true value
							}
						}
					}
				}
				UpOffsetValue = UKismetMathLibrary::FClamp(UpOffsetValue - 20, -40, 100);
				ForwardOffsetValue = UKismetMathLibrary::FClamp(ForwardOffsetValue + 0.1, 0, 1.0);
			}
			ReturnValue = false; return;
		}
		ReturnValue = false; return;
	}
	else
	{
		JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), SAFEDELTATIME, 8);
		ReturnValue = false; return;
	}
}

//CREATE AXIS VALUES WITH INTERPOLATION
void UCpp_DynamicClimbingComponent::CreateAxisValuesWithInterpFast(float InterpSpeed, float Delta)
{
	float AxisForward = 0.0;
	float AxisRight = 0.0;
	if (IsValid(CharacterC) == false)
	{ return; }
	//float CurveValue = CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(FName("DLCv2_Movement_Speed"));
	float CurveValue = 0.0;
	GetCharacterAxisC(AxisForward, AxisRight);
	AxisValuesInterpC = FVector2D(UKismetMathLibrary::FInterpTo(AxisValuesInterpC.X, AxisForward, Delta, InterpSpeed), 
								  UKismetMathLibrary::FInterpTo(AxisValuesInterpC.Y, AxisRight, Delta, InterpSpeed));

	if (StartedZiplineC() || bIsSwingingC)
	{
		AxisValuesInterpSlowC = FVector2D(UKismetMathLibrary::FInterpTo(AxisValuesInterpSlowC.X, AxisForward, Delta, InterpSpeed * 0.4),
			UKismetMathLibrary::FClamp(UKismetMathLibrary::FInterpTo(AxisValuesInterpSlowC.Y, AxisRight, Delta, InterpSpeed * 0.4) + CurveValue, -1, 1));
	}
	else
	{
		AxisValuesInterpSlowC = AxisValuesInterpC;
	}
}

//CONVERT LEDGE TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeToCapPositionC(FCALS_ComponentAndTransform Center)
{
	FTransform ReturnTransform = FTransform(Center.Transform.Rotator(), Center.Transform.GetLocation() + 
	(UKismetMathLibrary::GetForwardVector(Center.Transform.Rotator()) * (DefCapsuleSizeC.X * -1.0)) + FVector(0,0,CapsuleUpOffsetC * -1.0), FVector(1, 1, 1));
	FCALS_ComponentAndTransform ReturnStructure = {};
	ReturnStructure.Transform = ReturnTransform;
	ReturnStructure.Component = Center.Component;
	return ReturnStructure;
}

//CONVERT FLOOR TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertFloorToCapPositionC(FCALS_ComponentAndTransform Center)
{
	FCALS_ComponentAndTransform ReturnStructure = {};
	ReturnStructure.Transform = FTransform(Center.Transform.Rotator(), Center.Transform.GetLocation() +
		FVector(0, 0, CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FVector(1, 1, 1));
	ReturnStructure.Component = Center.Component;
	return ReturnStructure;
}

//CHOOSE LEDGE FINDING TRANSFORM
void UCpp_DynamicClimbingComponent::ChooseLedgeFindingTransformC(bool GetByLedge, FVector& ReturnLocation, FVector& ReturnDirection)
{
	if (GetByLedge == true)
	{
		FCALS_ComponentAndTransform LedgeToSinglePoint = {};
		LedgeToSinglePoint.Transform = LedgePointsLS_C.Origin;
		LedgeToSinglePoint.Component = LedgePointsLS_C.Component;
		LedgeToSinglePoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(LedgeToSinglePoint);
		ReturnLocation = LedgeToSinglePoint.Transform.GetLocation() + (UKismetMathLibrary::GetForwardVector(LedgeToSinglePoint.Transform.Rotator()) * DefCapsuleSizeC.X * -0.9);
		ReturnDirection = UKismetMathLibrary::GetForwardVector(LedgeToSinglePoint.Transform.Rotator());
	}
	else
	{
		ReturnLocation = CharacterC->GetActorLocation() + FVector(0, 0, CapsuleUpOffsetC);
		ReturnDirection = CharacterC->GetActorForwardVector();
	}
}

//GET DIRECTION BY PLAYER AXIS INPUT
FVector UCpp_DynamicClimbingComponent::GetDirectionByInputC(float LerpWithForward)
{
	float AxisForward = 0.0;
	float AxisRight = 0.0;
	GetCharacterAxisC(AxisForward, AxisRight);
	if (AxisForward == 0.0 && AxisRight == 0.0)
	{ AxisForward = 1.0; AxisRight = 0.0; }
	return UKismetMathLibrary::VLerp(UKismetMathLibrary::ClampVectorSize((CharacterC->GetActorForwardVector() * abs(AxisForward)) + 
		   (CharacterC->GetActorRightVector() * AxisRight), -1.0, 1.0), CharacterC->GetActorForwardVector(), LerpWithForward);
}

//CONVERT SINGLE LEDGE STRUCTURE (AS TWO VECTORS - LOCATION & ROTATION) TO TRANSFORM & COMPONENT STRUCTURE
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeStructToWS(FCMC_SingleClimbPointC SingleClimbPointWS)
{
	FCALS_ComponentAndTransform ReturnStruct = {};
	ReturnStruct.Transform = FTransform(FRotator(UKismetMathLibrary::MakeRotFromX(SingleClimbPointWS.Normal)), FVector(SingleClimbPointWS.Location), FVector(1, 1, 1));
	ReturnStruct.Component = SingleClimbPointWS.Component;
	return ReturnStruct;
}

//CONVERT SINGLE LEDGE STRUCTURE (AS TWO VECTORS - LOCATION & ROTATION) TO TRANSFORM & COMPONENT STRUCTURE [BUT WITH LOCAL SPACE CONVERT]
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeStructToLS(FCMC_SingleClimbPointC SingleClimbPointWS)
{
	FCALS_ComponentAndTransform ReturnStruct = {};
	ReturnStruct.Transform = FTransform(FRotator(UKismetMathLibrary::MakeRotFromX(SingleClimbPointWS.Normal)), FVector(SingleClimbPointWS.Location), FVector(1, 1, 1));
	ReturnStruct.Component = SingleClimbPointWS.Component;
	return UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(ReturnStruct);
}



//CHECK CAN DROP TO LEDGE
bool UCpp_DynamicClimbingComponent::CheckCanDropToLedgeC(FCMC_LedgeC& LedgeStructWS)
{
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	TArray<AActor*> ToIgnore;
	FHitResult LineHitResult;
	FVector LineTraceOrigin = CharacterC->GetActorLocation() - FVector(0, 0, CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight())-FVector(0,0,10);
	FVector Offset = FVector(0, 0, 10);

	const bool LineHitValid = UKismetSystemLibrary::LineTraceSingle(CharacterC, LineTraceOrigin + (CharacterC->GetActorForwardVector() * 45.0), 
	LineTraceOrigin + (CharacterC->GetActorForwardVector() * -5.0), Channel, false, ToIgnore, TraceType, LineHitResult, true, FLinearColor::Black, FLinearColor::Green, 0.6f);

	if (LineHitValid == true && LineHitResult.bStartPenetrating == false)
	{
		//Check Capsule Have Room - If Not finish funtion
		LineTraceOrigin = LineHitResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal) * DefCapsuleSizeC.X * -1.0);
		if (UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnoreTransform(CharacterC, CharacterC, 
		FTransform(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal)), LineTraceOrigin, FVector(1, 1, 1)), ToIgnore, 0.8f, 0.9f, false) == false)
		{ return false; }
		for (int i = 0; i <= 1; i++)
		{
			LineTraceOrigin = LineHitResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal) * DefCapsuleSizeC.X * -0.6f);
			if (i > 0)
			{ Offset = FVector(0, 0, 25); }
			bool LedgeValid = false;
			bool WallValid = true;
			FCMC_SingleClimbPointC Right;
			FCMC_SingleClimbPointC Left;
			FCMC_SingleClimbPointC Origin;
			FCMC_LedgeC ReturnStruct;

			TryCreateLedgeStructureC_Implementation(LedgeValid, Left, Right, Origin, WallValid, LineTraceOrigin + Offset, 
			UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal), 0, 45, true);
			if (LedgeValid == true)
			{
				ReturnStruct.LeftPoint = ConvertLedgeStructToWS(Left).Transform;
				ReturnStruct.RightPoint = ConvertLedgeStructToWS(Right).Transform;
				ReturnStruct.Origin = ConvertLedgeStructToWS(Origin).Transform;
				ReturnStruct.Component = Origin.Component;
				LedgeStructWS = ReturnStruct;
				return true;
			}
		}
	}
	return false;
}

//RESIZE CAPSULE RADIUS TO DEFAULT
void UCpp_DynamicClimbingComponent::ResizeCapsuleToDefaultC(float InterpSpeed)
{
	if (StartNarrowFloorMovementC == true)
	{ return; }
	if (DefCapsuleSizeC.X != CharacterC->GetCapsuleComponent()->GetUnscaledCapsuleRadius())
	{
		CharacterC->GetCapsuleComponent()->SetCapsuleRadius(UKismetMathLibrary::FInterpTo_Constant(CharacterC->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 
		DefCapsuleSizeC.X, SAFEDELTATIME, InterpSpeed), true);
	}
	return;
}

//DO WHEN IS CLIMBING - CHECK LEDGE PER FRAME
void UCpp_DynamicClimbingComponent::UpdateLedgePerFrameC(FCMC_LedgeC& OutLedge, FVector& OutOrigin, FVector2D SlopeScale, float ConstMovementOffset, bool InputLock)
{
	//Update Cached Ledge
	if (AxisValuesC.X == 0 && AxisValuesC.Y == 0)
	{ CachedLedgePointsLS_C = LedgePointsLS_C; }

	//Calculate Slope Offset
	float SlopeOffset = 0;
	FCALS_ComponentAndTransform LP, RP;
	LP.Transform = LedgePointsLS_C.LeftPoint;
	LP.Component = LedgePointsLS_C.Component;
	RP.Transform = LedgePointsLS_C.RightPoint;
	RP.Component = LedgePointsLS_C.Component;
	SlopeOffset = (UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(LP).Transform.GetLocation().Z - 
	UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(RP).Transform.GetLocation().Z) * AxisValuesC.Y * -1;
	if (SlopeOffset < 0)
	{ SlopeOffset = SlopeOffset * SlopeScale.Y; }
	else
	{ SlopeOffset = SlopeOffset * SlopeScale.X; }

	//Set Detection Origin
	FVector DetectionNormal = FVector(0, 0, 0);
	FVector DetectionOrigin = FVector(0, 0, 0);
	ChooseLedgeFindingTransformC(false, DetectionOrigin, DetectionNormal);
	DetectionOrigin = DetectionOrigin + FVector(0, 0, SlopeOffset) + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(DetectionNormal)) 
	* AxisValuesC.Y * ConstMovementOffset * UKismetMathLibrary::SelectFloat(0, 1, InputLock));
	OutOrigin = DetectionOrigin;
	//Find Ledge
	if (StartNarrowFloorMovementC == false)
	{
		bool LedgeValid, WallHitValid = false;
		FCMC_SingleClimbPointC LPR, RPR, OPR;
		FCMC_LedgeC OutStruct;
		TryCreateLedgeStructureC_Implementation(LedgeValid, LPR, RPR, OPR, WallHitValid, DetectionOrigin, DetectionNormal, 0, 45, true);
		OutStruct.LeftPoint = ConvertLedgeStructToLS(LPR).Transform;
		OutStruct.RightPoint = ConvertLedgeStructToLS(RPR).Transform;
		OutStruct.Origin = ConvertLedgeStructToLS(OPR).Transform;
		OutStruct.Component = OPR.Component;
		OutLedge = OutStruct;
		return;
	}
	else
	{ OutLedge = LedgePointsLS_C; return; }
}

//CHECK FOOTS INVERSE KINEMATIC 
bool UCpp_DynamicClimbingComponent::CheckFootIkValidC(FTransform Transform, bool ForRightFoot, float TraceUpOffset)
{
	FVector FootOffset = FootsDefOffsetsC.v1;
	FVector V = FVector(0, 0, 0);
	if (ForRightFoot == true)
	{ FootOffset = FootsDefOffsetsC.v2; }
	
	FootOffset = UKismetMathLibrary::Quat_RotateVector(CharacterC->GetMesh()->GetComponentTransform().GetRotation(), FootOffset);
	FootOffset = FootOffset + CharacterC->GetMesh()->GetComponentTransform().GetLocation();
	V = UKismetMathLibrary::Quat_RotateVector(Transform.GetRotation(), UKismetMathLibrary::MakeRelativeTransform(Transform, CharacterC->GetActorTransform()).GetLocation());
	V = V + FootOffset;
	bool HitValid = false;
	FTwoVectors HitTransform;
	UHelpfulFunctionsBPLibrary::ClimbingFootIK(CharacterC, HitValid, HitTransform.v1, HitTransform.v2, CharacterC, V, UKismetMathLibrary::GetForwardVector(Transform.Rotator()), 
	0, 18, FVector2D(8, 45), false, "Thigh_L", "calf_L", "Foot_L", 18, DebugFootsTraceIndexC);
	return HitValid;
}

//GET FOOTS RELATIVE VELOCITY
FVector UCpp_DynamicClimbingComponent::GetFootsRelativeVelocityC()
{
	FVector LocalVelocity = FVector(0, 0, 0);
	FTransform RelFootsT = UKismetMathLibrary::MakeRelativeTransform(UKismetMathLibrary::TLerp(CharacterC->GetMesh()->GetSocketTransform("Foot_L", ERelativeTransformSpace::RTS_World),
	CharacterC->GetMesh()->GetSocketTransform("Foot_R", ERelativeTransformSpace::RTS_World), 0.5), FTransform(UKismetMathLibrary::Conv_RotatorToQuaternion(CharacterC->GetActorRotation()), 
	CharacterC->GetActorLocation(), FVector(1, 1, 1)));
	LocalVelocity = (RelFootsT.GetLocation() - FootsRelativeVelocityC)/ SAFEDELTATIME;
	FootsRelativeVelocityC = RelFootsT.GetLocation();
	return LocalVelocity;
}

#pragma endregion


#pragma region Functions - Some Helpers 

//CONVERT AXIS TO NAME
FName UCpp_DynamicClimbingComponent::ConvertAxisToNameC()
{
	if (AxisValuesC.X == 1 && AxisValuesC.Y == 0)
	{ return "U"; }
	else if(abs(AxisValuesC.X) == 1 && abs(AxisValuesC.Y) == 1)
	{ return "UL"; }
	else if (abs(AxisValuesC.X) == 0 && abs(AxisValuesC.Y) == 1)
	{ return "L"; }
	else if (AxisValuesC.X < 0 && abs(AxisValuesC.Y) == 0)
	{ return "D"; }
	return "U";
}

//CONVERT WALL TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertWallToCapPositionC(FCALS_ComponentAndTransform TransformWS)
{
	FCALS_ComponentAndTransform Cap;
	Cap.Transform = FTransform(TransformWS.Transform.Rotator(), TransformWS.Transform.GetLocation() + UKismetMathLibrary::GetForwardVector(TransformWS.Transform.Rotator())
		* ((DefCapsuleSizeC.X + ConstCapsuleOffsetBetWallC) * -1), FVector(1, 1, 1));
	Cap.Component = TransformWS.Component;
	return Cap;
}

#pragma endregion


#pragma region Functions - WALL Climbing / Pickaxe Climb

//CHECK PLAYER CAN MOVE TO WALL
bool UCpp_DynamicClimbingComponent::CheckPlayerCanMoveToWallC(bool Check, FCALS_ComponentAndTransform TransformWS, FCALS_ComponentAndTransform& ReturnTransformWS)
{
	if (Check == false)
	{ ReturnTransformWS = TransformWS; return false;}
	FHitResult BoxResult = {};
	const ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DebugTraceIndexC > 1)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	FTransform WallT = ConvertWallToCapPositionC(TransformWS).Transform;
	const bool BoxValid = UKismetSystemLibrary::BoxTraceSingle(CharacterC, WallT.GetLocation() + (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4f) +
		(UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4f * -0.08f), WallT.GetLocation() + (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4f)
		+ (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4f * 0.08f), FVector(10, DefCapsuleSizeC.X * 0.84f, 25), WallT.Rotator(), Channel, false, ToIgnore, TraceType,
		BoxResult, true, FLinearColor::Black, FLinearColor::Red, 0.15f);
	if (BoxValid == true)
	{ ReturnTransformWS = TransformWS; return false; }
	FHitResult SphereResult = {};
	const bool SphereValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, CharacterC->GetActorLocation(), WallT.GetLocation(), 8, Channel, false, ToIgnore, TraceType, SphereResult,
		false, FLinearColor::Gray, FLinearColor::Red, 0.18f);
	if (SphereValid == true)
	{ ReturnTransformWS = TransformWS; return false; }
	ReturnTransformWS = TransformWS;
	return true;
}

// TRY FIND WALL TANGENT FOR CHARACTER
void UCpp_DynamicClimbingComponent::TryFindTangentForWallC(bool& ReturnValid, FCALS_ComponentAndTransform& TransformWS, FVector FindingLocation, 
	FVector FindingDirection, float FindingLength, float FirstRadius, float DistanceOffsetScale, int VerticalAccuracy, FVector2D CapsuleSize)
{
	//Default Trace Parameters
	const ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(PickaxeClimbChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DebugTraceIndexC > 1)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	//Run First Trace
	FHitResult FTHR = {};
	const bool FirstValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, FindingLocation, FindingLocation + (FindingDirection * FindingLength), FirstRadius, Channel, false, ToIgnore,
	TraceType, FTHR, true, FLinearColor(0.2f, 0.0f, 0.5f, 1.0f), FLinearColor(0.0f, 0.4f, 1.0f, 1.0f), 0.05f);
	if (FirstValid == false)
	{ ReturnValid = false; return; }

	FVector WallImpactWithOffset = FVector(0, 0, 0);
	FVector WallForwardVector = FVector(0, 0, 0);

	WallImpactWithOffset = UKismetMathLibrary::FindClosestPointOnLine(FTHR.ImpactPoint, UKismetMathLibrary::VLerp(FTHR.TraceStart, FTHR.TraceEnd, 0.5), FindingDirection);
	WallImpactWithOffset = FTHR.ImpactPoint + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(FTHR.ImpactPoint, WallImpactWithOffset)) * 
	(UKismetMathLibrary::Vector_Distance(FTHR.ImpactPoint, WallImpactWithOffset) * UKismetMathLibrary::SelectFloat(DistanceOffsetScale, 0, UKismetMathLibrary::Vector_Distance(FTHR.ImpactPoint, WallImpactWithOffset) > 8));
	WallForwardVector = UHelpfulFunctionsBPLibrary::NormalToVector(FTHR.Normal);

	//Init Local Variables
	FVector TStart, TEnd, AvgNormalL, AvgNormalR = FVector(0, 0, 0);
	FTwoVectors AvgHits, CurrentHit;
	TArray<FTwoVectors> HitArray; //Array Variable
	FHitResult SR = {};
	int HitCounter = 0;

	for (int i = 0; i <= 1; i++)
	{
		for (int j = 0; j <= VerticalAccuracy; j++)
		{
			TStart = WallImpactWithOffset + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallForwardVector)) * CapsuleSize.X * 0.6 * UKismetMathLibrary::SelectFloat(-1, 1, i > 0));
			TStart = TStart + FVector(0, 0, UKismetMathLibrary::MapRangeClamped(j * 1.0, 0, VerticalAccuracy * 1.0, -40, 40));

			const bool SphereValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TStart + (WallForwardVector * -25.0), TStart + (WallForwardVector * 22), 4, Channel, false, 
			ToIgnore, TraceType, SR, true, FLinearColor::Red, FLinearColor::Yellow, 0.07f);
			if (SphereValid == true)
			{
				HitCounter = HitCounter + 1;
				CurrentHit = FTwoVectors(SR.ImpactPoint, UKismetMathLibrary::SelectVector(UKismetMathLibrary::VLerp(SR.Normal, FTHR.Normal, abs(UKismetMathLibrary::Dot_VectorVector(SR.Normal, 
				FVector(0, 0, 1)))), SR.Normal, abs(UKismetMathLibrary::Dot_VectorVector(SR.Normal, FVector(0, 0, 1))) > 0.55));
				HitArray.Emplace(CurrentHit);
				AvgHits = AvgHits + CurrentHit;
				if (i == 0)
				{ AvgNormalR = AvgNormalR + CurrentHit.v2; }
				else
				{ AvgNormalL = AvgNormalL + CurrentHit.v2; }
			}
		}
	}
	if (HitArray.Num() > 3)
	{
		FCALS_ComponentAndTransform OutputTransform;
		FHitResult LastResult;

		if (UKismetMathLibrary::Dot_VectorVector(AvgNormalL / 3, AvgNormalR / 3) < 0.5)
		{ ReturnValid = false; return; }

		OutputTransform.Transform = FTransform(UKismetMathLibrary::MakeRotFromX((AvgHits.v2 / HitCounter) * -1.0), AvgHits.v1 / HitCounter, FVector(1, 1, 1));
		OutputTransform.Component = FTHR.GetComponent();

		TStart = OutputTransform.Transform.GetLocation() + (UKismetMathLibrary::GetForwardVector(OutputTransform.Transform.Rotator()) * -1 * (CapsuleSize.X + ConstCapsuleOffsetBetWallC));
		TStart = TStart + FVector(0, 0, -15.0);
		TEnd = TStart + (UKismetMathLibrary::GetUpVector(OutputTransform.Transform.Rotator()) * CapsuleSize.Y * 0.55*-1.0);
		TStart = TStart + (UKismetMathLibrary::GetUpVector(OutputTransform.Transform.Rotator()) * CapsuleSize.Y * 0.55 * 1.0);

		const bool LastTraceValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TStart, TEnd, CapsuleSize.X * UKismetMathLibrary::SelectFloat(0.5, 0.85f, IsClimbingC), 
		UEngineTypes::ConvertToTraceType(ForClimbingChannelC), false, ToIgnore, TraceType, LastResult, true, FLinearColor::Red, FLinearColor::Gray, 0.06f);
		//Final Condition
		if (LastTraceValid == false)
		{
			ReturnValid = IsValid(OutputTransform.Component); //Try Return TRUE
			TransformWS = OutputTransform;
			return;
		}
		else
		{ ReturnValid = false; return; }
	}
	else
	{ ReturnValid = false; return; }
}

#pragma endregion


#pragma region Functions - Rope Swinging

// ROPE SWING FUNCTION - FORCE CALCULATION
void UCpp_DynamicClimbingComponent::RopeSwingUpdatePhysicC(UCurveVector* RegulationCurve, float SwingMinRange, float SwingMaxRange, int HandAttachIndex, bool AddtiveCondition, float HysteresisMin,
	float GravityStabilityFactor, float ForceScaleFactor, float InterpSpeedIn, float SphereOriginInterpSpeed, float RadiusInterpSpeed, FVector2D SwingLenghtFactor)
{
	if (RopeHookedConditionC() == false || IsValid(CableSimC) == false) { return; }
	if(AddtiveCondition == false) { return; }

	//const float dt = GetWorld()->GetDeltaSeconds();

	//Part 1: Find Collision Index
	TArray<FExposedCableParticle> Particles = CableSimC->GetCableParticlesStructure();
	FVector AnchorPoint = AnchorPointInterpC;
	int CollisionIndex = -1;

	for (int i = HandAttachIndex + 1; i < Particles.Num(); i++)
	{
		if (Particles[i].bIsColliding == true || Particles[i].bIsFree == false)
		{
			if (CheckNormalForPointC(Particles[i]) == true)
			{
				AnchorPoint = Particles[i].Position;
				CollisionIndex = i;
				break;
			}
		}
	}
	if (CollisionIndex == -1) { return; }

	CollisionIndexC = CollisionIndex;

	//Part 2:
	AnchorPointInterpC = UKismetMathLibrary::VInterpTo(AnchorPointInterpC, AnchorPoint, dt, SphereOriginInterpSpeed);

	const TArray<float> StretchArray = CableSimC->GetStretchTolleranceValuePerSegment();										
	if (StretchArray.Num() == 0) { return; }
	float StretchValue = StretchArray.Last();
	StretchValue = UKismetMathLibrary::MapRangeClamped(StretchValue, 0.9f, 1.1f, SwingLenghtFactor.X, SwingLenghtFactor.Y);

	//Part 3:
	float MaxLenght = 0.0;
	const TArray<int> AttachPoints = CableSimC->GetIndicesOfAttachedPoints();
	if (AttachPoints.Num() < 2) { return; }

	const int ParticlesDelta = AttachPoints[2] - AttachPoints[1];
	const float LastSegmet = CableSimC->GetCableInitSegmentsLength()[FMath::Clamp<int>(2, 0, CableSimC->GetCableInitSegmentsLength().Num() - 1)];

	MaxLenght = (LastSegmet / (float)ParticlesDelta) * (CollisionIndex - AttachPoints[1]) * StretchValue;
	MaxLenght = MaxLenght + ConstSwingLenghtOffsetC;
	MaxLenght = FMath::Clamp<float>(MaxLenght, SwingMinRange, SwingMaxRange);

	SwingRadiusSmoothC = UKismetMathLibrary::FInterpTo(SwingRadiusSmoothC, MaxLenght, SAFEDELTATIME, RadiusInterpSpeed);

	//Part 4: Calculate Regulation Factor From Curve Data. This Values control Force Strenght
	FVector RegulationConstrolFactors = RegulationCurve->GetVectorValue(MaxLenght - (CharacterC->GetActorLocation()-AnchorPointInterpC).Length());
	RegulationConstrolFactors.Y = RegulationConstrolFactors.Y * FMath::Lerp<float>(CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue("CMW_MotionStrenght"), 1, 1.25f);

	//Part 5: Calculate Target Final Force For Player
	FVector ExtoritionForce = FVector::Zero();

	if (UKismetMathLibrary::Vector_Distance(CharacterC->GetActorLocation(), AnchorPointInterpC) > (MaxLenght - HysteresisMin))
	{
		FVector Direction = CharacterC->GetActorLocation() - AnchorPointInterpC;
		Direction.Normalize();

		const float Scale = UKismetMathLibrary::Dot_VectorVector(CharacterC->GetVelocity(), CharacterC->GetActorLocation() - AnchorPointInterpC);
		float ForceStrenght = Scale / (MaxLenght * RegulationConstrolFactors.X); // By using this Player can increase distance between anchor point
		ForceStrenght = ForceStrenght + (UKismetMathLibrary::Vector_Distance(AnchorPointInterpC, CharacterC->GetActorLocation()) * RegulationConstrolFactors.Y); // By using this Player can reduce distance between anchor point

		ExtoritionForce = Direction * ForceStrenght * (1 / SAFEDELTATIME) * CharacterC->GetCharacterMovement()->Mass * ForceScaleFactor * -1.0;
	}

	//Part 6: Smoothing Force
	if ((CharacterC->GetActorLocation().Z - AnchorPoint.Z) < 0.0)
	{
		TargetForceC = UKismetMathLibrary::VInterpTo(TargetForceC, ExtoritionForce, SAFEDELTATIME, InterpSpeedIn);
	}
	else
	{
		TargetForceC = UKismetMathLibrary::VInterpTo(TargetForceC, FVector(0,0,0), SAFEDELTATIME, InterpSpeedIn);
	}
	
	//Part 7: Calculate Gravity Stabilization Target Force (Optional)
	FVector TargetGravityForce = FVector::Zero();

	if (GravityStabilityFactor > 0)
	{
		float GravityReduceStrenght = CharacterC->GetCharacterMovement()->Mass * CharacterC->GetCharacterMovement()->GetGravityZ();
		GravityReduceStrenght = GravityReduceStrenght * (UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::FindLookAtRotation(CharacterC->GetActorLocation(), 
			AnchorPointInterpC)), FVector(0, 0, 1)) / 1.5707f);

		GravityReduceStrenght = GravityReduceStrenght * -1.0 * GravityStabilityFactor;

		if (CharacterC->GetActorLocation().Z - AnchorPointInterpC.Z >= 0.0) { GravityReduceStrenght = 0.0; }

		TargetGravityForce = FVector(0, 0, GravityReduceStrenght);
	}
	GravityStabilityForceC = UKismetMathLibrary::VInterpTo(GravityStabilityForceC, TargetGravityForce, dt, InterpSpeedIn);

	//Part 8: Apply Final Forces To Character
	CharacterC->GetCharacterMovement()->AddForce(UKismetMathLibrary::ClampVectorSize(TargetForceC + GravityStabilityForceC, -1000000, 1000000));
	return;
}

// ROPE SWING FUNCTION - TRY FIND HOOK ACTOR
bool UCpp_DynamicClimbingComponent::TryFindHookPointC(AActor*& HookActor, TEnumAsByte<EObjectTypeQuery> TraceObject, float FindingRadius, float CapsuleHeightScale, 
	FVector Direction, float DistancePioryty, int DrawDebug)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DrawDebug > 0) { TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DrawDebug > 1) { TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	TArray< TEnumAsByte<EObjectTypeQuery>> Objects;
	Objects.Add(TraceObject);

	const FVector TStart = CharacterC->GetActorLocation() + FVector(0,0,40) + (Direction * FindingRadius * 0.5f);

	TArray<FHitResult> HitsResult;
	const bool HitValid = UKismetSystemLibrary::SphereTraceMultiForObjects(CharacterC, TStart, TStart + (Direction * FindingRadius * 0.5f * CapsuleHeightScale), FindingRadius, 
		Objects, false, ToIgnore, TraceType, HitsResult, true, FLinearColor::Red, FLinearColor::Yellow, 0.2f);

	AActor* HitActor = nullptr;
	//Define Pioriting Arrays
	TArray<AActor*> HitActorsArray; 
	TArray<float> WeightArray;

	for (FHitResult SingleHit : HitsResult)
	{
		HitActor = SingleHit.GetActor();

		if (HitActor && HitActor->Implements<UALS_HookActorInterface>())
		{
			IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(HitActor); //Get Interface
			
			bool ValidActor = false;
			HookInterface->Execute_HAFSI_Get_IsHookActor(HitActor, ValidActor);
			if (ValidActor == true)
			{
				HookInterface->Execute_HAFSI_Get_ItsCurrentUsed(HitActor, ValidActor);
				if (ValidActor == false)
				{
					const float DistanceMap = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(HitActor->GetActorLocation(), CharacterC->GetActorLocation()), 100.0, 
						UKismetMathLibrary::Vector_Distance(SingleHit.TraceStart, SingleHit.TraceEnd) + FindingRadius + 50.0, 0.0, 1.0);

					const float RotationMap = 1.0 - (UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(HitActor->GetActorLocation(), 
						CharacterC->GetActorLocation())), UKismetMathLibrary::GetForwardVector(CharacterC->GetControlRotation())) * -1.0);

					ToIgnore.Add(HitActor);
					FHitResult SecondResult;
					TEnumAsByte<ETraceTypeQuery> Channel = ETraceTypeQuery::TraceTypeQuery1;

					const bool SecondHit = UKismetSystemLibrary::SphereTraceSingle(CharacterC, CharacterC->GetActorLocation(), HitActor->GetActorLocation(), 25, Channel, false, ToIgnore, 
						TraceType, SecondResult, true, FLinearColor::Black, FLinearColor::Red, 0.25);

					if (SecondHit == false)
					{
						HitActorsArray.Add(HitActor);
						WeightArray.Add(FMath::Lerp<float>(RotationMap, DistanceMap, DistancePioryty));
					}
				}
			}
		}
	}
	if (HitActorsArray.Num() == 0) { HookActor = nullptr;  return false; }

	float MinInArray = 0.0; int MinIndex = 0;
	UKismetMathLibrary::MinOfFloatArray(WeightArray, MinIndex, MinInArray);

	HookActor = HitActorsArray[MinIndex];

	if (IsValid(HookActor) == false) { return false; }

	for (AActor* A : HitActorsArray)
	{
		if (HookActor != A)
		{
			//Clear Widget For Other Actors
			IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(A); //Get Interface
			HookInterface->Execute_HAFSI_Play_AnimOut(A);
		}
	}

	return IsValid(HookActor);
}

void UCpp_DynamicClimbingComponent::RopeLenghtUpdateC(UCurveVector* ForceCurve, float RopeMinLeght, float RopeMaxLenght, int HandAttachIndex, FName TimerName, float TimerMaxTime,
	float UpForceStrenght, int ExpandDivite, FVector2D InterpSpeedRange)
{
	if (IsValid(ForceCurve) == false || IsValid(CableSimC) == false) { return; }

	//const float dt = GetWorld()->GetDeltaSeconds();

	TArray<FVector> PLocations;
	CableSimC->GetCableParticleLocations(PLocations);
	const FVector P1 = PLocations[HandAttachIndex];
	// Section 1:
	if (UKismetMathLibrary::Vector_Distance(AnchorPointInterpC, P1) < RopeMinLeght && CableSimC->CableLength < RopeMinLeght)
	{ TargetRopeLenghtC = FMath::Clamp<float>(TargetRopeLenghtC + 10, RopeMinLeght, RopeMaxLenght); }

	//Section 2:
	const float NewTargetValue = FMath::Clamp<float>(TargetRopeLenghtC + 10, RopeMinLeght, RopeMaxLenght);
	float InterpSpeedValue = FMath::Lerp<float>(InterpSpeedRange.X, InterpSpeedRange.Y, CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(TEXT("CMW_MotionStrenght")));

	//Section 3:
	if (TargetRopeLenghtC > RopeMinLeght)
	{
		const float NewLeght = UKismetMathLibrary::FInterpTo(CableSimC->CableLength, NewTargetValue, SAFEDELTATIME, InterpSpeedValue);
		CableSimC->UpdateCableLength(NewLeght, true);
	}
	//Section 4: stretching the rope
	float TimeElapsed = UKismetSystemLibrary::K2_GetTimerElapsedTime(this, TimerName.ToString());
	if (TimeElapsed != -1 && CollisionIndexC > 0 && CollisionIndexC != CableSimC->NumSegments)
	{
		const float TimeMapped = ForceCurve->GetVectorValue(UKismetMathLibrary::MapRangeClamped(TimeElapsed, 0.0, TimerMaxTime, 0.0, 1.0)).Y;
		FVector Force = FVector(CharacterC->GetVelocity().X, CharacterC->GetVelocity().Y, 0.0) * TimeMapped;
		Force = Force + FVector(0, 0, TimeMapped * UpForceStrenght);

		CableSimC->ApplyConstForceToParticle(Force, HandAttachIndex + 1, true, CableSimC->NumSegments / 4, 0.1f);
	}
	else
	{
		CableSimC->ReduceForceForParticles(0, HandAttachIndex + 1 + (CableSimC->NumSegments / 4) + 2, 6, dt);
	}
	return;
}


void UCpp_DynamicClimbingComponent::ReducingVelocityWhenSwingC(float ReductionDampingFactor, float VelocityTrigger, float SwingingDampingFactor, int ActionIndex)
{
	if (IsValid(CharacterC) == false) { return; }

	if (bIsFallingStartedC == false && CharacterC->GetVelocity().Length() > VelocityTrigger)
	{
		if (CableSimC->GetAnyPointIsColliding(6, CableSimC->NumSegments - 1) == true)
		{
			const FVector Impulse = CharacterC->GetVelocity() * -1.0 * ReductionDampingFactor;
			CharacterC->GetCharacterMovement()->AddImpulse(Impulse, true);
		}
	}
	else
	{
		if (ActionIndex < 8) {
			const FVector Impulse = CharacterC->GetVelocity() * FMath::Clamp<float>(1.0 - SwingingDampingFactor, 0.0, 1.0);
			CharacterC->GetCharacterMovement()->AddImpulse(Impulse, false);
		}
	}
	return;
}


void UCpp_DynamicClimbingComponent::UpdateAirControlC(int ActionIndex, float RopeMinLenght, float SwingMinRange, float SwingMaxRange, FVector2D AirControlRange, 
	float SwingMinBias, float InterpToSpeed, float ReduceToZeroSpeed)
{
	const float cdt = SAFEDELTATIME;

	if (ActionIndex >= 8)
	{
		CharacterC->GetCharacterMovement()->AirControl = UKismetMathLibrary::FInterpTo(CharacterC->GetCharacterMovement()->AirControl, 0.0, cdt, ReduceToZeroSpeed);
	}
	else
	{
		float TargetValue = 0.0;
		const int FreeParticles = FMath::Clamp<int>(CableSimC->NumSegments - CollisionIndexC, 1, 100);
		if (SwingRadiusSmoothC < (SwingMinRange + SwingMinBias) / (float)FreeParticles)
		{
			TargetValue = 0.0;
		}
		else
		{
			const float MappedRange = UKismetMathLibrary::MapRangeClamped(TargetRopeLenghtC, RopeMinLenght, FMath::Clamp<float>(RopeMinLenght + 250.0, 0.0, SwingMaxRange), 0.0, 1.0);
			TargetValue = FMath::Lerp<float>(AirControlRange.X, AirControlRange.Y, MappedRange);
			CharacterC->GetCharacterMovement()->AirControl = UKismetMathLibrary::FInterpTo(CharacterC->GetCharacterMovement()->AirControl, TargetValue, cdt, InterpToSpeed);
		}

	}
	return;
}

#pragma endregion


#pragma region Functions - NEW LEDGE SOLVER functions collection ADDED FOR AGLS v2.0

bool UCpp_DynamicClimbingComponent::FindSingleLedgePointUsingComplexTraces(ACharacter* InCharacter, FTransform& PointTransform, FHitResult& WallHit, FHitResult& SurfaceHit, int32& ReturnQueries, 
	FVector SearchOrigin, FVector SearchDirection, TEnumAsByte<ECollisionChannel> Channel, double MaxLedgeWidth, double MinLedgeWidth, double ForwardWallSearchLength, double ForwardWallSearchRadius, int32 UpTracesChecksNumber, 
	int32 MaxUpTracesCandidates, int PhaseCheckTracesCount, float PhaseTollerance, FVector2D SurfaceLineTraceLength, int DebugingModeIndex, float DebugDrawTime)
{
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(Channel);
	const TArray<AActor*> ActorsToIgnore = ActorsInstancesIgnoredByLedgeTraces;
	const float DrawTime = DebugDrawTime;
	const float WallSphereDrawTime = DrawTime * 0.2f; // BP: 0.2 s.
	const bool bIgnoreSelf = true;


	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if (DebugingModeIndex == 4 || DebugingModeIndex == 2) DrawDebugType = EDrawDebugTrace::ForDuration;
	if (DebugingModeIndex == 1 || DebugingModeIndex == 3) DrawDebugType = EDrawDebugTrace::ForOneFrame;
	const bool DrawAdditiveShapes = DebugingModeIndex == 4 || DebugingModeIndex == 3;

	// W BP kontekst swiata jest niepodlaczony: odpowiada mu this.
	// InCharacter jest jedynie sprawdzany przez IsValid.
	// Nie dodajemy go automatycznie do ActorsToIgnore.
	PointTransform = FTransform::Identity;
	WallHit = FHitResult();
	SurfaceHit = FHitResult();
	ReturnQueries = 0;

	if (!IsValid(InCharacter))
	{
		return false;
	}

	// Te wejscia nie sa uzywane w przeslanym grafie.
	(void)MaxLedgeWidth;
	(void)MinLedgeWidth;

	const double UpTracingWallNormalSearchOffset = 4.0;
	const FVector2D PhaseAnglesClamp(0.25, 0.25);
	const int32 PhaseCheckNumber = FMath::Clamp<int>(0, 1, PhaseCheckTracesCount - 1); // Petla 0..1: dwa sprawdzenia.
	TArray<FHitResult> ValidUpTraces;

	// 1. Wstepne wyszukanie sciany.
	TArray<FHitResult> CapsuleHits;
	KSL::CapsuleTraceMulti(
		this,
		SearchOrigin - SearchDirection,
		SearchOrigin + SearchDirection * ForwardWallSearchLength
		+ FVector(0.0, 0.0, -2.0),
		static_cast<float>(ForwardWallSearchRadius),
		static_cast<float>(ForwardWallSearchRadius + 20.0),
		TraceChannel, true, ActorsToIgnore, DrawDebugType,
		CapsuleHits, bIgnoreSelf,
		FLinearColor(0.0f, 0.011018f, 0.083333f, 1.0f),
		FLinearColor(0.0f, 0.915124f, 1.0f, 1.0f),
		DrawTime);

	if(CapsuleHits.Num() == 0) { ++ReturnQueries; }

	for (const FHitResult& CapsuleWallTrace : CapsuleHits)
	{
		// BP liczy elementy wynikow Multi, nie wywolania trace'ow.
		++ReturnQueries;
		if (!CapsuleWallTrace.bBlockingHit || CapsuleWallTrace.bStartPenetrating)
		{
			continue;
		}

		//if (!UHelpfulFunctionsBPLibrary::ClassToIgnore(CapsuleWallTrace.GetActor()->GetClass(), ClassToIgnoreByLedgeC)) { continue; } //IGNORE CLASSES
		if (!CLASSTOIGNORESAFE(CapsuleWallTrace, ClassToIgnoreByLedgeC, CapsuleWallTrace.GetComponent())) { continue; } //IGNORE CLASSES

		const FVector UpTracingOffsetFromWallDirection = KML::Vector_SlerpNormals(
			UHelpfulFunctionsBPLibrary::NormalToVector(CapsuleWallTrace.Normal),
			SearchDirection,
			FMath::Abs(KML::Dot_VectorVector(
				CapsuleWallTrace.Normal, FVector::UpVector)));

		// 2. Pionowe trace'y wokol punktu kontaktu ze sciana.
		bool bBreakUpSearch = false;
		for (int32 I1 = 1; I1 <= UpTracesChecksNumber; ++I1)
		{
			const bool bPositiveOffset = ((I1 - 1) % 2) == 1;
			const double AlternatingOffset =
				UpTracingWallNormalSearchOffset * (I1 / 2 + 1)
				* (bPositiveOffset ? 1.0 : -1.0)
				+ (bPositiveOffset ? 0.0 : UpTracingWallNormalSearchOffset * 2.0);
			const double SearchOffset =
				(I1 == 1 ? UpTracingWallNormalSearchOffset : AlternatingOffset) - 2.0;

			FVector BlendImpactWithLocation = KML::VLerp(CapsuleWallTrace.ImpactPoint, CapsuleWallTrace.Location, 0.75f); BlendImpactWithLocation.Z = CapsuleWallTrace.ImpactPoint.Z;

			const FVector TraceCenter = BlendImpactWithLocation
				+ UpTracingOffsetFromWallDirection * SearchOffset;
			TArray<FHitResult> UpHits;
			KSL::LineTraceMulti(
				this,
				TraceCenter + FVector(0.0, 0.0, SurfaceLineTraceLength.X),
				TraceCenter - FVector(0.0, 0.0, SurfaceLineTraceLength.Y),
				TraceChannel, true, ActorsToIgnore, DrawDebugType,
				UpHits, bIgnoreSelf,
				KML::SelectColor(
					FLinearColor(1.0f, 0.323976f, 0.001167f, 1.0f),
					FLinearColor(0.296875f, 0.012598f, 0.0f, 1.0f),
					I1 == 1),
				FLinearColor(1.0f, 0.644887f, 0.0f, 1.0f),
				DrawTime);

			for (const FHitResult& UpHit : UpHits)
			{
				++ReturnQueries;
				if (UpHit.bBlockingHit
					&& ValidUpTraces.Num() <= MaxUpTracesCandidates
					&& !UpHit.bStartPenetrating
					&& FMath::Abs(KML::Dot_VectorVector(
						UpHit.Normal, FVector::UpVector)) > 0.4)
				{
					ValidUpTraces.Add(UpHit);
					// Array_Add prowadzi w BP prosto do Break petli I1.
					// Wewnetrzny ForEach nie ma pinu Break.
					bBreakUpSearch = true;
				}
			}
			if (bBreakUpSearch)
			{
				break;
			}
		}

		// 3. Ustalenie normalnej sciany przy znalezionej powierzchni.
		if (ValidUpTraces.Num() > 0)
		{
			for (const FHitResult& CurrentUpTraceChecking : ValidUpTraces)
			{
				const FVector WallSearchDirection = KML::Vector_SlerpNormals(UHelpfulFunctionsBPLibrary::NormalToVector(CurrentUpTraceChecking.Normal),
					UpTracingOffsetFromWallDirection, FMath::Abs(KML::Dot_VectorVector(CurrentUpTraceChecking.Normal, FVector::UpVector)));

				TArray<FHitResult> WallSphereHits;
				KSL::SphereTraceMulti(
					this,
					CurrentUpTraceChecking.ImpactPoint - WallSearchDirection * 15.0,
					CurrentUpTraceChecking.ImpactPoint + WallSearchDirection * 10.0,
					5.0f,
					TraceChannel, true, ActorsToIgnore, DrawDebugType,
					WallSphereHits, bIgnoreSelf,
					FLinearColor(0.153221f, 0.000133f, 0.234375f, 1.0f),
					FLinearColor(0.610169f, 0.368727f, 1.0f, 1.0f),
					WallSphereDrawTime);

				++ReturnQueries;

				if(WallSphereHits.Num() > 0)
				{
					FHitResult WallNormalSphereTrace = WallSphereHits[0];
					if (!WallNormalSphereTrace.bBlockingHit || WallNormalSphereTrace.bStartPenetrating || !(FMath::Abs(KML::Dot_VectorVector(WallNormalSphereTrace.Normal, FVector::UpVector)) < 0.6))
					{
						if (WallSphereHits.Num() >= 2)
						{
							WallNormalSphereTrace = WallSphereHits[1];
							if (!WallNormalSphereTrace.bBlockingHit || WallNormalSphereTrace.bStartPenetrating || !(FMath::Abs(KML::Dot_VectorVector(WallNormalSphereTrace.Normal, FVector::UpVector)) < 0.6))
							{ continue; }
						}
						else { continue; }
					}

					//if (!UHelpfulFunctionsBPLibrary::ClassToIgnore(WallNormalSphereTrace.GetActor()->GetClass(), ClassToIgnoreByLedgeC)) { continue; } //IGNORE CLASSES
					if (!CLASSTOIGNORESAFE(CapsuleWallTrace, ClassToIgnoreByLedgeC, CapsuleWallTrace.GetComponent())) //IGNORE CLASSES
					{
						continue;
					}

					const FVector InitialLedgePosition(WallNormalSphereTrace.ImpactPoint.X, WallNormalSphereTrace.ImpactPoint.Y, CurrentUpTraceChecking.ImpactPoint.Z);

#if WITH_ENGINE
					// Oddzielny, bezwarunkowy marker z BP, niezalezny od DrawDebugType.
					if (DrawAdditiveShapes)
					{
						KSL::DrawDebugSphere(this, InitialLedgePosition, 1.0f, 8, FLinearColor(1.0f, 0.0f, 0.069808f, 1.0f), 0.0f, 0.1f, EDrawDebugSceneDepthPriorityGroup::Foreground);
					}
#endif // WITH_ENGINE

					// 4. Sprawdzenie profilu krawedzi.
					const FVector InwardWallDirection = UHelpfulFunctionsBPLibrary::NormalToVector(WallNormalSphereTrace.Normal);
					const FVector PhaseNormalA = KML::Vector_SlerpNormals(WallNormalSphereTrace.Normal, CurrentUpTraceChecking.Normal, PhaseAnglesClamp.X);
					const FVector PhaseNormalB = KML::Vector_SlerpNormals(CurrentUpTraceChecking.Normal, WallNormalSphereTrace.Normal, PhaseAnglesClamp.Y);

					double TotalAngle = 0.0;
					bool bAnyPhaseCheckNotValid = false;
					for (int32 I2 = 0; I2 <= PhaseCheckNumber; ++I2)
					{
						const double Alpha = KML::MapRangeClamped(static_cast<double>(I2), 0.0, static_cast<double>(PhaseCheckNumber), 0.0, 1.0);
						const double OffsetDistance = KML::MapRangeClamped(static_cast<double>(I2), 0.0, static_cast<double>(PhaseCheckNumber), 2.0, 5.0);

						const FVector PhaseDirection = KML::Vector_SlerpNormals(PhaseNormalA, PhaseNormalB, Alpha);
						const FVector PhaseCenter = InitialLedgePosition + KML::Vector_SlerpNormals(FVector(0.0, 0.0, -1.0), InwardWallDirection, Alpha) * OffsetDistance;

						++ReturnQueries;
						FHitResult PhaseHit;
						const bool bPhaseHit = KSL::LineTraceSingle(
							this,
							PhaseCenter + PhaseDirection * 5.0,
							PhaseCenter - PhaseDirection * 5.0,
							TraceChannel, false, ActorsToIgnore, DrawDebugType,
							PhaseHit, bIgnoreSelf,
							FLinearColor(0.473958f, 0.443980f, 0.0f, 1.0f),
							FLinearColor(1.0f, 0.946486f, 0.623544f, 1.0f),
							DrawTime);

						if (!bPhaseHit || PhaseHit.bStartPenetrating)
						{
							// FunctionResult_1 zwraca pozycje, ale puste HitResult.
							PointTransform = KML::MakeTransform(InitialLedgePosition, FRotator::ZeroRotator, FVector::OneVector);
							bAnyPhaseCheckNotValid = true;
							break;
						}

						const bool bOddPhase = (I2 % 2) == 1;
						const FVector ReferenceNormal = bOddPhase ? InwardWallDirection : FVector::UpVector;
						// Zachowujemy znak i brak dodatkowego Clamp jak w BP.
						TotalAngle += KML::DegAcos( KML::Dot_VectorVector(PhaseHit.Normal, ReferenceNormal)) * (bOddPhase ? 1.0 : -1.0);
					}

					if (TotalAngle / static_cast<double>(PhaseCheckNumber + 1) < PhaseTollerance && bAnyPhaseCheckNotValid == false) //VALID RETURN <------------------------------------------------------------------------------------ ✔
					{
						PointTransform = KML::MakeTransform(InitialLedgePosition, KML::MakeRotFromX(InwardWallDirection), FVector::OneVector);
						WallHit = WallNormalSphereTrace;
						SurfaceHit = CurrentUpTraceChecking;
						return true;
					}
				}
				// Completed wewnetrznego ForEach -> FunctionResult_0.
				// To konczy cala funkcje, a nie tylko sprawdzanie kandydata.
				return false;
			}
		}
		// Completed ForEach ValidUpTraces -> FunctionResult_5.
		return false;
	}

	// Completed ForEach CapsuleHits -> FunctionResult_4.
	return false;
}



bool UCpp_DynamicClimbingComponent::VerifyLedgeGeneratedUsingComplexTraces(FTransform InLedgeLeft, FTransform InLedgeRight, UPARAM(ref)int32& CapsuleRoomQueries, FTransform& ReturnLedgeLeft, FTransform& ReturnLedgeRight, 
	FTransform& ReturnLedgeCenter, FTransform& ReturnCapsulePosition, UPrimitiveComponent*& ReturnComponent, TEnumAsByte<ECollisionChannel> Channel, double CapsuleRadius, double CapsuleHeight, double ForwardCapOffset, double CapsuleOffsetZ,
	FAGLS_LedgeFinderConfig_ComplexTracigMethod Settings, int DrawDebugTracesIndex, int brawDebugShapesIndex, float DrawDebugsTime)
{
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(Channel);

	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if(DrawDebugTracesIndex == 1) DrawDebugType = EDrawDebugTrace::ForOneFrame;
	if (DrawDebugTracesIndex >= 2) DrawDebugType = EDrawDebugTrace::ForDuration;
	//DEVELEPMENT OPTION
	bool bPrintStats = bPrintStringForComplexLedgeVerify;

	const TArray<AActor*> ActorsToIgnore = ActorsInstancesIgnoredByLedgeTraces;
	const float DrawTime = DrawDebugsTime;
	const bool bIgnoreSelf = true;

	// Wyjatki wizualne z BP: srodkowy trace rysuje sie przez jedna klatke,
	// a trace skroconej krawedzi ma DrawTime = 5 s.
	const EDrawDebugTrace::Type CenterDrawDebugType = DrawDebugType;
	const float ShortenedLedgeDrawTime = DrawTime * 2.0f;

	ReturnLedgeLeft = InLedgeLeft;
	ReturnLedgeRight = InLedgeRight;
	ReturnLedgeCenter = FTransform(KML::RLerp(InLedgeLeft.Rotator(), InLedgeRight.Rotator(), 0.5f, true), KML::VLerp(InLedgeLeft.GetLocation(), InLedgeRight.GetLocation(), 0.5f));
	ReturnCapsulePosition = FTransform::Identity;
	ReturnComponent = nullptr;
	// CapsuleRoomQueries jest licznikiem we/wy. Nie zerujemy go.

	const FTransform LeftTransform = InLedgeLeft;
	FTransform RightTransform = InLedgeRight;
	
	double LedgeLength = KML::Vector_Distance(LeftTransform.GetLocation(), RightTransform.GetLocation());

	// 1. Weryfikacja dlugosci, z opcjonalnym skroceniem prawego konca.
	if (LedgeLength > Settings.MaxLedgeLength + 2.0
		&& LedgeLength <= Settings.MaxLedgeLength * 2.0)
	{
		const FVector AlongLedge = KML::GetForwardVector(
			KML::FindLookAtRotation(
				LeftTransform.GetLocation(), RightTransform.GetLocation()));
		// Zachowujemy rotacje i skale prawego konca.
		RightTransform.SetLocation(
			LeftTransform.GetLocation() + AlongLedge * Settings.MaxLedgeLength);

		const FVector RightForward = KML::GetForwardVector(RightTransform.Rotator());
		FHitResult ShortenedLedgeHit;
		KSL::SphereTraceSingle(
			this,
			RightTransform.GetLocation() - RightForward * 9.0,
			RightTransform.GetLocation() + RightForward * 5.0,
			4.0f,
			TraceChannel, false, ActorsToIgnore, DrawDebugType,
			ShortenedLedgeHit, bIgnoreSelf,
			FLinearColor(0.3f, 0.3f, 0.3f, 1.0f),
			FLinearColor(1.0f, 1.0f, 0.5f, 1.0f),
			ShortenedLedgeDrawTime);

		if (!ShortenedLedgeHit.bBlockingHit || ShortenedLedgeHit.bStartPenetrating)
		{
			// FunctionResult_2: wszystkie wyjscia pozostaja puste.
			if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse durning cutting length TraceHit not Valid"));
			return false;
		}
		// BP po skroceniu przechodzi bezposrednio do Sequence.
		// Nie sprawdza ponownie MinLedgeLength.														// ┎━━━━━━━━━━━━━━━━ Min 2D Distance between Ledge Points
	}																									// ┃
																										// ┃
	LedgeLength = KML::Vector_Distance(LeftTransform.GetLocation(), RightTransform.GetLocation());		// ▼
	if (LedgeLength > Settings.MaxLedgeLength * 2.0 || LedgeLength < Settings.MinLedgeLength || KML::Vector_Distance2D(LeftTransform.GetLocation(), RightTransform.GetLocation()) < 5.0)
	{
		// FunctionResult_3: wszystkie wyjscia pozostaja puste.
		if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse length is not in accetable range"));
		return false;
	}

	// 2. Sequence, Then 0: roznica wysokosci i zgodnosc kierunkow.
	// Od tego etapu porazki zwracaja juz oba konce krawedzi.
	ReturnLedgeLeft = LeftTransform;
	ReturnLedgeRight = RightTransform;

	if (FMath::Abs(LeftTransform.GetLocation().Z - RightTransform.GetLocation().Z) 
	> KML::MapRangeClamped(LedgeLength, Settings.MinLedgeLength, Settings.MaxLedgeLength, Settings.MaxHeightDifferencyBetweenPoints.X, Settings.MaxHeightDifferencyBetweenPoints.Y))
	{
		if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse distance between Z axis > Tollerance"));
		return false; // FunctionResult_5.
	}

	const FVector LeftForward = KML::GetForwardVector(LeftTransform.Rotator());
	const FVector RightForward = KML::GetForwardVector(RightTransform.Rotator());
	if (!(FMath::Abs(KML::Dot_VectorVector(LeftForward, RightForward)) > 0.75))
	{
		if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse DOT between directions < 0.75"));
		return false; // FunctionResult_6; zachowany Abs z BP.
	}

	// 3. Sequence, Then 1: potwierdzenie powierzchni w srodku krawedzi.
	const FVector Midpoint = KML::VLerp(
		LeftTransform.GetLocation(), RightTransform.GetLocation(), 0.5);
	FHitResult CenterHit;
	const bool bCenterHit = KSL::SphereTraceSingle(
		this,
		Midpoint + FVector(0.0, 0.0, 3.0),
		Midpoint + FVector(0.0, 0.0, -5.0),
		4.5f,
		TraceChannel,
		false, ActorsToIgnore, CenterDrawDebugType,
		CenterHit, bIgnoreSelf,
		FLinearColor(0.08f, 0.02f, 0.008f, 1.0f),
		FLinearColor(1.0f, 0.3f, 0.0f, 1.0f),
		DrawTime);

	if (!bCenterHit || CenterHit.bStartPenetrating == false)
	{
		if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse on center SphereTrace not finded collision"));
		return false; // FunctionResult_7.
	}
	// BP nie odrzuca tutaj bStartPenetrating.
	UPrimitiveComponent* const CenterHitComponent = CenterHit.GetComponent();

	// 4. Sequence, Then 2: transformacja srodka z sama rotacja Yaw.
	const FVector LedgeRightDirection = KML::GetRightVector(KML::FindLookAtRotation(LeftTransform.GetLocation(), RightTransform.GetLocation()));
	const FVector AverageForward = KML::Vector_SlerpNormals(LeftForward, RightForward, 0.5);

	const double DirectionSign = KML::Dot_VectorVector(LedgeRightDirection, AverageForward) > 0.5 ? 1.0 : -1.0;
	const double CenterYaw = KML::MakeRotFromX(LedgeRightDirection * DirectionSign).Yaw;
	const FRotator CenterRotation(0.0, CenterYaw, 0.0);
	const FTransform LedgeCenter = KML::MakeTransform(Midpoint, CenterRotation, FVector::OneVector);

	const bool VerifyFreeSpaceForHands = true; //_______________________________________________________________________________
	if (VerifyFreeSpaceForHands)
	{
		FHitResult FreeSpaceForHandsHit;
		const bool SpaceForHandsValid = KSL::LineTraceSingle
		(
			this, 
			InLedgeLeft.GetLocation() + FVector(0, 0, 2), 
			InLedgeRight.GetLocation() + FVector(0, 0, 2), 
			UEngineTypes::ConvertToTraceType(ECC_Visibility), 
			false, 
			ActorsToIgnore, 
			DrawDebugType,
			FreeSpaceForHandsHit, 
			true, 
			FLinearColor(0.5f, 0.2f, 0.1f, 1.0f), 
			FLinearColor(1.0f, 0.4f, 0.2f, 1.0f), 
			DrawTime
		);
		if (SpaceForHandsValid) 
		{ 
			if (bPrintStats) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("VerifyLedgeGeneratedUsingComplexTraces() -> Ledge not valid becouse LineTrace above LedgePoints detect Collision on ECC_Visibility"));
			return false;
		}
	}

	// 5. Sequence, Then 3: pionowe przemiatanie kuli sprawdzajace miejsce.
	// CapsuleHeight jest uzywany jak w BP; nie dzielimy go przez dwa.
	if (Settings.bUseAdvancedCapsuleFreeSpaceFinder)
	{
		const FVector ComplexCapsulePosition = LedgeCenter.GetLocation() - KML::GetForwardVector(LedgeCenter.Rotator()) * (CapsuleRadius + ForwardCapOffset) + FVector(0.0, 0.0, CapsuleOffsetZ); //!!!!!

		double InputVolume = 0.0;
		double OutputVolume = 0.0;
		double VolumeDifference = 0.0;
		double ReductionPercent = 0.0;

		FHitResult CapsuleHit;
		FVector OutCapsuleCenter = FVector::ZeroVector;

		float OutRadius = 0.0f;
		float OutHalfHeight = 0.0f;
		float OutTopReduction = 0.0f;
		float OutBottomReduction = 0.0f;

		int32 OutQueriesUsed = 0;
		bool bOutQueryLimitReached = false;

		// W przeslanym BP wynik bool tej funkcji nie jest podlaczony.
		FindUnblockedCapsule(
			ComplexCapsulePosition,
			static_cast<float>(CapsuleRadius - Settings.PlayerCapsuleRadiusBias),
			static_cast<float>(CapsuleHeight - Settings.PlayerCapsuleHalfHeightBias),
			ActorsToIgnore,
			InputVolume,
			OutputVolume,
			VolumeDifference,
			ReductionPercent,
			CapsuleHit,
			OutCapsuleCenter,
			OutRadius,
			OutHalfHeight,
			OutTopReduction,
			OutBottomReduction,
			OutQueriesUsed,
			bOutQueryLimitReached,
			ECC_Visibility,
			true,																			// bTraceComplex
			true,																			// bIgnoreSelf
			Settings.MaxSingleCapsuleFreeSpaceTracesQueries,								// MaxTracesQueries
			true,																			// CanReduceRadius
			true,																			// CanReduceHeightFromTop
			true,																			// CanReduceHeightFromBottom
			8.0f,																			// RadiusReductionStep
			12.0f,																			// HeightReductionStep
			10.0f,																			// MinimumRadius
			0.5f,																			// CollisionPadding
			Settings.CapsuleFreeSpaceMaxTopReduction,										// MaxTopReduction
			Settings.CapsuleFreeSpaceMaxBottomReduction,									// MaxBottomReduction           
			FLinearColor::Black,															// TraceColor
			FLinearColor::Red,																// TraceHitColor
			0.5f);																			// DrawTime

		CapsuleRoomQueries += OutQueriesUsed;

		ReturnLedgeLeft = LeftTransform;
		ReturnLedgeRight = RightTransform;
		ReturnLedgeCenter = LedgeCenter;
		ReturnComponent = CenterHitComponent;

		// BP zwraca pozycje WEJSCIOWA, nie OutCapsuleCenter.
		ReturnCapsulePosition = KML::MakeTransform(
			ComplexCapsulePosition,
			LedgeCenter.Rotator(),
			FVector::OneVector);

		// Odpowiednik Select: przekroczony limit -> false.
		return !bOutQueryLimitReached
			&& OutHalfHeight > Settings.MinValidCapsuleOutHalfHeight
			&& OutTopReduction < Settings.MaxValidCapsuleReducedSize.Y
			&& OutBottomReduction < Settings.MaxValidCapsuleReducedSize.X
			&& OutRadius > Settings.MinValidCapsuleOutRadius;
	}
	else
	{
		const FVector CapsulePosition = LedgeCenter.GetLocation() - KML::GetForwardVector(CenterRotation) * ((CapsuleRadius) + ForwardCapOffset) + FVector(0.0, 0.0, CapsuleOffsetZ); //!!!!!

		const double CapsuleAxisOffset = (CapsuleHeight - Settings.PlayerCapsuleHalfHeightBias) - (CapsuleRadius - Settings.PlayerCapsuleRadiusBias);
		double ReductionCapsuleRadiusScale = 1.0;
		double ReduceCapsuleHeightDown = 0.0;

		for (int32 I1 = 0; I1 < Settings.CapsuleFreeSpaceCheckNumber; ++I1)
		{
			++CapsuleRoomQueries;
			FHitResult RoomHit;
			const bool bRoomBlocked = KSL::SphereTraceSingle(
				this,

				CapsulePosition + FVector(0.0, 0.0, CapsuleAxisOffset),
				CapsulePosition - FVector( 0.0, 0.0, CapsuleAxisOffset - ReduceCapsuleHeightDown),

				static_cast<float>((CapsuleRadius - Settings.PlayerCapsuleRadiusBias) * ReductionCapsuleRadiusScale),
				UEngineTypes::ConvertToTraceType(ECC_Visibility), true, ActorsToIgnore, DrawDebugType,
				RoomHit, bIgnoreSelf,
				FLinearColor(0.054437f, 0.307292f, 0.0f, 0.617391f),
				FLinearColor(1.0f, 0.0f, 0.006357f, 1.0f),
				DrawTime);

			if (!bRoomBlocked)
			{
				// FunctionResult_1: sukces oznacza brak kolizji przemiatania.
				ReturnLedgeCenter = LedgeCenter;
				ReturnCapsulePosition = KML::MakeTransform( CapsulePosition, CenterRotation, FVector::OneVector);
				ReturnComponent = CenterHitComponent;
				return true;
			}

			const FVector TraceCenter = KML::VLerp(RoomHit.TraceStart, RoomHit.TraceEnd, 0.5);
			const double HitHeightFromTraceCenter = RoomHit.ImpactPoint.Z - TraceCenter.Z;
			const bool bReduceRadius =
				KML::Dot_VectorVector(
					RoomHit.Normal,
					UHelpfulFunctionsBPLibrary::NormalToVector(RoomHit.Normal) * -1.0) > 0.75
				|| HitHeightFromTraceCenter > -15.0;

			if (bReduceRadius)
			{
				ReductionCapsuleRadiusScale = FMath::Clamp(
					ReductionCapsuleRadiusScale - Settings.ReduceCapsuleRadiusScaleBias, 0.6, 1.0);
			}
			else
			{
				// Skracamy tylko dolna czesc przemiatania; Start pozostaje staly.
				ReduceCapsuleHeightDown = FMath::Clamp(
					ReduceCapsuleHeightDown + Settings.ReduceCapsuleHeightBias,
					0.0, CapsuleAxisOffset / 2.0);
			}
		}

		// Completed -> FunctionResult_4, takze przy zerowej liczbie sprawdzen.
		// Then 4 z zewnetrznego Sequence jest nieosiagalny: petla zawsze zwraca wynik.
		ReturnLedgeCenter = LedgeCenter;
		ReturnComponent = CenterHitComponent;
		// ReturnCapsulePosition pozostaje Identity.
		return false;
	}
}


bool UCpp_DynamicClimbingComponent::FindUnblockedCapsule( FVector CapsuleCenter, float Radius, float HalfHeight, const TArray<AActor*>& ActorsToIgnore, double& InputVolume, double& OutputVolume, double& VolumeDifference,
	double& ReductionPercent, FHitResult& OutHit, FVector& OutCapsuleCenter, float& OutRadius, float& OutHalfHeight, float& OutTopReduction, float& OutBottomReduction, int32& OutQueriesUsed, bool& bOutQueryLimitReached,
	TEnumAsByte<ECollisionChannel> Channel,
	bool bTraceComplex,
	bool bIgnoreSelf,
	int32 MaxTracesQueries,
	bool CanReduceRadius,
	bool CanReduceHeightFromTop,
	bool CanReduceHeightFromBottom,
	float RadiusReductionStep,
	float HeightReductionStep,
	float MinimumRadius,
	float CollisionPadding,
	float MaxTopReduction,
	float MaxBottomReduction,
	FLinearColor TraceColor,
	FLinearColor TraceHitColor,
	float DrawTime)
{
	InputVolume = 0.0;
	OutputVolume = 0.0;
	VolumeDifference = 0.0;
	ReductionPercent = 0.0;
	OutHit = FHitResult();
	OutCapsuleCenter = FVector::ZeroVector;
	OutRadius = 0.0f;
	OutHalfHeight = 0.0f;
	OutTopReduction = 0.0f;
	OutBottomReduction = 0.0f;
	OutQueriesUsed = 0;
	bOutQueryLimitReached = false;

	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if (DebugCapsuleSpaceVerfy == 1)  DrawDebugType = EDrawDebugTrace::ForOneFrame;
	else if(DebugCapsuleSpaceVerfy == 2)  DrawDebugType = EDrawDebugTrace::ForDuration;


	// HalfHeight obejmuje polkule. Nie naprawiamy blednych wymiarow po cichu.
	if (!FMath::IsFinite(CapsuleCenter.X)
		|| !FMath::IsFinite(CapsuleCenter.Y)
		|| !FMath::IsFinite(CapsuleCenter.Z)
		|| !FMath::IsFinite(Radius) || Radius <= 0.0f
		|| !FMath::IsFinite(HalfHeight) || HalfHeight < Radius)
	{
		return false;
	}

	const auto CapsuleVolume = [](float R, float H) -> double
		{
			constexpr double Pi = 3.14159265358979323846;
			const double RD = static_cast<double>(R);
			const double CylinderHeight = 2.0 * (static_cast<double>(H) - RD);
			return Pi * RD * RD * CylinderHeight + (4.0 / 3.0) * Pi * RD * RD * RD;
		};
	InputVolume = CapsuleVolume(Radius, HalfHeight);

	if (MaxTracesQueries <= 0
		|| !GetWorld()
		|| Channel.GetValue() >= ECC_MAX
		|| !FMath::IsFinite(DrawTime) || DrawTime < 0.0f
		|| !FMath::IsFinite(CollisionPadding) || CollisionPadding <= 0.0f)
	{
		return false;
	}

	if (CanReduceRadius
		&& (!FMath::IsFinite(RadiusReductionStep) || RadiusReductionStep <= 0.0f
			|| !FMath::IsFinite(MinimumRadius) || MinimumRadius <= 0.0f
			|| MinimumRadius > Radius))
	{
		return false;
	}
	if ((CanReduceHeightFromTop || CanReduceHeightFromBottom)
		&& (!FMath::IsFinite(HeightReductionStep) || HeightReductionStep <= 0.0f))
	{
		return false;
	}

	// -1 oznacza brak dodatkowego limitu poza poprawna geometria kapsuly.
	if ((CanReduceHeightFromTop
		&& (!FMath::IsFinite(MaxTopReduction)
			|| (MaxTopReduction < 0.0f && MaxTopReduction != -1.0f)))
		|| (CanReduceHeightFromBottom
			&& (!FMath::IsFinite(MaxBottomReduction)
				|| (MaxBottomReduction < 0.0f && MaxBottomReduction != -1.0f))))
	{
		return false;
	}

	const ETraceTypeQuery TraceChannel =
		UEngineTypes::ConvertToTraceType(Channel.GetValue());
	if (TraceChannel == TraceTypeQuery_MAX)
	{
		return false;
	}

	const int32 QueryBudget = FMath::Min(MaxTracesQueries, 12);
	const double SideNormalThreshold = 0.75;
	const double PositionTolerance = 1.e-4;
	float CurrentRadius = Radius;
	float CurrentHalfHeight = HalfHeight;
	FVector CurrentCenter = CapsuleCenter;
	double TotalTopCut = 0.0;
	double TotalBottomCut = 0.0;

	enum class EMethod { Radius, Height };
	enum class EHeightSide { Unset, Top, Bottom };
	EMethod LastMethod = EMethod::Radius;
	EHeightSide LockedHeightSide = EHeightSide::Unset;
	int32 AppliedReductions = 0;

	const auto FiniteVector = [](const FVector& V)
		{
			return FMath::IsFinite(V.X) && FMath::IsFinite(V.Y) && FMath::IsFinite(V.Z);
		};

	while (OutQueriesUsed < QueryBudget)
	{
		const double R = CurrentRadius;
		const double H = CurrentHalfHeight;
		const double CylinderHalfHeight = H - R;
		const FVector TraceStart = CurrentCenter + FVector(0.0, 0.0, CylinderHalfHeight);
		const FVector TraceEnd = CurrentCenter - FVector(0.0, 0.0, CylinderHalfHeight);
		FHitResult Hit;
		++OutQueriesUsed;
		const bool bHit = KSL::SphereTraceSingle(
			this, TraceStart, TraceEnd, CurrentRadius,
			TraceChannel, bTraceComplex, ActorsToIgnore, DrawDebugType,
			Hit, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);

		if (!(bHit || Hit.bBlockingHit || Hit.bStartPenetrating))
		{
			OutputVolume = CapsuleVolume(CurrentRadius, CurrentHalfHeight);
			VolumeDifference = FMath::Max(0.0, InputVolume - OutputVolume);
			ReductionPercent = 100.0 * VolumeDifference / InputVolume;
			OutCapsuleCenter = CurrentCenter;
			OutRadius = CurrentRadius;
			OutHalfHeight = CurrentHalfHeight;
			OutTopReduction = static_cast<float>(TotalTopCut);
			OutBottomReduction = static_cast<float>(TotalBottomCut);
			return true; // OutHit nadal oznacza ostatnie BLOKUJACE trafienie.
		}
		OutHit = Hit;

		FVector N = FiniteVector(Hit.Normal) ? Hit.Normal.GetSafeNormal() : FVector::ZeroVector;
		if (N.IsNearlyZero() && FiniteVector(Hit.ImpactNormal))
		{
			N = Hit.ImpactNormal.GetSafeNormal();
		}
		const bool bHaveNormal = !N.IsNearlyZero();
		const bool bSideNormal = bHaveNormal && FMath::Abs(N.Z) <= SideNormalThreshold;

		FVector ContactPoint = Hit.ImpactPoint;
		bool bHavePoint = !Hit.bStartPenetrating && FiniteVector(ContactPoint);
		if (Hit.bStartPenetrating && bHaveNormal
			&& FMath::IsFinite(Hit.PenetrationDepth) && Hit.PenetrationDepth > 0.0f)
		{
			// ImpactPoint przy initial overlap moze byc niepoprawny.
			// Przyblizamy kontakt z MTD kuli w TraceStart; to estymacja.
			ContactPoint = TraceStart - N * (R - Hit.PenetrationDepth);
			bHavePoint = FiniteVector(ContactPoint);
		}
		const FVector ContactOffset = bHavePoint ? ContactPoint - CurrentCenter : FVector::ZeroVector;
		const double Distance2D = bHavePoint ? ContactOffset.Size2D() : 0.0;
		const bool bInCylinder = bHavePoint && CylinderHalfHeight > PositionTolerance
			&& FMath::Abs(ContactOffset.Z) <= CylinderHalfHeight + PositionTolerance;

		const double GeometricHeightCapacity =
			2.0 * static_cast<double>(CylinderHalfHeight);

		const double RemainingTopReduction = MaxTopReduction < 0.0f
			? GeometricHeightCapacity
			: static_cast<double>(MaxTopReduction)
			- static_cast<double>(TotalTopCut);

		const double RemainingBottomReduction = MaxBottomReduction < 0.0f
			? GeometricHeightCapacity
			: static_cast<double>(MaxBottomReduction)
			- static_cast<double>(TotalBottomCut);

		const double TopCapacity = CanReduceHeightFromTop
			? FMath::Max<double>(
				0.0,
				FMath::Min<double>(
					GeometricHeightCapacity,
					RemainingTopReduction))
			: 0.0;

		const double BottomCapacity = CanReduceHeightFromBottom
			? FMath::Max<double>(
				0.0,
				FMath::Min<double>(
					GeometricHeightCapacity,
					RemainingBottomReduction))
			: 0.0;

		// Strone wysokosci blokujemy dopiero po jej pierwszej rzeczywistej zmianie.
		EHeightSide ProposedHeightSide = LockedHeightSide;
		if (ProposedHeightSide == EHeightSide::Unset)
		{
			const bool bPreferTop = bHavePoint && FMath::Abs(ContactOffset.Z) > PositionTolerance
				? ContactOffset.Z > 0.0
				: (bHaveNormal && N.Z < 0.0);
			ProposedHeightSide = bPreferTop ? EHeightSide::Top : EHeightSide::Bottom;
			if (ProposedHeightSide == EHeightSide::Top && TopCapacity <= 0.0)
			{
				ProposedHeightSide = EHeightSide::Bottom;
			}
			else if (ProposedHeightSide == EHeightSide::Bottom && BottomCapacity <= 0.0)
			{
				ProposedHeightSide = EHeightSide::Top;
			}
		}
		const double HeightCapacity = ProposedHeightSide == EHeightSide::Top
			? TopCapacity : BottomCapacity;
		const bool bHeightAvailable = HeightCapacity > 0.0;

		// Ponizej MinimumRadius nie wykonujemy nawet proby redukcji promienia.
		// Poza cylindrem wybieramy wysokosc, jesli nadal jest dozwolona/dostepna.
		const bool bRadiusAvailable = CanReduceRadius && R > MinimumRadius
			&& (!bHavePoint || Distance2D >= MinimumRadius)
			&& (!bHavePoint || bInCylinder || !bHeightAvailable);

		EMethod DesiredMethod;
		if (AppliedReductions == 0)
		{
			DesiredMethod = bSideNormal && bInCylinder && bRadiusAvailable
				? EMethod::Radius : EMethod::Height;
		}
		else if (AppliedReductions == 1)
		{
			DesiredMethod = LastMethod; // Po drugim trace drugi raz ta sama metoda.
		}
		else
		{
			DesiredMethod = LastMethod == EMethod::Radius ? EMethod::Height : EMethod::Radius;
		}

		// Wyliczamy obie propozycje bez dodatkowych trace'ow. Harmonogram ustala
		// priorytet; strefa, limity i flagi moga wymusic dostepna alternatywe.
		float ProposedRadius = CurrentRadius;
		if (bRadiusAvailable)
		{
			double TargetRadius = bHavePoint
				? FMath::Clamp(Distance2D - CollisionPadding,
					static_cast<double>(MinimumRadius), R)
				: FMath::Max(static_cast<double>(MinimumRadius), R - RadiusReductionStep);
			// Gdy kontakt nie daje mniejszego promienia, nie testujemy tej samej bryly.
			if (static_cast<float>(TargetRadius) >= CurrentRadius)
			{
				TargetRadius = FMath::Max(static_cast<double>(MinimumRadius), R - RadiusReductionStep);
			}
			ProposedRadius = static_cast<float>(TargetRadius);
		}

		float ProposedHalfHeight = CurrentHalfHeight;
		if (bHeightAvailable)
		{
			// Przesuwamy wybrany zewnetrzny skraj za ImpactPoint + margines.
			// Gora: Top' = ContactZ - Padding. Dol: Bottom' = ContactZ + Padding.
			double Cut = bHavePoint
				? (ProposedHeightSide == EHeightSide::Top
					? H - ContactOffset.Z + CollisionPadding
					: H + ContactOffset.Z + CollisionPadding)
				: static_cast<double>(HeightReductionStep);
			if (Cut <= 0.0)
			{
				Cut = HeightReductionStep;
			}
			Cut = FMath::Clamp(Cut, 0.0, HeightCapacity);
			ProposedHalfHeight = static_cast<float>(FMath::Max(R, H - Cut * 0.5));
			// Round-to-nearest float nie moze przekroczyc limitu lacznego skrocenia.
			if (2.0 * (H - ProposedHalfHeight) > HeightCapacity)
			{
				ProposedHalfHeight = std::nextafter(ProposedHalfHeight, CurrentHalfHeight);
			}
			if (ProposedHalfHeight >= CurrentHalfHeight)
			{
				Cut = FMath::Min(static_cast<double>(HeightReductionStep), HeightCapacity);
				ProposedHalfHeight = static_cast<float>(FMath::Max(R, H - Cut * 0.5));
				if (2.0 * (H - ProposedHalfHeight) > HeightCapacity)
				{
					ProposedHalfHeight = std::nextafter(ProposedHalfHeight, CurrentHalfHeight);
				}
			}
		}
		const bool bCanApplyRadius = ProposedRadius < CurrentRadius;
		const bool bCanApplyHeight = ProposedHalfHeight < CurrentHalfHeight;
		EMethod ActualMethod = DesiredMethod;
		if (ActualMethod == EMethod::Radius && !bCanApplyRadius)
		{
			ActualMethod = EMethod::Height;
		}
		else if (ActualMethod == EMethod::Height && !bCanApplyHeight)
		{
			ActualMethod = EMethod::Radius;
		}
		if ((ActualMethod == EMethod::Radius && !bCanApplyRadius)
			|| (ActualMethod == EMethod::Height && !bCanApplyHeight))
		{
			return false; // Brak dopuszczalnego postepu; nie zmieniamy zablokowanej strony.
		}
		if (OutQueriesUsed >= QueryBudget)
		{
			bOutQueryLimitReached = true;
			return false;
		}

		if (ActualMethod == EMethod::Radius)
		{
			CurrentRadius = ProposedRadius;
		}
		else
		{
			const double AppliedCut = 2.0 * (H - ProposedHalfHeight);
			CurrentHalfHeight = ProposedHalfHeight;
			LockedHeightSide = ProposedHeightSide;
			if (LockedHeightSide == EHeightSide::Top)
			{
				TotalTopCut += AppliedCut;
				CurrentCenter.Z -= AppliedCut * 0.5;
			}
			else
			{
				TotalBottomCut += AppliedCut;
				CurrentCenter.Z += AppliedCut * 0.5;
			}
		}
		LastMethod = ActualMethod;
		++AppliedReductions;
	}
	return false;
}



bool UCpp_DynamicClimbingComponent::TryCreateLedgeUsingComplexTracesMethod(FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, int& TotalTracesQueries, const TArray<UClass*>& ClassToIgnoreByTraces,
	ACharacter* InCharacter, FVector TraceOrigin, FVector TraceDirection, float InCapsuleRadius, float InCapsuleHalfHeight, float CapsuleOffsetFromWall, float CapsuleOffsetFromLedgeUpAxis, TEnumAsByte<ECollisionChannel> TracesChannel, 
	FAGLS_LedgeFinderConfig_ComplexTracigMethod SolverSettings, bool CanDrawDebugShapes, bool CanDrawDebugTraces, float DrawDebugTime)
{
	TotalTracesQueries = 0;
	if (!InCharacter) return false;

	const FVector InTracingRightVector = KML::GetRightVector(KML::MakeRotFromX(TraceDirection));
	int RightQueriesCounter = 0;
	int LeftQueriesCounter = 0;
	int TotalQueriesCounter = 0;
	int CapsuleRoomQueriesCounter = 0;

	//DEBUG CONFIG
	int LedgeGenDebugIndex = 0; 
	if (CanDrawDebugShapes && CanDrawDebugTraces) { LedgeGenDebugIndex = KML::SelectInt(4, 3, DrawDebugTime > 0.0); }
	else if (CanDrawDebugTraces) { LedgeGenDebugIndex = KML::SelectInt(2, 1, DrawDebugTime > 0.0); }
	//END

	for (int i_Left = 0; i_Left <= SolverSettings.SingleLedgePointFinderMaxCalls - 1; i_Left++)
	{
		FVector OriginOffsetLeft = InTracingRightVector *
			(KML::MapRangeClamped((float)i_Left, 0.0, (float)(SolverSettings.SingleLedgePointFinderMaxCalls - 1), SolverSettings.MaxLedgeLength, SolverSettings.MinLedgeLength) * -0.5f);

		const FRotator TracingRotator = KML::MakeRotFromX(TraceDirection);

		FTransform LeftLedgePoint; FHitResult WallHitLeft; FHitResult SurfaceHitLeft; int LeftLedgeQueries;
		const bool LeftPointValid = FindSingleLedgePointUsingComplexTraces
		(
			InCharacter, 
			LeftLedgePoint, 
			WallHitLeft, 
			SurfaceHitLeft, 
			LeftLedgeQueries, 
			TraceOrigin + (KML::GetForwardVector(TracingRotator) * SolverSettings.SearchingPointsOriginOffset.X) + (KML::GetUpVector(TracingRotator) * SolverSettings.SearchingPointsOriginOffset.Z) + OriginOffsetLeft,
			TraceDirection, 
			TracesChannel, 
			SolverSettings.MaxLedgeLength, 
			SolverSettings.MinLedgeLength, 
			SolverSettings.SearcherForwardWallLength, 
			KML::MapRangeClamped((float)i_Left, 0.0, (float)(i_Left < SolverSettings.SingleLedgePointFinderMaxCalls - 1), SolverSettings.SearcherForwardWallRadius, SolverSettings.SearcherForwardWallRadius * 0.75f),
			SolverSettings.SearcherUpTracesChecksNumber, 
			SolverSettings.SearcherMaxUpTracesCandidates, 
			SolverSettings.SearcherPhaseCheckTracesCount, 
			SolverSettings.SearcherPhaseTollerance, 
			SolverSettings.SearcherSurfaceLineTraceLength,
			LedgeGenDebugIndex,
			DrawDebugTime
		);
		LeftQueriesCounter = LeftQueriesCounter + LeftLedgeQueries;

		if (LeftPointValid)
		{
			const FVector WallHitRightVector = KML::GetRightVector(KML::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(WallHitLeft.Normal)));
			FVector RightTraceOrigin = LeftLedgePoint.GetLocation() + (WallHitRightVector * SolverSettings.MaxLedgeLength) + (UHelpfulFunctionsBPLibrary::NormalToVector(WallHitLeft.Normal) * -50.0f);
			RightTraceOrigin = RightTraceOrigin + FVector(0, 0, 10);
			RightTraceOrigin.Z = KML::Lerp(RightTraceOrigin.Z, TraceOrigin.Z, 0.5f);

			const FVector TracingDirectionRight = KML::Vector_SlerpNormals(TraceDirection, UHelpfulFunctionsBPLibrary::NormalToVector(WallHitLeft.Normal), 0.8f);

			for (int i_RightSub = 0; i_RightSub < 2; i_RightSub++)
			{
				for (int i_Right = 0; i_Right <= SolverSettings.SingleLedgePointFinderMaxCalls - 1; i_Right++)
				{
					FVector RightOffseting = KML::Vector_SlerpNormals(InTracingRightVector, WallHitRightVector, 0.8f) *
						((KML::MapRangeClamped((float)i_Right, 0.0, (float)(SolverSettings.SingleLedgePointFinderMaxCalls - 1), 0.0f, SolverSettings.MaxLedgeLength - SolverSettings.MinLedgeLength) * -0.5f) +
						((float)i_RightSub * (SolverSettings.MaxLedgeLength - SolverSettings.MinLedgeLength) * -0.5f));

					//DrawDebugLine(GetWorld(), RightTraceOrigin + RightOffseting, RightTraceOrigin + RightOffseting + (TracingDirectionRight * 20), FColor::Yellow, false, 0.0f, -1, 0.5);

					FTransform RightLedgePoint; FHitResult WallHitRight; FHitResult SurfaceHitRight; int RightLedgeQueries;
					const bool RightPointValid = FindSingleLedgePointUsingComplexTraces
					(
						InCharacter,
						RightLedgePoint,
						WallHitRight,
						SurfaceHitRight,
						RightLedgeQueries,
						RightTraceOrigin + RightOffseting,
						TracingDirectionRight,
						TracesChannel,
						SolverSettings.MaxLedgeLength,
						SolverSettings.MinLedgeLength,
						SolverSettings.SearcherForwardWallLength,
						KML::MapRangeClamped((float)i_Right, 0.0, (float)(i_Right < SolverSettings.SingleLedgePointFinderMaxCalls - 1), SolverSettings.SearcherForwardWallRadius, SolverSettings.SearcherForwardWallRadius * 0.75f),
						SolverSettings.SearcherUpTracesChecksNumber,
						SolverSettings.SearcherMaxUpTracesCandidates,
						SolverSettings.SearcherPhaseCheckTracesCount,
						SolverSettings.SearcherPhaseTollerance,
						SolverSettings.SearcherSurfaceLineTraceLength,
						LedgeGenDebugIndex,
						DrawDebugTime
					);
					RightQueriesCounter = RightQueriesCounter + RightLedgeQueries;
					//GEngine->AddOnScreenDebugMessage(-1, 0.1, FColor::Cyan, FString::FromInt(i_Right) + " " + FString::FromInt(RightQueriesCounter));

					// Weryfikacja poprawności półki oraz sprawdzenie czy kapsuła się mieści
					if (RightPointValid && LeftPointValid)
					{
						FTransform LedgeCenter; FTransform CapsuleDesiredPosition; UPrimitiveComponent* CenterHitComponent;
						const bool LedgeVerifyValid = VerifyLedgeGeneratedUsingComplexTraces
						(
							LeftLedgePoint, 
							RightLedgePoint, 
							CapsuleRoomQueriesCounter, 
							LeftLedgePoint, 
							RightLedgePoint, 
							LedgeCenter, 
							CapsuleDesiredPosition, 
							CenterHitComponent, 
							TracesChannel, 
							InCapsuleRadius, 
							InCapsuleHalfHeight, 
							CapsuleOffsetFromWall, 
							CapsuleOffsetFromLedgeUpAxis, 
							SolverSettings
						);

						bool a = true;
						if (LedgeVerifyValid && CenterHitComponent && KML::Vector_Distance(LeftLedgePoint.GetLocation(), CapsuleDesiredPosition.GetLocation()) < 400) // THIS IS OUTPUT WITH VALID LEDGE STRUCTURE
						{
							LeftPoint.ValidPoint = true;
							LeftPoint.Location = LeftLedgePoint.GetLocation();
							LeftPoint.Normal = KML::GetForwardVector(LeftLedgePoint.Rotator());
							LeftPoint.Component = CenterHitComponent;
							//if (IsValid(CenterHitComponent->GetOwner())) { LeftPoint.ActorTransform = CenterHitComponent->GetOwner()->GetTransform(); }

							RightPoint.ValidPoint = true;
							RightPoint.Location = RightLedgePoint.GetLocation();
							RightPoint.Normal = KML::GetForwardVector(RightLedgePoint.Rotator());
							RightPoint.Component = CenterHitComponent;
							//if (IsValid(CenterHitComponent->GetOwner())) { RightPoint.ActorTransform = CenterHitComponent->GetOwner()->GetTransform(); }

							OriginPoint.ValidPoint = true;
							OriginPoint.Location = LedgeCenter.GetLocation();
							OriginPoint.Normal = KML::GetForwardVector(LedgeCenter.Rotator());
							OriginPoint.Component = CenterHitComponent;
							//if (IsValid(CenterHitComponent->GetOwner())) { OriginPoint.ActorTransform = CenterHitComponent->GetOwner()->GetTransform(); }

							TotalTracesQueries = LeftQueriesCounter + RightQueriesCounter + CapsuleRoomQueriesCounter;
							return true;

						}
					}

					//Optimalization
					if (RightQueriesCounter > SolverSettings.MaxLedgeFindingTracesQueries) { return false; }
					if (CapsuleRoomQueriesCounter > SolverSettings.MaxCapsuleTotalRoomQueriesPerLedgeGen) { return false; }
					
				}
			}
			TotalTracesQueries = LeftQueriesCounter + RightQueriesCounter + CapsuleRoomQueriesCounter;
			return false;
		}
		//Optimalization
		if (LeftQueriesCounter + RightQueriesCounter + CapsuleRoomQueriesCounter > SolverSettings.MaxTotalTracesQueries) { return false; }
	}
	TotalTracesQueries = LeftQueriesCounter + RightQueriesCounter + CapsuleRoomQueriesCounter;
	return false;
}


#pragma endregion



bool UCpp_DynamicClimbingComponent::DetermineWallPositionAndFindLedgeDuringTheFall_Implementation(FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& NoEvenWallHit, 
	int& CheckedLedgesNumber, FVector InitialCheckPosition, FVector InitialCheckDirection, float LedgeCheckOffsetZ, float LedgeCheckForwardWallTraceScale, bool CanUsePredictFallPosition, int MaxLedgeCheckExecute, 
	int MaxTotalTracesQueries, int DrawAdditiveTracesIndex)
{
	// Replace this with your BP_BeamForSwinging class reference.
	TSubclassOf<AActor> BeamActorClass = BeamForSwingingIdentifyClass;

	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;
	if (DebugTraceIndexC == 1) DrawDebugType = EDrawDebugTrace::ForOneFrame;
	if (DebugTraceIndexC >= 2) DrawDebugType = EDrawDebugTrace::ForDuration;

	// Preserve the channel identifiers serialized in this particular graph.
	const ECollisionChannel ProjectileTraceChannel = ForClimbingChannelC;
	const ETraceTypeQuery WallTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const ETraceTypeQuery CapsuleTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const TArray<TEnumAsByte<EObjectTypeQuery>> BeamObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Destructible) };
	const TArray<AActor*> ActorsToIgnore;
	const TArray<AActor*> CapsuleActorsToIgnore = { CharacterC };
	const bool bIgnoreSelf = true;
	const float WallDrawTime = 0.1f;
	const float OverheadDrawTime = 2.0f;
	const float CapsuleDrawTime = 1.0f;

	(void)LedgeCheckForwardWallTraceScale;
	//(void)MaxLedgeCheckExecute;
	//(void)DrawAdditiveTracesIndex;

	LeftPoint = FCMC_SingleClimbPointC{};
	RightPoint = FCMC_SingleClimbPointC{};
	OriginPoint = FCMC_SingleClimbPointC{};
	NoEvenWallHit = true;
	CheckedLedgesNumber = 1;

	if (!IsValid(CharacterC))
	{
		return false;
	}

	FHitResult FirstWallHit;
	int32 I1 = 0;
	int32 AllTracesFromLedgeGen = 0;
	const FVector DefScale = FVector::OneVector;
	TArray<FCMC_LedgeC> ValidLedgeResult;
	TArray<double> ValidLedgesWeight;
	FCMC_LedgeC CurrentLedgeStruct{};
	double CurrentWeightCalculation = 0.0;
	float MaxSimTime = 0.5f;
	const FVector BeamFindingExtend(120.0, 20.0, 100.0);
	AActor* FindedBeamActor = nullptr;
	FVector BeamForSwingingPosition = FVector::ZeroVector;

	// Read modifiers at the same execution stages as the pure BP nodes.
	// Do not cache them across TryCreateLedgeStructureC calls.
	struct FWallDetectionModifiersLocal
	{
		int32 AdditiveLedgeCheckInterations = 0;
		float OverrideSimTime = -1.0f;
		bool bCanSearchForBeamForSwinging = false;
		float AimWallDetectionTraceOnBeamPosition = 0.5f;
		int32 OverrideWallDetectionMode = -1;
		int32 AddMaxTotalTracesCallPerExecution = 0;
	};

	const auto ReadModifiers = [this]()
		{
			FWallDetectionModifiersLocal Result;
			GetModifyParametersForWallDetection(
				Result.AdditiveLedgeCheckInterations,
				Result.OverrideSimTime,
				Result.bCanSearchForBeamForSwinging,
				Result.AimWallDetectionTraceOnBeamPosition,
				Result.OverrideWallDetectionMode,
				Result.AddMaxTotalTracesCallPerExecution);
			return Result;
		};

	// ASSUMPTION: the custom SmallRangeClamp macro is MapRangeClamped.
	// Its implementation is not included in the supplied node export.
	const auto SmallRangeClamp = [](double Value, double InMin, double InMax,
		double OutMin, double OutMax) -> double
		{
			return KML::MapRangeClamped(Value, InMin, InMax, OutMin, OutMax);
		};

	const FWallDetectionModifiersLocal TimeModifiers = ReadModifiers();
	if (TimeModifiers.OverrideSimTime > 0.0f)
	{
		MaxSimTime = TimeModifiers.OverrideSimTime;
	}

	// Sequence 0, Then 0: optional oriented overlap for a swinging beam.
	if (ReadModifiers().bCanSearchForBeamForSwinging)
	{
		const FVector BeamDirection = KML::Vector_SlerpNormals(
			InitialCheckDirection, CharacterC->GetActorForwardVector(), 0.6);
		const FRotator BeamRotation = KML::MakeRotFromX(BeamDirection);
		const FVector BeamBoxPosition = InitialCheckPosition
			+ FVector(0.0, 0.0, -20.0)
			+ KML::ClampVectorSize(
				CharacterC->GetVelocity() * (static_cast<double>(MaxSimTime) * 0.2),
				0.0, 150.0)
			+ BeamDirection * (BeamFindingExtend.X * 0.5);

		TArray<AActor*> OverlappingActors;
		if (KSL::BoxOverlapActorsWithOrientation(this, BeamBoxPosition, BeamFindingExtend, BeamRotation, BeamObjectTypes, BeamActorClass.Get(), ActorsToIgnore, OverlappingActors))
		{
			float NearestDistance = 0.0f;
			FindedBeamActor = UGameplayStatics::FindNearestActor(InitialCheckPosition, OverlappingActors, NearestDistance);

			if (IsValid(FindedBeamActor))
			{
				const FTransform SearchTransform(BeamRotation, InitialCheckPosition, DefScale);
				const FVector RelativeBeamLocation = KML::MakeRelativeTransform(FindedBeamActor->GetActorTransform(), SearchTransform).GetLocation();

				FVector AxisX, AxisY, AxisZ;
				KML::GetAxes(BeamRotation, AxisX, AxisY, AxisZ);
				BeamForSwingingPosition = InitialCheckPosition
					+ AxisX * (RelativeBeamLocation.X * 1.1)
					+ AxisY * (RelativeBeamLocation.Y * 0.2)
					+ AxisZ * (RelativeBeamLocation.Z * 1.1);
			}
		}
	}

	// Sequence 0, Then 1: choose projectile prediction or a straight sphere sweep.
	const int32 WallDetectionMode = ReadModifiers().OverrideWallDetectionMode;
	const bool bUsePrediction = WallDetectionMode == -1
		? CanUsePredictFallPosition
		: WallDetectionMode == 0;

	const FVector Velocity = CharacterC->GetVelocity();
	const FVector RedirectedVelocity = KML::GreaterGreater_VectorRotator(
		KML::LessLess_VectorRotator( Velocity, KML::MakeRotFromX(KML::Normal(Velocity, 0.0001f))),
		KML::MakeRotFromX(InitialCheckDirection));

	++AllTracesFromLedgeGen;
	if (bUsePrediction)
	{
		const FVector Start = InitialCheckPosition + FVector(0.0, 0.0, SmallRangeClamp(KML::VSizeXY(Velocity), 100.0, 300.0, 40.0, 0.0));
		const FVector ClampedVelocity = KML::ClampVectorSize(RedirectedVelocity * static_cast<double>(1.1f), 300.0, 1200.0);
		const FVector LaunchVelocity(ClampedVelocity.X, ClampedVelocity.Y, Velocity.Z + 50.0);

		TArray<FVector> PathPositions;
		FVector LastTraceDestination = FVector::ZeroVector;
		UGameplayStatics::Blueprint_PredictProjectilePath_ByTraceChannel(
			this, FirstWallHit, PathPositions, LastTraceDestination,
			Start, LaunchVelocity, true, 20.0f, ProjectileTraceChannel,
			true, ActorsToIgnore, 
			DrawAdditiveTracesIndex > 0 ? DrawDebugType : EDrawDebugTrace::None, 
			WallDrawTime, 15.0f, MaxSimTime, 0.0f);
	}
	else
	{
		const FVector Start = InitialCheckPosition + FVector(0.0, 0.0, SmallRangeClamp(KML::VSizeXY(Velocity), 100.0, 300.0, 60.0, 0.0));
		const FVector ClampedOffset = KML::ClampVectorSize(RedirectedVelocity * static_cast<double>(MaxSimTime), 150.0, 300.0);
		// ASSUMPTION: GetFromVectorZ(Velocity) is Velocity.Z.
		const FVector DefaultEnd = InitialCheckPosition + FVector(ClampedOffset.X, ClampedOffset.Y, Velocity.Z * 0.5);
		const FWallDetectionModifiersLocal AimModifiers = ReadModifiers();

		const double AimAlpha = IsValid(FindedBeamActor)
			? static_cast<double>(AimModifiers.AimWallDetectionTraceOnBeamPosition) : 0.0;
		const FVector End = KML::VLerp(DefaultEnd, BeamForSwingingPosition, AimAlpha);

		KSL::SphereTraceSingle(
			this, Start, End, 20.0f, WallTraceChannel, false,
			ActorsToIgnore, 
			DrawAdditiveTracesIndex > 0 ? DrawDebugType : EDrawDebugTrace::None,
			FirstWallHit, bIgnoreSelf,
			FLinearColor::Red, FLinearColor::Green, WallDrawTime);
	}

	// Both failure return nodes use NoEvenWallHit = true and CheckedLedgesNumber = 1.
	if (!FirstWallHit.bBlockingHit
		|| !(FMath::Abs(FVector::DotProduct(FirstWallHit.Normal, FVector::UpVector)) < 0.75))
	{
		return false;
	}
	NoEvenWallHit = false;

	const double DistanceMapRangeMax = KML::Vector_Distance(
		FirstWallHit.Location
		+ UHelpfulFunctionsBPLibrary::NormalToVector(FirstWallHit.Normal) * 50.0,
		InitialCheckPosition + FVector(0.0, 0.0, DefCapsuleSizeC.Y * 0.8));

	// Equivalent to MinOfFloatArray + Get + the successful FunctionResult node.
	const auto ReturnBestLedge = [&]() -> bool
		{
			int32 BestIndex = 0;
			for (int32 Index = 1; Index < ValidLedgesWeight.Num(); ++Index)
			{
				if (ValidLedgesWeight[Index] < ValidLedgesWeight[BestIndex])
				{
					BestIndex = Index;
				}
			}

			const FCMC_LedgeC& Best = ValidLedgeResult[BestIndex];
			LeftPoint.ValidPoint = true;
			LeftPoint.Location = Best.LeftPoint.GetLocation();
			LeftPoint.Normal = KML::GetForwardVector(Best.LeftPoint.Rotator());
			LeftPoint.Component = Best.Component;
			RightPoint.ValidPoint = true;
			RightPoint.Location = Best.RightPoint.GetLocation();
			RightPoint.Normal = KML::GetForwardVector(Best.RightPoint.Rotator());
			RightPoint.Component = Best.Component;
			OriginPoint.ValidPoint = true;
			OriginPoint.Location = Best.Origin.GetLocation();
			OriginPoint.Normal = KML::GetForwardVector(Best.Origin.Rotator());
			OriginPoint.Component = Best.Component;
			NoEvenWallHit = false;
			CheckedLedgesNumber = I1 + 1;

			// The graph still populates the points if this validity check fails.
			return IsValid(Best.Component);
		};

	// ForLoop is inclusive. Its LastIndex input is reevaluated at each loop test.
	for (int32 Index = 0; Index <= (MaxLedgeCheckExecute - 1) + ReadModifiers().AdditiveLedgeCheckInterations; ++Index)
	{
		I1 = Index;
		const FVector SearchDirection = KML::Vector_SlerpNormals(
			UHelpfulFunctionsBPLibrary::NormalToVector(FirstWallHit.Normal),
			InitialCheckDirection, 0.2);
		const FVector TraceOrigin = FirstWallHit.Location
			+ FVector(0.0, 0.0,
				static_cast<double>(LedgeCheckOffsetZ)
				+ (IsValid(FindedBeamActor) ? -5.0 : 0.0)
				+ static_cast<double>(I1) * 25.0)
			+ SearchDirection * -15.0;

		bool bValid = false;
		bool bFirstNotValid = false;
		FCMC_SingleClimbPointC CandidateLeft{};
		FCMC_SingleClimbPointC CandidateRight{};
		FCMC_SingleClimbPointC CandidateOrigin{};
		TryCreateLedgeStructureC_Implementation(bValid, CandidateLeft, CandidateRight, CandidateOrigin, bFirstNotValid, TraceOrigin, SearchDirection, 0.0f, 50.0f, true); //MOST INPORTANT!!!

		// Sequence 1, Then 0: validate, score and possibly store the candidate.
		// A local lambda lets candidate rejection return to Then 1 below.
		const auto ProcessCandidate = [&]()
			{
				if (!bValid)
				{
					return; // Do not reset CurrentWeightCalculation here.
				}
				CurrentWeightCalculation = 0.0;
				CurrentLedgeStruct.LeftPoint = FTransform(
					KML::MakeRotFromX(CandidateLeft.Normal), CandidateLeft.Location, DefScale);
				CurrentLedgeStruct.RightPoint = FTransform(
					KML::MakeRotFromX(CandidateRight.Normal), CandidateRight.Location, DefScale);
				CurrentLedgeStruct.Origin = FTransform(
					KML::MakeRotFromX(CandidateOrigin.Normal), CandidateOrigin.Location, DefScale);
				CurrentLedgeStruct.Component = CandidateOrigin.Component;

				const FVector LedgeLocation = CurrentLedgeStruct.Origin.GetLocation();
				const FVector LedgeForward = KML::GetForwardVector(CurrentLedgeStruct.Origin.Rotator());
				const FVector DistanceOrigin = InitialCheckPosition
					+ FVector(0.0, 0.0, DefCapsuleSizeC.Y * 0.8);
				const double HeightDifference = LedgeLocation.Z - InitialCheckPosition.Z;
				const double Distance2D = KML::Vector_Distance2D(LedgeLocation, DistanceOrigin);

				if (!(HeightDifference < 200.0 && HeightDifference > -100.0
					&& Distance2D < 400.0 * static_cast<double>(MaxSimTime)
					&& FVector::DotProduct(LedgeForward, InitialCheckDirection) > 0.0))
				{
					return;
				}

				if (!IsValid(FindedBeamActor))
				{
					const FVector OverheadPosition = LedgeLocation + LedgeForward * 10.0
						+ FVector(0.0, 0.0, DefCapsuleSizeC.Y + 1.0);
					FHitResult OverheadHit;
					// Both endpoints are identical in the exported graph.
					const bool bOverheadHit = KSL::CapsuleTraceSingle(
						this, OverheadPosition, OverheadPosition,
						static_cast<float>(DefCapsuleSizeC.X),
						static_cast<float>(DefCapsuleSizeC.Y * 0.95),
						CapsuleTraceChannel, false, ActorsToIgnore, 
						DrawAdditiveTracesIndex > 1 ? DrawDebugType : EDrawDebugTrace::None,
						OverheadHit, bIgnoreSelf,
						FLinearColor(0.033338f, 0.0f, 0.354167f, 1.0f),
						FLinearColor(0.331854f, 0.369372f, 1.0f, 1.0f), OverheadDrawTime);

					if (!bOverheadHit)
					{
						return; // Preserve the actual Branch wiring, not the BP comment.
					}
				}

				CurrentWeightCalculation = SmallRangeClamp(KML::Vector_Distance(LedgeLocation, DistanceOrigin), 50.0, DistanceMapRangeMax, 0.0, 1.0) + SmallRangeClamp(Distance2D, 50.0, DistanceMapRangeMax, 0.0, 1.0) * 0.5;
				CurrentWeightCalculation += KML::MapRangeUnclamped(FMath::Abs(CurrentLedgeStruct.LeftPoint.GetLocation().Z - CurrentLedgeStruct.RightPoint.GetLocation().Z), 2.0, 20.0, 0.0, 1.0);

				FCALS_ComponentAndTransform LedgeCenter{};
				LedgeCenter.Transform = CurrentLedgeStruct.Origin;
				LedgeCenter.Component = CurrentLedgeStruct.Component;
				const FVector CapsulePosition = ConvertLedgeToCapPositionC(LedgeCenter)
					.Transform.GetLocation();
				FHitResult CapsuleHit;
				const bool bCapsuleHit = KSL::CapsuleTraceSingle(
					this, CapsulePosition, CapsulePosition + FVector(0.0, 0.0, -1.0),
					static_cast<float>(DefCapsuleSizeC.X), static_cast<float>(DefCapsuleSizeC.Y),
					CapsuleTraceChannel, false, CapsuleActorsToIgnore, DrawAdditiveTracesIndex > 2 ? DrawDebugType : EDrawDebugTrace::None,
					CapsuleHit, bIgnoreSelf,
					FLinearColor(0.098958f, 0.0f, 0.0f, 1.0f),
					FLinearColor(1.0f, 0.013920f, 0.0f, 1.0f), CapsuleDrawTime);

				if (bCapsuleHit && KML::Vector_Distance2D(CapsuleHit.ImpactPoint, CapsuleHit.TraceStart)
					< DefCapsuleSizeC.X * 0.25)
				{
					CurrentWeightCalculation += 10.0;
				}

				if (IsValid(FindedBeamActor))
				{
					const FVector RelativeLedgeLocation = KML::MakeRelativeTransform(
						FTransform(FRotator::ZeroRotator, LedgeLocation, DefScale),
						CharacterC->GetActorTransform()).GetLocation();
					if (!(RelativeLedgeLocation.X > -20.0
						&& FMath::Abs(RelativeLedgeLocation.Y) < 40.0))
					{
						return;
					}
				}

				ValidLedgesWeight.Add(CurrentWeightCalculation);
				ValidLedgeResult.Add(CurrentLedgeStruct);
			};
		ProcessCandidate();

		// Sequence 1, Then 1: always runs, including after candidate rejection.
		AllTracesFromLedgeGen += SingleLedgeFindTotalTracesResult;
		const int32 QueryThreshold = MaxTotalTracesQueries
			+ ReadModifiers().AddMaxTotalTracesCallPerExecution;
		if ((AllTracesFromLedgeGen > QueryThreshold || CurrentWeightCalculation < 0.5)
			&& ValidLedgesWeight.Num() > 0)
		{
			return ReturnBestLedge();
		}
		// With no accepted candidates, the Blueprint continues even over budget.
	}

	// ForLoop Completed -> LoopComplete = true -> final candidate selection.
	if (ValidLedgesWeight.Num() > 0)
	{
		return ReturnBestLedge();
	}

	// Outputs were initialized above; NoEvenWallHit is already false.
	return false;
}


bool UCpp_DynamicClimbingComponent::PrepareAndStartLedgeClimbing_Implementation(bool& UseFreeHang, FCMC_SingleClimbPointC InLeftPoint, FCMC_SingleClimbPointC InRightPoint, FCMC_SingleClimbPointC InOriginPoint)
{
	if (!InOriginPoint.Component) return false;

	//Step 1) Convert function parameters to ledge structure and save current character transform in local space
	LedgePointsLS_C.LeftPoint = ConvertLedgeStructToLS(InLeftPoint).Transform;
	LedgePointsLS_C.RightPoint = ConvertLedgeStructToLS(InRightPoint).Transform;
	LedgePointsLS_C.Origin = ConvertLedgeStructToLS(InOriginPoint).Transform;
	LedgePointsLS_C.Component = ConvertLedgeStructToLS(InOriginPoint).Component;

	FCALS_ComponentAndTransform CurrentCapsulePosition; CurrentCapsulePosition.Transform = CharacterC->GetActorTransform(); CurrentCapsulePosition.Component = LedgePointsLS_C.Component;
	SavedCapsuleTransformLS = UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(CurrentCapsulePosition);

	FCALS_ComponentAndTransform LedgeCenterStructWS; LedgeCenterStructWS.Transform = FTransform(KML::MakeRotFromX(InOriginPoint.Normal), InOriginPoint.Location, FVector(1, 1, 1));
	LedgeCenterStructWS.Component = InOriginPoint.Component;
	const FCALS_ComponentAndTransform ConvertedToCapsuleWS = ConvertLedgeToCapPositionC(LedgeCenterStructWS);

	bool FootIKLeft = CheckFootIkValidC(ConvertedToCapsuleWS.Transform, false, 6.0);
	bool FootIKRight = CheckFootIkValidC(ConvertedToCapsuleWS.Transform, true, -6.0);
	UseFreeHang = !(FootIKLeft && FootIKRight);

	//Step 3) From function parametr create final capsule position. Converted to local space
	const FTransform DesiredCapsuleTransform = FTransform(
		KML::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(InOriginPoint.Normal) * -1.0f),															//Rotation
		InOriginPoint.Location + (UHelpfulFunctionsBPLibrary::NormalToVector(InOriginPoint.Normal) * DefCapsuleSizeC.X) - FVector(0, 0, CapsuleUpOffsetC),		//Location
		FVector(1, 1, 1));																																		//Scale
	FCALS_ComponentAndTransform DesiredCapsuleTransformStruct; DesiredCapsuleTransformStruct.Transform = DesiredCapsuleTransform; DesiredCapsuleTransformStruct.Component = InOriginPoint.Component;
	if (!DesiredCapsuleTransformStruct.Component) return false;

	CapsuleTargetTransformLS = UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(DesiredCapsuleTransformStruct);

	return true;
	//This function require contintinuation but using Blueprints
}


void UCpp_DynamicClimbingComponent::GetModifyParametersForWallDetection(int& AdditiveLedgeCheckInterations, float& OverrideSimTime, bool& bCanSearchForBeamForSwinging, float& AimWallDetectionTraceOnBeamPosition, 
	int& OverrideWallDetectionMode, int& AddMaxTotalTracesCallPerExecution) const
{
	if (CurrentModifyVolume)
	{
		AdditiveLedgeCheckInterations = CurrentModifyVolume->AdditiveLedgeCheckInterations;
		OverrideSimTime = CurrentModifyVolume->OverrideSimTime;
		bCanSearchForBeamForSwinging = CurrentModifyVolume->bCanSearchForBeamForSwinging;
		AimWallDetectionTraceOnBeamPosition = CurrentModifyVolume->AimWallDetectionTraceOnBeamPosition;
		OverrideWallDetectionMode = CurrentModifyVolume->OverrideWallDetectionMode;
		AddMaxTotalTracesCallPerExecution = CurrentModifyVolume->AddMaxTotalTracesCallPerExecution;
		return;
	}
	AdditiveLedgeCheckInterations = 0;
	OverrideSimTime = -1.0f;
	bCanSearchForBeamForSwinging = false;
	AimWallDetectionTraceOnBeamPosition = 0.0f;
	OverrideWallDetectionMode = -1;
	AddMaxTotalTracesCallPerExecution = 0;
	return;
}


#undef SAFEDELTATIME
#undef KSL
#undef KML
#undef CLASSTOIGNORESAFE