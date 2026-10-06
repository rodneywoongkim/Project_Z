

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "IWALS_BaseAttributeSet.h"
#include "IWALS_GameplayAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include <GameplayEffectTypes.h>
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "MovementParamsControlComponent.h"
#include "GAS_MainCharacterCpp.generated.h"

/*
Structure intended for controlling parameters related to CharacterMovement.
Values such as Speed are stored as vector variables, where:
- X represents forward speed,
- Y represents left/right movement speed,
- Z represents backward speed.

MovementCurve is associated with:
- WalkingAcceleration (X curve),
- Deceleration (Y curve),
- GroundFriction (Z curve).

RotationRateCurve is intended for controlling the interpolation speed of capsule rotation toward DesiredRotation.

NOTE: This structure is marked as deprecated in AGLS v1.9.
Currently, these values for both PlayerCharacter and HumanAI are controlled by MovementParamsControlComponent, 
which uses different structure types. */
USTRUCT(BlueprintType)
struct FMovementSettingsStrafe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector WalkSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector RunSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector SprintSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	UCurveVector* MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	UCurveFloat* RotationRateCurve = nullptr;
};

/*Base UCharacter class intended for implementing player logic. Declared and prepared for the AdventureGameLocomotionSystem (AGLS v1.0+) project. 
By default, it contains declarations for many essential variables and functions. GAS_MainCharacter also creates the required structure for proper 
operation and integration of the Gameplay Ability System. This includes, among other things, the construction of the UAbilitySystemComponent and 
UIWALS_GameplayAbilitySet. This class also contains components such as UMovementParamsControlComponent, whose purpose is to manage values related 
to CharacterMovementComponent.*/
UCLASS()
class IWALS_ABILITYSYSTEM_API AGAS_MainCharacterCpp : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_MainCharacterCpp();

	//Define Base Variables For ALS Character
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	FVector2D DefCapsuleSizeC = FVector2D(30, 90);

	/*IsMoving refers to whether the character is currently moving. By default, this value is calculated as Speed > 1.0.*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool IsMovingC = false;

	/*Usually indicates whether the player currently has any active movement input. In the case of a keyboard, this would be for example the W key.
	if (MovementInputAmountC > 0.0) { HasMovementInputC = true; } else { HasMovementInputC = false; }*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool HasMovementInputC = false;

	/*IsStartedMovementOnTargetC indicates whether the character is currently in automatic movement mode toward a target location. This functionality 
	is sometimes used by interactions. Such a mode triggers MovementInput in the direction and for the duration required for the character to reach 
	the destination. During such a sequence, player movement inputs should usually be disabled.*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool IsStartedMovementOnTargetC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True", DisplayName = "Start Interaction With Dynamic Prop C"))
	bool InteractionWithPropC = false;

	/*⚠︎ DEPRECATED FUNCTIONALITY - AGLS v1.9*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool IsLayBackC = false;

	/*IsSwimmingC indicates whether the player is currently in swimming movement mode. Usually this is equivalent to CharacterMovementComponent->IsSwimming().*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool IsSwimmingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
	bool RagdollOnGroundC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
	bool RagdollFaceUpC = false;

	/*AccelerationC is the physical representation of the capsule’s acceleration. It is calculated based on the Velocity value. 
	The equation representing this value can be written as: (Velocity[n] - Velocity[n - 1]) / dt*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	FVector AccelerationC = FVector(0, 0, 0);

	/*RelativeAccelerationC is AccelerationC with the unrotated direction.*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	FVector RelativeAcceleractionC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	FRotator LastVelocityRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	FRotator LastMovementInputRotationC = FRotator(0, 0, 0);

	/*stores the previous velocity value of the capsule.*/
	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
	FVector PreviousVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
	FVector LastRagdollVelocityC = FVector(0, 0, 0);

	/*SpeedC is the speed calculated from Velocity while ignoring the Z axis. SpeedC = GetVelocity().SizeXY()*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	float SpeedC = 0.0;

	/*MovementInputAmountC = AccelerationXY.Length() / this->GetCharacterMovement()->GetMaxAcceleration();*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	float MovementInputAmountC = 0.0;

	/*MovementSpeedDifferenceC = FVector(GetVelocity().X, GetVelocity().Y, 0.0).Length() - FVector(PreviousVelocityC.X, PreviousVelocityC.Y, 0.0).Length();*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	float MovementSpeedDifferenceC = 0.0;

	/*AimYawRateC = abs((GetControlRotation().Yaw - PreviousAimYawC) / SafeDelta);*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	float AimYawRateC = 0.0;

	/*PreviousAimYawC = GetControlRotation().Yaw;*/
	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
	float PreviousAimYawC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
	FGameplayAbilitySpecHandle AbilityHandle;

	/*⚠︎ DEPRECATED VARIABLE - AGLS v1.9*/
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	UCurveFloat* StrafeSpeedMapCurveC = nullptr;

	/*⚠︎ DEPRECATED VARIABLE - AGLS v1.9*/
	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
	FMovementSettingsStrafe CurrentMovementSettingsC;

	/*FloorVelocityC represents the current velocity of the surface the Character is moving on. It is calculated based on the component’s 
	current position relative to the previous frame. The component is obtained using:
	this->GetCharacterMovement()->CurrentFloor.HitResult.GetComponent()*/
	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
	FVector FloorVelocityC = FVector(0, 0, 0);

	/*PrevFloorVelocityC stores a copy of FloorVelocityC, but only when non-inertial floor correction is active.*/
	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
	FVector PrevFloorVelocityC = FVector(0, 0, 0);

	/*PrevFloorComponent is required to correctly calculate FloorVelocityC. It helps avoid incorrect velocity values when the player moves 
	from one component to another.*/
	UPrimitiveComponent* PrevFloorComponent = nullptr;

	void CalculateFloorVelocity(FVector& VelocityToUpdate, FVector& PositionToSave, float inDT);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
	bool OverlayStateLeavingStarted = false;

	bool CanUpdateFromDesiredOverlay = false;

	/* Experimental function. Improves the behavior of the capsule in a non-inertial reference frame (the floor moves relative to the world space) */
	UPROPERTY(BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "True"))
	bool CorrectNonInertialFloor = true;

	FVector PrevFloorLocation = FVector(0, 0, 0);
	bool AddedFloorForce = false;

	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	///UFUNCTION(BlueprintPure, Category = "Movement System", meta = (DisplayName = "Get Target Speed With Strafe", Keywords = "Movement"))
	virtual float GetTargetSpeedWithStrafeC(FVector SpeedVector);

	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintPure, Category = "Movement System", meta = (DisplayName = "Get Mapped Speed", Keywords = "Movement"))
	virtual float GetMappedSpeedC(float SpeedScale = 1.0);

	/*A simple function that creates capsule rotation interpolation. The default code for this function is below:
	FRotator NewRotation = KML::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->DeltaTimeSeconds, ActorInterpSpeed);
	const FQuat TargetQuat = KM::Conv_RotatorToQuaternion(NewRotation);
	SetActorRotation(TargetQuat, ETeleportType::None);*/
	UFUNCTION(BlueprintCallable, Category = "Rotation System", meta = (DisplayName = "Smooth Character Rotation", Keywords = "Rotation"))
	virtual void SmoothCharacterRotationC(FRotator TargetRotation = FRotator(0, 0, 0), float ActorInterpSpeed = 10.0);

	/*⚠︎ DEPRECATED FUNCTION - AGLS v1.9 ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤ ◢◤*/
	//UFUNCTION(BlueprintPure, Category = "Rotation System", meta = (DisplayName = "Calculate Grounded Rotation Speed", Keywords = "Rotation"))
	virtual float CalculateGroundedRotationSpeedC(float Scale = 1.0, FVector2D YawScaleRange = FVector2D(1.0, 3.0));

	/*Return Forward and Right vectors from ControlRotation. Default code:
	ForwardVector = KML::GetForwardVector(FRotator(0.0, GetControlRotation().Yaw, 0.0));
	RightVector = KML::GetRightVector(FRotator(0.0, GetControlRotation().Yaw, 0.0));*/
	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Get Control Vectors", Keywords = "Others"))
	virtual void GetControlVectorsC(FVector& ForwardVector, FVector& RightVector);

	//Simple converting ActorLocation to Capsule start position
	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Get Capsule Base Location", Keywords = "Others"))
	virtual FVector GetCapsuleBaseLocationC(float ZOffset);

	//Simple converting Capsule start position to actor location
	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Floor To Capsule Location", Keywords = "Others"))
	virtual FVector FloorToCapsuleLocationC(FVector BaseLocation, float ZOffset, bool ByDefSize);




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FRotator GetViewRotation() const override;

	virtual FRotator GetBaseAimRotation() const override;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Pawn|Rotation", meta = (ForceAsFunction, Keywords = "Pawn,Character,Rotation"))
	bool GetCustomViewRotation(FRotator& ReturnRotation) const;
	virtual bool GetCustomViewRotation_Implementation(FRotator& ReturnRotation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Pawn|Rotation", meta = (ForceAsFunction, Keywords = "Pawn,Character,Rotation"))
	bool GetCustomBaseAimRotation(FRotator& ReturnRotation) const;
	virtual bool GetCustomBaseAimRotation_Implementation(FRotator& ReturnRotation) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", Category = "Components"))
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", Category = "Components"))
	class UMovementParamsControlComponent* MovementControlComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	UIWALS_GameplayAbilitySet* AbilitiesData;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY()
	class UIWALS_BaseAttributeSet* Attributes;

	// Gameplay Tag Functions
	// -----------------------------------------------
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Convert Literal Name To Tag", Keywords = "Gameplay Tag"))
	virtual FGameplayTag ConvertLiteralNameToTag(FName TagName);

	/*𝐂𝐔𝐒𝐓𝐎𝐌 𝕋𝔸𝔾𝕊 𝐌𝐀𝐂𝐑𝐎*/
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Get Sub Tag", Keywords = "Gameplay,Tag"))
	virtual FString GetSubTag(const FGameplayTag& Tag, int32 DesiredDepth);

	/*𝐂𝐔𝐒𝐓𝐎𝐌 𝕋𝔸𝔾𝕊 𝐌𝐀𝐂𝐑𝐎*/
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Is Tag Leaf", Keywords = "Gameplay,Tag"))
	virtual bool IsTagLeaf(const FGameplayTag& Tag);

	/*𝐂𝐔𝐒𝐓𝐎𝐌 𝕋𝔸𝔾𝕊 𝐌𝐀𝐂𝐑𝐎*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Switch On Owned Tags", Keywords = "Gameplay,Tag"))
	virtual bool SwitchOnOwnedTags(const FGameplayTag& NewState);

	/*𝐂𝐔𝐒𝐓𝐎𝐌 𝕋𝔸𝔾𝕊 𝐌𝐀𝐂𝐑𝐎*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Switch On Owned Tags With Ignore", Keywords = "Gameplay,Tag"))
	virtual bool SwitchOnOwnedTagsWithIgnore(const FGameplayTag& NewState, const FGameplayTagContainer& DoNotEdit);

	/*𝐂𝐔𝐒𝐓𝐎𝐌 𝕋𝔸𝔾𝕊 𝐌𝐀𝐂𝐑𝐎*/
	UFUNCTION(BlueprintPure, Category = "Gameplay Tags|Tag Container|Custom", meta = (DisplayName = "Filter Tags By Root Group", Keywords = "Gameplay,Tag"))
	virtual void FilterTagsByRootGroup(const FGameplayTagContainer& Input, FGameplayTag RootTag, bool StopWhenFirstValid, FGameplayTagContainer& ReturnContainer);



	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void InitializeAttributes();
	virtual void GiveAbilities();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/*WARNING - This function has been disabled due to outdated code. It is no longer available.*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Input", meta = (DisplayName = "Try Create Inputs Binds For GAS", Keywords = "Inputs Player"))
	virtual void TryCreateInputsGAS();


	// For Overlay System
	UFUNCTION(BlueprintImplementableEvent)
	void DoWhenOverlayLeaving(float DeltaSecond);

	UFUNCTION(BlueprintImplementableEvent)
	void OverlayLeavingFinshed();


	UFUNCTION(BlueprintImplementableEvent, Category = "Movement System", meta = (Keywords = "Tick"))
	void TickBeforeMovementComponent(float DeltaSecond);

	/*A function designed to update the values ​​of variables such as:
	- AccelerationC (Vector)
	- MovementSpeedDifferenceC (float)
	- SpeedC (float)
	- IsMovingC (bool)
	- MovementInputAmountC (float)
	- HasMovementInputC (bool)
	- AimYawRateC (float)
	- IsSwimmingC (bool)
	- LastMovementInputRotationC (Rotator)
	- LastVelocityRotationC (Rotator)
	- PreviousVelocityC (Vector)
	- PreviousAimYawC (float)*/
	UFUNCTION(BlueprintCallable, Category = "Movement System", meta = (DisplayName = "Update Essential Movement Values", Keywords = "Movement,CMC"))
	void UpdateEssentialMovementValues(float InDelta);


};
