// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "FootContactAnimModifier.generated.h"

//FootsContact
UCLASS(meta = (DisplayName = "FootsContact"))
class COMBATCOREEDITOR_API UFootContactAnimModifier : public UAnimationModifier
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Bones")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Bones")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Bones")
    FName RootBoneName = TEXT("root");

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Curves")
    FName LeftContactCurveName = TEXT("Contact_L");

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Curves")
    FName RightContactCurveName = TEXT("Contact_R");

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Curves")
    FLinearColor LeftContactCurveColor = FLinearColor(0.0f, 0.35f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Curves")
    FLinearColor RightContactCurveColor = FLinearColor(1.0f, 0.15f, 0.1f, 1.0f);

    // Sampling the output curve. In your example, the contact is every 1/60 s.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Sampling", meta = (ClampMin = "1.0"))
    float SampleRate = 60.0f;

    // How many cm above the lowest detected foot level do we still consider contact?.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Detection", meta = (ClampMin = "0.0"))
    float HeightTolerance = 4.0f;

    // Horizontal speed of the foot relative to the world/root motion. For runes, typically 80-180 cm/s.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Detection", meta = (ClampMin = "0.0"))
    float MaxPlanarSpeed = 900.0f;

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Detection", meta = (ClampMin = "0.0"))
    FVector2D MaxFootBoneSpeed = FVector2D(20,200);

    FVector2D UseFootSpeedTolleranceRange = FVector2D(20, 300);

    // Additional criterion: vertical speed must be low.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Detection", meta = (ClampMin = "0.0"))
    float MaxVerticalSpeed = 110.0f;

    // Minimal contact time. Shorter "pins" will be removed.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Filtering", meta = (ClampMin = "0.0"))
    float MinContactDuration = 0.06f;

    // Minimum break time. Shorter gaps in contact will be closed..
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Filtering", meta = (ClampMin = "0.0"))
    float MinGapDuration = 0.04f;

    // Miekkosc przejscia 0<->1.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Filtering", meta = (ClampMin = "0.0"))
    float BlendTime = 0.06f;

    // Disabled Parameter
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Curves")
    bool bUseAutoTangents = false;

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Phase")
    float LeftPhaseOffset = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Foot Contact|Phase")
    float RightPhaseOffset = 0.0f;

    // For looped animations, usually true.
    // For start/stop animations, it is often false.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Phase")
    bool bWrapPhaseOffset = true;

    // true:
    // Smoothing is performed towards the center of the detected contact.
    // Values ​​0..1 and 1..0 are within the original contact segment.
    //
    // false:
    // Smoothing is performed towards the outside of the detected contact.
    // The original contact segment remains a flat plateau of 1,
    // and soft transitions are added before and after contact.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Filtering")
    bool bBlendInward = true;

    // A positive value narrows the detected contact. 
    // // A negative value widens the detected contact. 
    // // Unit: seconds.
    UPROPERTY(EditAnywhere, Category = "Foot Contact|Filtering")
    float ContactPaddingTime = 0.0f;

protected:
    virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
    virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

private:
    struct FSample
    {
        float Time = 0.0f;
        FVector FootCS = FVector::ZeroVector;
        FVector RootCS = FVector::ZeroVector;
    };

    struct FContactSegment
    {
        int32 StartIndex = INDEX_NONE;
        int32 EndIndex = INDEX_NONE; // inclusive
    };

    bool SampleBoneComponentSpace(
        const UAnimSequence* AnimationSequence,
        FName BoneName,
        float Time,
        FTransform& OutComponentSpaceTransform) const;

    void BuildContactCurve(
        UAnimSequence* AnimationSequence,
        FName FootBoneName,
        FName CurveName,
        float PhaseOffset,
        FLinearColor CurveColor) const;

    void RemoveCurveIfExists(UAnimSequence* AnimationSequence, FName CurveName) const;

    static void RemoveShortContacts(TArray<uint8>& InOutMask, int32 MinFrames);
    static void FillShortGaps(TArray<uint8>& InOutMask, int32 MinFrames);
    static float SmoothContactValue(const TArray<uint8>& Mask, int32 Index, int32 BlendFrames);

    static void BuildSmoothedContactValues(
        const TArray<uint8>& Mask,
        int32 BlendFrames,
        bool bStartBlendAfterTransition,
        TArray<float>& OutValues);

    static float SmoothStep01(float Alpha);

    static float SampleFloatArrayByTime(
        const TArray<float>& Values,
        float Time,
        float Duration,
        float SampleRate,
        bool bWrap);

    static void ApplyContactPadding(TArray<uint8>& InOutMask, int32 PaddingFrames);

    static void BuildSmoothedContactValuesFromSegments(
        const TArray<uint8>& Mask,
        int32 BlendFrames,
        bool bBlendInward,
        TArray<float>& OutValues);

    bool ExtractRootMotion(const UAnimSequence* AnimationSequence, FName BoneName, float Time, FTransform& OutComponentSpaceTransform) const;

};
