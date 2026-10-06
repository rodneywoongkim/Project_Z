// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AGLS_AdvancedLedgesFinder.generated.h"

class ACharacter;
class AActor;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EAGLSLedgeFailure : uint8
{
    None,
    InvalidInput,
    MissingCollisionChannel,
    NoCandidate,
    NoUsableSegment,
    CapsuleRejected,
    QueryBudgetExceeded
};

/** Distances in world centimetres; angles in degrees. Settings are checked at runtime too. */
USTRUCT(BlueprintType)
struct HELPFULFUNCTIONS_API FAGLSLedgeSearchSettings
{
    GENERATED_BODY()

    /** Resolved by name at runtime; there is deliberately no silent Visibility fallback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName TraceChannelName = FName(TEXT("For_Climbing"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bTraceComplex = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<AActor*> ActorsToIgnore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "1"))
    float SearchDistance = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "0"))
    float SearchAbove = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "0"))
    float SearchBelow = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "0"))
    float SearchHalfWidth = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "0", ClampMax = "75"))
    float SearchFanHalfAngle = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "1"))
    float DiscoveryHeightStep = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search", meta = (ClampMin = "1", ClampMax = "64"))
    int32 MaxCandidates = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "1"))
    float MinLength = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "1"))
    float MaxLength = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "1"))
    float SampleStep = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "0.1"))
    float BoundaryTolerance = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "0", ClampMax = "75"))
    float MaxLedgeSlope = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "0", ClampMax = "85"))
    float MaxTopSlope = 55.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "0", ClampMax = "89"))
    float MaxNormalChange = 30.f;

    /** Maximum distance of sampled contacts from the returned straight segment. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge", meta = (ClampMin = "0"))
    float MaxChordDeviation = 3.f;

    /** Local profile search, also used to follow bevels and rounded bars. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "1"))
    float ProfileHeightRange = 14.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "0.5"))
    float ProfileStep = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "1"))
    float ProfileReach = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "0.5"))
    float MaxTransitionDepth = 14.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "0.1"))
    float MinSupportDepth = 3.f;

    /** Radius of the local finger-space probe, not of the character capsule. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grip", meta = (ClampMin = "0.1"))
    float GripProbeRadius = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grip", meta = (ClampMin = "0"))
    float GripGap = 0.5f;

    /** Negative Z relative to contact; independently checks interfering lower beams. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grip", meta = (ClampMin = "0"))
    float GripClearanceBelow = 5.f;

    /** Allowed capsule intrusion along its outward axis, as a fraction of test radius. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "0", ClampMax = "1"))
    float MaxIntrusionDepthRatio = 0.25f;

    /** Fraction of the sampled capsule silhouette affected, NOT penetrated volume. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "0", ClampMax = "1"))
    float MaxIntrusionAreaFraction = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "0"))
    float ContactTolerance = 0.5f;

    /** Front-side start distance for profile rays. Obstructed/ambiguous starts reject. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "1"))
    float CapsuleProbeStandOff = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "5", ClampMax = "65"))
    int32 CapsuleColumns = 17;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule", meta = (ClampMin = "5", ClampMax = "65"))
    int32 CapsuleRows = 25;

    /** Signed coefficients: positive minimizes the metric, negative maximizes it,
     * zero removes it from scoring. Hard validity/clearance limits still apply.
     * Metrics retain their units; these are not normalized percentages.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float DistanceToSearchOriginWeightScale = 1.f;

    /** Distance in 3D from the horizontal SearchDirection axis through SearchOrigin. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float SideDistanceWeightScale = 0.25f;

    /** Negative by default: prefer longer usable ledges. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float LedgeLengthWeightScale = -0.15f;

    /** Multiplies test capsule radius * estimated intrusion area fraction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float IntrusionAreaWeightScale = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    float IntrusionDepthWeightScale = 1.f;

    /** Bounds endpoint combinations and their capsule evaluations per candidate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "1", ClampMax = "128"))
    int32 MaxSegmentVariants = 24;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget", meta = (ClampMin = "100", ClampMax = "200000"))
    int32 MaxSceneQueries = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = false;

    /** Very verbose: draws every trace and every capsule silhouette sample. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawAllQueries = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0"))
    float DebugDuration = 3.f;
};

USTRUCT(BlueprintType)
struct HELPFULFUNCTIONS_API FAGLSLedgeResult
{
    GENERATED_BODY()

    /** X points INTO the surface; Y is player's right; Z is local up. Scale is one. */
    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    FTransform LeftTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    FTransform RightTransform = FTransform::Identity;

    /** On the sampled contact curve, at half arc length, not inside its chord. */
    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    FTransform CenterTransform = FTransform::Identity;

    /** World-upright test capsule. This is a diagnostic target, not a movement command. */
    UPROPERTY(BlueprintReadOnly, Category = "Capsule")
    FTransform CapsuleTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadOnly, Category = "Capsule")
    float CapsuleRadius = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Capsule")
    float CapsuleHalfHeight = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Capsule")
    float EstimatedMaxIntrusionDepth = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Capsule")
    float EstimatedIntrusionAreaFraction = 0.f;

    /** Euclidean distance between returned endpoints; limits apply with BoundaryTolerance to this length. */
    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    float Length = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    float ArcLength = 0.f;

    /** Actual sampled contact curve for debugging and future curved traversal. */
    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    TArray<FVector> ContactSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    UPrimitiveComponent* SurfaceComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    bool bSpansMultipleComponents = false;

    /** Local profile suggests a bevel/rounding; not a mesh classification guarantee. */
    UPROPERTY(BlueprintReadOnly, Category = "Ledge")
    bool bLikelyRoundedOrBeveled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    EAGLSLedgeFailure Failure = EAGLSLedgeFailure::None;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    int32 SceneQueryCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    int32 TestedCandidates = 0;

    /** Finite candidate/variant/sample limits restricted search coverage. */
    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    bool bSearchTruncated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    bool bQueryBudgetExhausted = false;
};

UCLASS()
class HELPFULFUNCTIONS_API UAGLS_AdvancedLedgesFinder : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Synchronous game-thread query. Does not move Character or change its collision.
     * SearchDirection points approximately INTO the wall. CapsuleScale: X=radius,
     * Y=total half-height. Offsets: center - horizontal(X)*factor*radius - localZ*down.
     * World Z gravity / upright Character capsules are supported by this version.
     */
    UFUNCTION(BlueprintCallable, Category = "AGLS|Climbing",
        meta = (DisplayName = "Try Find Ledge For Climbing", AutoCreateRefTerm = "Settings",
            CPP_Default_CapsuleScale = "(X=1.000000,Y=1.000000)",
            CPP_Default_CapsuleHorizontalOffsetFactor = "1.0",
            CPP_Default_CapsuleVerticalOffset = "60.0"))
    static bool TryFindLedgeForClimbing(
        ACharacter* Character,
        FVector SearchOrigin,
        FVector SearchDirection,
        FVector2D CapsuleScale,
        float CapsuleHorizontalOffsetFactor,
        float CapsuleVerticalOffset,
        const FAGLSLedgeSearchSettings& Settings,
        FAGLSLedgeResult& OutResult);
};
