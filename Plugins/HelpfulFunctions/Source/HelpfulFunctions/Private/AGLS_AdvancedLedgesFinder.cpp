#include "AGLS_AdvancedLedgesFinder.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Math/RotationMatrix.h"

namespace AGLSLedgeFinder
{
    static const FVector Up = FVector::UpVector;

    struct FContact
    {
        FVector Point = FVector::ZeroVector;
        FVector Outward = FVector::ForwardVector;
        FVector TopNormal = FVector::UpVector;
        UPrimitiveComponent* Component = nullptr;
        bool bRounded = false;
        float ApproachDepth = 0.f;
    };

    struct FSeed
    {
        FVector Point;
        FVector Outward;
        double Score = 0.;
    };

    struct FContext
    {
        UWorld* World;
        const FAGLSLedgeSearchSettings& S;
        ECollisionChannel Channel;
        FCollisionQueryParams Params;
        int32 Queries = 0;
        bool bExhausted = false;
        bool bTruncated = false;
        FVector Origin;
        FVector Forward;

        FContext(UWorld* InWorld, ACharacter* Character,
            const FAGLSLedgeSearchSettings& Settings, ECollisionChannel InChannel,
            const FVector& InOrigin, const FVector& InForward)
            : World(InWorld), S(Settings), Channel(InChannel),
            Params(SCENE_QUERY_STAT(AGLSAdvancedLedgeFinder), Settings.bTraceComplex, Character),
            Origin(InOrigin), Forward(InForward)
        {
            for (AActor* Actor : S.ActorsToIgnore)
            {
                if (IsValid(Actor)) { Params.AddIgnoredActor(Actor); }
            }
        }

        bool Spend()
        {
            if (Queries >= S.MaxSceneQueries) { bExhausted = true; return false; }
            ++Queries;
            return true;
        }

        bool Line(const FVector& A, const FVector& B, FHitResult& Hit)
        {
            Hit = FHitResult();
            if (!Spend()) { return false; }
            const bool bHit = World->LineTraceSingleByChannel(Hit, A, B, Channel, Params);
            if (S.bDrawDebug && S.bDrawAllQueries)
            {
                DrawDebugLine(World, A, bHit ? Hit.ImpactPoint : B,
                    bHit ? FColor::Orange : FColor::Cyan, false, S.DebugDuration, 0, 0.3f);
            }
            return bHit;
        }

        bool Sphere(const FVector& A, const FVector& B, float Radius, FHitResult& Hit)
        {
            Hit = FHitResult();
            if (!Spend()) { return false; }
            return World->SweepSingleByChannel(Hit, A, B, FQuat::Identity, Channel,
                FCollisionShape::MakeSphere(Radius), Params);
        }

        bool InSearchBounds(const FVector& P) const
        {
            const FVector D = P - Origin;
            const float Horizontal = FVector(D.X, D.Y, 0.).Size();
            return D.Z <= S.SearchAbove && D.Z >= -S.SearchBelow &&
                Horizontal <= S.SearchDistance && FVector::DotProduct(D, Forward) >= -S.ProfileStep;
        }
    };

    static float Angle(const FVector& A, const FVector& B)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(
            static_cast<float>(FVector::DotProduct(A, B)), -1.f, 1.f)));
    }

    static FVector FlatUnit(const FVector& V)
    {
        return FVector(V.X, V.Y, 0.).GetSafeNormal();
    }

    static FVector RightOf(const FContact& C)
    {
        return FVector::CrossProduct(C.Outward, Up).GetSafeNormal();
    }

    static bool ResolveChannel(FName Name, ECollisionChannel& Channel)
    {
        const UCollisionProfile* Profile = UCollisionProfile::Get();
        if (!Profile || Name.IsNone()) { return false; }
        // Never assume which ECC_GameTraceChannel slot the project assigned to the name.
        for (int32 Index = 0; Index < static_cast<int32>(ECC_MAX); ++Index)
        {
            if (Profile->ReturnChannelNameFromContainerIndex(Index) == Name)
            {
                Channel = static_cast<ECollisionChannel>(Index);
                return true;
            }
        }
        return false;
    }

    // Shared signed geometry score for discovery, variant ordering and final ranking.
    // Missing capsule metrics are not guessed during preliminary ordering.
    static double GeometryScore(const FContext& C, const FVector& Point, double Length)
    {
        const FVector Delta = Point - C.Origin;
        const double SideDistance = (Delta - C.Forward *
            FVector::DotProduct(Delta, C.Forward)).Size();
        return static_cast<double>(C.S.DistanceToSearchOriginWeightScale) * Delta.Size() +
            static_cast<double>(C.S.SideDistanceWeightScale) * SideDistance +
            static_cast<double>(C.S.LedgeLengthWeightScale) * Length;
    }

    static bool ValidateSettings(const FAGLSLedgeSearchSettings& S)
    {
        const auto Positive = [](float V) { return FMath::IsFinite(V) && V > 0.f; };
        const auto NonNegative = [](float V) { return FMath::IsFinite(V) && V >= 0.f; };
        const auto Range = [](float V, float A, float B)
            { return FMath::IsFinite(V) && V >= A && V <= B; };
        return FMath::IsFinite(S.DistanceToSearchOriginWeightScale) &&
            FMath::IsFinite(S.SideDistanceWeightScale) &&
            FMath::IsFinite(S.LedgeLengthWeightScale) &&
            FMath::IsFinite(S.IntrusionAreaWeightScale) &&
            FMath::IsFinite(S.IntrusionDepthWeightScale) &&
            Positive(S.SearchDistance) && NonNegative(S.SearchAbove) &&
            NonNegative(S.SearchBelow) && NonNegative(S.SearchHalfWidth) &&
            Range(S.SearchFanHalfAngle, 0.f, 75.f) && Positive(S.DiscoveryHeightStep) &&
            Positive(S.MinLength) && Positive(S.MaxLength) && S.MaxLength >= S.MinLength &&
            Positive(S.SampleStep) && Positive(S.BoundaryTolerance) &&
            S.BoundaryTolerance <= S.SampleStep && Range(S.MaxLedgeSlope, 0.f, 75.f) &&
            Range(S.MaxTopSlope, 0.f, 85.f) && Range(S.MaxNormalChange, 0.f, 89.f) &&
            NonNegative(S.MaxChordDeviation) && Positive(S.ProfileHeightRange) &&
            Positive(S.ProfileStep) && S.ProfileStep <= S.ProfileHeightRange &&
            Positive(S.ProfileReach) && Positive(S.MaxTransitionDepth) &&
            Positive(S.MinSupportDepth) &&
            S.ProfileReach > S.MaxTransitionDepth + S.MinSupportDepth &&
            Positive(S.GripProbeRadius) && NonNegative(S.GripGap) &&
            NonNegative(S.GripClearanceBelow) && Range(S.MaxIntrusionDepthRatio, 0.f, 1.f) &&
            Range(S.MaxIntrusionAreaFraction, 0.f, 1.f) && NonNegative(S.ContactTolerance) &&
            Positive(S.CapsuleProbeStandOff) && S.MaxCandidates >= 1 && S.MaxCandidates <= 64 &&
            S.CapsuleColumns >= 5 && S.CapsuleColumns <= 65 &&
            S.CapsuleRows >= 5 && S.CapsuleRows <= 65 &&
            S.MaxSegmentVariants >= 1 && S.MaxSegmentVariants <= 128 &&
            S.MaxSceneQueries >= 100 && S.MaxSceneQueries <= 200000 &&
            NonNegative(S.DebugDuration) &&
            // Bound integer loop counts even when called from native code without editor clamps.
            S.SearchAbove / S.DiscoveryHeightStep + S.SearchBelow / S.DiscoveryHeightStep < 2048.f &&
            S.ProfileHeightRange / S.ProfileStep < 128.f &&
            S.MaxTransitionDepth / S.ProfileStep < 128.f &&
            S.MaxLength / S.BoundaryTolerance < 4096.f;
    }

    static void Discover(FContext& C, TArray<FSeed>& Seeds)
    {
        TArray<float> Heights;
        Heights.Add(0.f);
        for (float H = C.S.DiscoveryHeightStep; H <= FMath::Max(C.S.SearchAbove, C.S.SearchBelow);
            H += C.S.DiscoveryHeightStep)
        {
            if (H <= C.S.SearchAbove) { Heights.Add(H); }
            if (H <= C.S.SearchBelow) { Heights.Add(-H); }
        }
        Heights.AddUnique(C.S.SearchAbove);
        Heights.AddUnique(-C.S.SearchBelow);
        const FVector Right = FVector::CrossProduct(Up, C.Forward);
        const float Angles[] = { 0.f, -C.S.SearchFanHalfAngle, C.S.SearchFanHalfAngle };
        const float Offsets[] = { 0.f, -C.S.SearchHalfWidth, C.S.SearchHalfWidth };
        for (float H : Heights)
        {
            for (float Y : Offsets)
            {
                for (float A : Angles)
                {
                    const FVector Start = C.Origin + Up * H + Right * Y;
                    const FVector Direction = C.Forward.RotateAngleAxis(A, Up);
                    FHitResult Hit;
                    if (C.Line(Start, Start + Direction * C.S.SearchDistance, Hit) &&
                        Hit.IsValidBlockingHit() && C.InSearchBounds(Hit.ImpactPoint))
                    {
                        FVector Outward = FlatUnit(Hit.ImpactNormal);
                        // Top-facing hits can occur on pipes; approach supplies the fallback normal.
                        if (Outward.IsNearlyZero()) { Outward = -Direction; }
                        if (FVector::DotProduct(Outward, -Direction) < 0.15) { continue; }
                        bool bDuplicate = false;
                        for (const FSeed& Existing : Seeds)
                        {
                            if (FVector::DistSquared(Existing.Point, Hit.ImpactPoint) <
                                FMath::Square(C.S.DiscoveryHeightStep * 0.75f) &&
                                Angle(Existing.Outward, Outward) < 12.f)
                            {
                                bDuplicate = true; break;
                            }
                        }
                        if (!bDuplicate)
                        {
                            FSeed Seed;
                            Seed.Point = Hit.ImpactPoint;
                            Seed.Outward = Outward;
                            Seed.Score = GeometryScore(C, Seed.Point, 0.);
                            FHitResult Above;
                            const FVector AboveStart = Start + Up * C.S.DiscoveryHeightStep;
                            const bool bAboveHit = C.Line(AboveStart,
                                AboveStart + Direction * C.S.SearchDistance, Above);
                            // Prioritize depth retreats/openings so nearby flat wall seeds
                            // do not consume the profile budget before a higher ledge.
                            if (!bAboveHit || (Above.IsValidBlockingHit() &&
                                Above.Distance > Hit.Distance + C.S.MinSupportDepth))
                            {
                                Seed.Score -= 2.f * C.S.SearchDistance;
                            }
                            Seeds.Add(Seed);
                        }
                    }
                    if (C.bExhausted) { return; }
                }
            }
        }
        Seeds.Sort([](const FSeed& A, const FSeed& B) { return A.Score < B.Score; });
    }

    /** Only accepts a top probe reachable horizontally from outside. A local ceiling
     * may invalidate the high approach; lower approaches are tried for recessed slots.
     */
    static bool ProbeTop(FContext& C, const FHitResult& Side, const FVector& Outward,
        float Depth, FHitResult& Top)
    {
        const float Rises[] = { C.S.ProfileHeightRange, C.S.ProfileHeightRange * 0.5f, C.S.ProfileStep };
        const float MinUp = FMath::Cos(FMath::DegreesToRadians(C.S.MaxTopSlope));
        for (float Rise : Rises)
        {
            const FVector Start = Side.ImpactPoint + Outward * C.S.ProfileReach + Up * Rise;
            const FVector Over = Side.ImpactPoint - Outward * Depth + Up * Rise;
            FHitResult Block;
            if (C.Line(Start, Over, Block)) { continue; }
            if (C.bExhausted) { return false; }
            if (C.Line(Over, Side.ImpactPoint - Outward * Depth - Up * C.S.ProfileStep, Top) &&
                Top.IsValidBlockingHit() && FVector::DotProduct(Top.ImpactNormal, Up) >= MinUp)
            {
                return true;
            }
            if (C.bExhausted) { return false; }
        }
        return false;
    }

    static FVector GripPosition(const FContact& P, const FAGLSLedgeSearchSettings& S)
    {
        return P.Point + P.Outward * (S.GripProbeRadius + S.GripGap + P.ApproachDepth);
    }

    static bool GripClear(FContext& C, const FContact& P)
    {
        const FVector At = GripPosition(P, C.S);
        FHitResult Hit;
        const bool bHit = C.Sphere(At + Up * (C.S.GripProbeRadius + C.S.GripGap),
            At - Up * C.S.GripClearanceBelow, C.S.GripProbeRadius, Hit);
        return !bHit && !C.bExhausted;
    }

    /** Reconstructs the nearest accessible top transition around a predicted contact. */
    static bool FindContact(FContext& C, const FVector& Predicted, const FVector& ExpectedOut,
        float HeightWindow, FContact& Out)
    {
        const int32 Levels = FMath::CeilToInt(HeightWindow / C.S.ProfileStep);
        for (int32 Level = 0; Level <= 2 * Levels; ++Level)
        {
            const int32 OffsetIndex = Level == 0 ? 0 : ((Level + 1) / 2) * (Level % 2 ? -1 : 1);
            const float DZ = OffsetIndex * C.S.ProfileStep;
            const FVector Base = Predicted + Up * DZ;
            FHitResult Side;
            if (!C.Line(Base + ExpectedOut * C.S.ProfileReach,
                Base - ExpectedOut * C.S.ProfileReach, Side) || !Side.IsValidBlockingHit())
            {
                if (C.bExhausted) { return false; }
                continue;
            }
            FVector Outward = FlatUnit(Side.ImpactNormal);
            if (Outward.IsNearlyZero() || FVector::DotProduct(Outward, ExpectedOut) < 0.5) { continue; }

            // Coarse-to-fine locate the first top-normal-supported point, including a bevel.
            float LastFailedDepth = -C.S.ProfileStep;
            const int32 DepthSteps = FMath::CeilToInt(C.S.MaxTransitionDepth / C.S.ProfileStep);
            for (int32 D = 0; D <= DepthSteps; ++D)
            {
                const float Depth = FMath::Min(D * C.S.ProfileStep, C.S.MaxTransitionDepth);
                FHitResult Top;
                if (!ProbeTop(C, Side, Outward, Depth, Top))
                {
                    if (C.bExhausted) { return false; }
                    LastFailedDepth = Depth;
                    continue;
                }
                float Low = LastFailedDepth;
                float High = Depth;
                for (int32 Refine = 0; Refine < 8 && High - Low > C.S.BoundaryTolerance; ++Refine)
                {
                    FHitResult Refined;
                    const float Mid = 0.5f * (Low + High);
                    if (ProbeTop(C, Side, Outward, Mid, Refined)) { High = Mid; Top = Refined; }
                    else { Low = Mid; }
                    if (C.bExhausted) { return false; }
                }
                if (!C.InSearchBounds(Top.ImpactPoint) ||
                    FMath::Abs(Top.ImpactPoint.Z - Predicted.Z) > HeightWindow + C.S.ProfileStep ||
                    FVector::Distance(Top.ImpactPoint, Predicted) > C.S.ProfileReach)
                {
                    break;
                }

                FHitResult Support;
                if (!ProbeTop(C, Side, Outward, High + C.S.MinSupportDepth, Support)) { break; }
                const float AllowedRise = C.S.MinSupportDepth *
                    FMath::Tan(FMath::DegreesToRadians(C.S.MaxTopSlope)) + C.S.BoundaryTolerance;
                if (FMath::Abs(Support.ImpactPoint.Z - Top.ImpactPoint.Z) > AllowedRise) { break; }

                // Check the interval between front contact and deeper support as well.
                bool bSupportContinuous = true;
                const int32 SupportSteps = FMath::CeilToInt(C.S.MinSupportDepth / C.S.ProfileStep);
                for (int32 I = 1; I < SupportSteps; ++I)
                {
                    FHitResult Interior;
                    if (!ProbeTop(C, Side, Outward,
                        High + C.S.MinSupportDepth * I / SupportSteps, Interior) ||
                        FMath::Abs(Interior.ImpactPoint.Z - Top.ImpactPoint.Z) > AllowedRise)
                    {
                        bSupportContinuous = false; break;
                    }
                }
                if (!bSupportContinuous || C.bExhausted) { break; }
                FContact P;
                P.Point = Top.ImpactPoint;
                P.Outward = Outward;
                P.TopNormal = Top.ImpactNormal;
                P.Component = Top.GetComponent();
                P.ApproachDepth = FMath::Max(0.f, static_cast<float>(
                    FVector::DotProduct(Side.ImpactPoint - Top.ImpactPoint, Outward)));
                P.bRounded = High > 2.f * C.S.BoundaryTolerance ||
                    FVector::DotProduct(Top.ImpactNormal, Up) < 0.97;
                if (GripClear(C, P)) { Out = P; return true; }
                break;
            }
            if (C.bExhausted) { return false; }
        }
        return false;
    }

    static bool Connect(FContext& C, const FContact& A, const FContact& B,
        const FVector& TravelDirection, float ExpectedStep)
    {
        const FVector Delta = B.Point - A.Point;
        const float Progress = FVector::DotProduct(Delta, TravelDirection);
        const float Length = Delta.Size();
        if (Progress < 0.25f * ExpectedStep || Length > 1.8f * ExpectedStep + C.S.BoundaryTolerance ||
            Length < UE_SMALL_NUMBER || Angle(A.Outward, B.Outward) > C.S.MaxNormalChange ||
            FMath::Abs(Delta.Z) / Length > FMath::Sin(FMath::DegreesToRadians(C.S.MaxLedgeSlope)))
        {
            return false;
        }
        // Verify support between endpoints at ProfileStep resolution. An empty
        // finger-space sweep alone does not prove that a supporting ledge exists.
        const int32 InteriorSteps = FMath::CeilToInt(Length / C.S.ProfileStep);
        for (int32 I = 1; I < InteriorSteps; ++I)
        {
            const float Alpha = static_cast<float>(I) / InteriorSteps;
            const FVector Predicted = FMath::Lerp(A.Point, B.Point, Alpha);
            const FVector Normal = FMath::Lerp(A.Outward, B.Outward, Alpha).GetSafeNormal();
            FContact Interior;
            if (!FindContact(C, Predicted, Normal, C.S.ProfileStep, Interior) ||
                FVector::Distance(Interior.Point, Predicted) >
                C.S.MaxChordDeviation + C.S.BoundaryTolerance ||
                Angle(Interior.Outward, Normal) > C.S.MaxNormalChange)
            {
                return false;
            }
        }
        FHitResult Hit;
        // Continuous finger-space sweep catches narrow blockers between contact samples.
        if (C.Sphere(GripPosition(A, C.S) - Up * C.S.GripClearanceBelow,
            GripPosition(B, C.S) - Up * C.S.GripClearanceBelow, C.S.GripProbeRadius, Hit))
        {
            return false;
        }
        return !C.bExhausted;
    }

    static void Follow(FContext& C, const FContact& Seed, float Sign, TArray<FContact>& Contacts)
    {
        FContact Current = Seed;
        FVector Tangent = RightOf(Seed) * Sign;
        float Travel = 0.f;
        const int32 Limit = FMath::Min(256, FMath::CeilToInt(C.S.MaxLength / C.S.BoundaryTolerance) + 1);
        for (int32 I = 0; I < Limit && Travel < C.S.MaxLength; ++I)
        {
            float Step = FMath::Min(C.S.SampleStep, C.S.MaxLength - Travel);
            bool bFound = false;
            FContact Next;
            for (int32 Refine = 0; Refine < 12 && Step >= C.S.BoundaryTolerance; ++Refine)
            {
                const FVector Predicted = Current.Point + Tangent * Step;
                const float Window = FMath::Min(C.S.ProfileHeightRange,
                    Step * FMath::Tan(FMath::DegreesToRadians(C.S.MaxLedgeSlope)) + C.S.ProfileStep);
                if (FindContact(C, Predicted, Current.Outward, Window, Next) &&
                    Connect(C, Current, Next, Tangent, Step))
                {
                    bFound = true; break;
                }
                if (C.bExhausted) { return; }
                Step *= 0.5f;
            }
            if (!bFound) { return; }
            const FVector Move = Next.Point - Current.Point;
            Travel += Move.Size();
            Contacts.Add(Next);
            // Keep measured slope and update horizontal heading as surface normals turn.
            FVector FlatTangent = RightOf(Next) * Sign;
            const float HorizontalTravel = FVector(Move.X, Move.Y, 0.).Size();
            Tangent = (FlatTangent + Up * (Move.Z / FMath::Max(HorizontalTravel, 0.01f))).GetSafeNormal();
            Current = Next;
            if (C.bExhausted) { return; }
        }
        if (Travel < C.S.MaxLength) { C.bTruncated = true; }
    }

    struct FVariant
    {
        int32 First = 0;
        int32 Last = 0;
        float EndAlpha = 1.f;
        double Rank = 0.;
    };

    static FTransform ContactTransform(const FContact& P, FVector Tangent)
    {
        if (FVector::DotProduct(Tangent, RightOf(P)) < 0.) { Tangent = -Tangent; }
        // Preserve measured tangent, orthogonalize inward normal; no Euler interpolation.
        const FQuat Rotation = FRotationMatrix::MakeFromYX(Tangent, -P.Outward).ToQuat();
        return FTransform(Rotation, P.Point, FVector::OneVector);
    }

    static FContact InterpolateContact(const FContact& A, const FContact& B, float Alpha)
    {
        FContact P = A;
        P.Point = FMath::Lerp(A.Point, B.Point, Alpha);
        P.Outward = FMath::Lerp(A.Outward, B.Outward, Alpha).GetSafeNormal();
        P.TopNormal = FMath::Lerp(A.TopNormal, B.TopNormal, Alpha).GetSafeNormal();
        P.bRounded = A.bRounded || B.bRounded;
        P.ApproachDepth = FMath::Lerp(A.ApproachDepth, B.ApproachDepth, Alpha);
        if (Alpha > 0.5f) { P.Component = B.Component; }
        return P;
    }

    static bool BuildSegment(FContext& C, const TArray<FContact>& InContacts,
        const FVariant& V, FAGLSLedgeResult& Result)
    {
        TArray<FContact> Contacts;
        for (int32 I = V.First; I <= V.Last; ++I) { Contacts.Add(InContacts[I]); }
        if (V.EndAlpha < 1.f)
        {
            const FContact Predicted = InterpolateContact(Contacts[Contacts.Num() - 2],
                Contacts.Last(), V.EndAlpha);
            FContact End;
            if (!FindContact(C, Predicted.Point, Predicted.Outward, C.S.ProfileStep, End) ||
                FVector::Distance(End.Point, Predicted.Point) > C.S.BoundaryTolerance)
            {
                return false;
            }
            Contacts.Last() = End;
        }
        const int32 First = 0;
        const int32 Last = Contacts.Num() - 1;
        const FContact& Left = Contacts[First];
        const FContact& Right = Contacts[Last];
        const FVector Chord = Right.Point - Left.Point;
        const float Length = Chord.Size();
        if (Length < C.S.MinLength - C.S.BoundaryTolerance || Length > C.S.MaxLength + C.S.BoundaryTolerance || Length < UE_SMALL_NUMBER)
        {
            return false;
        }
        const FVector Tangent = Chord / Length;
        if (FMath::Abs(Tangent.Z) > FMath::Sin(FMath::DegreesToRadians(C.S.MaxLedgeSlope)))
        {
            return false;
        }
        float Arc = 0.f;
        for (int32 I = First; I <= Last; ++I)
        {
            const FVector Delta = Contacts[I].Point - Left.Point;
            const double Along = FMath::Clamp(FVector::DotProduct(Delta, Tangent), 0., static_cast<double>(Length));
            if ((Delta - Tangent * Along).Size() > C.S.MaxChordDeviation ||
                Angle(Contacts[I].Outward, Left.Outward) > C.S.MaxNormalChange)
            {
                return false;
            }
            if (I > First) { Arc += FVector::Distance(Contacts[I - 1].Point, Contacts[I].Point); }
        }
        float Traversed = 0.f;
        FContact Center = Left;
        FVector CenterTangent = Tangent;
        for (int32 I = First + 1; I <= Last; ++I)
        {
            const float Distance = FVector::Distance(Contacts[I - 1].Point, Contacts[I].Point);
            if (Traversed + Distance >= Arc * 0.5f)
            {
                const float Alpha = (Arc * 0.5f - Traversed) / FMath::Max(Distance, 0.001f);
                const FContact Predicted = InterpolateContact(Contacts[I - 1], Contacts[I], Alpha);
                // The arithmetic midpoint of two samples may lie inside a convex cylinder.
                // Reproject through the same profile tests instead of returning that point.
                if (!FindContact(C, Predicted.Point, Predicted.Outward,
                    C.S.ProfileStep + C.S.MaxChordDeviation, Center) ||
                    FVector::Distance(Center.Point, Predicted.Point) >
                    C.S.MaxChordDeviation + C.S.BoundaryTolerance)
                {
                    return false;
                }
                CenterTangent = (Contacts[I].Point - Contacts[I - 1].Point).GetSafeNormal();
                break;
            }
            Traversed += Distance;
        }
        if (Angle(Center.Outward, Left.Outward) > C.S.MaxNormalChange) { return false; }
        Result = FAGLSLedgeResult();
        Result.LeftTransform = ContactTransform(Left, Contacts[First + 1].Point - Left.Point);
        Result.RightTransform = ContactTransform(Right, Right.Point - Contacts[Last - 1].Point);
        Result.CenterTransform = ContactTransform(Center, CenterTangent);
        Result.Length = Length;
        Result.ArcLength = Arc;
        Result.SurfaceComponent = Center.Component;
        for (int32 I = First; I <= Last; ++I)
        {
            Result.ContactSamples.Add(Contacts[I].Point);
            Result.bLikelyRoundedOrBeveled |= Contacts[I].bRounded;
            Result.bSpansMultipleComponents |= Contacts[I].Component != Center.Component;
        }
        return !C.bExhausted;
    }

    /** Conservative front-visible surface estimate for solid environment collision.
     * For each Y/Z sample, compare the first surface to the capsule's rear boundary.
     * This measures axial depth and weighted silhouette area, not MTD or solid volume.
     * A foreground obstruction/start penetration is ambiguous and rejects the variant.
     */
    static bool EvaluateCapsule(FContext& C, float Radius, float HalfHeight,
        float HorizontalOffsetFactor, float VerticalOffset, FAGLSLedgeResult& Result)
    {
        const FVector Outward = -FlatUnit(Result.CenterTransform.GetUnitAxis(EAxis::X));
        if (Outward.IsNearlyZero()) { return false; }
        const FVector Right = FVector::CrossProduct(Outward, Up).GetSafeNormal();
        const FVector Center = Result.CenterTransform.GetLocation() +
            Outward * (HorizontalOffsetFactor * Radius) -
            Result.CenterTransform.GetUnitAxis(EAxis::Z) * VerticalOffset;
        Result.CapsuleTransform = FTransform(FQuat::Identity, Center, FVector::OneVector);
        Result.CapsuleRadius = Radius;
        Result.CapsuleHalfHeight = HalfHeight;
        const float CylinderHalf = HalfHeight - Radius;
        const float AllowedDepth = C.S.MaxIntrusionDepthRatio * Radius;
        double TotalArea = 0.;
        double IntrudedArea = 0.;
        float MaxDepth = 0.f;
        bool bInvalid = false;
        const float DZ = 2.f * HalfHeight / C.S.CapsuleRows;
        for (int32 Row = 0; Row < C.S.CapsuleRows; ++Row)
        {
            const float Z = -HalfHeight + (Row + 0.5f) * DZ;
            const float CapZ = FMath::Max(0.f, FMath::Abs(Z) - CylinderHalf);
            const float SectionRadius = FMath::Sqrt(FMath::Max(0.f, Radius * Radius - CapZ * CapZ));
            const float DY = 2.f * SectionRadius / C.S.CapsuleColumns;
            for (int32 Column = 0; Column < C.S.CapsuleColumns; ++Column)
            {
                const float Y = -SectionRadius + (Column + 0.5f) * DY;
                const float X = FMath::Sqrt(FMath::Max(0.f, SectionRadius * SectionRadius - Y * Y));
                const FVector Axis = Center + Right * Y + Up * Z;
                const FVector Start = Axis + Outward * (X + C.S.CapsuleProbeStandOff);
                const FVector End = Axis - Outward * (X + C.S.ContactTolerance);
                FHitResult Hit;
                const bool bHit = C.Line(Start, End, Hit);
                if (C.bExhausted) { return false; }
                float Depth = 0.f;
                if (bHit)
                {
                    const float SurfaceX = FVector::DotProduct(Hit.ImpactPoint - Axis, Outward);
                    if (!Hit.IsValidBlockingHit() || SurfaceX > X + C.S.ContactTolerance)
                    {
                        bInvalid = true;
                    }
                    Depth = FMath::Max(0.f, SurfaceX + X);
                    // A back-facing exit is not proof that the start was free.
                    if (FVector::DotProduct(Hit.ImpactNormal, Outward) <= 0.) { bInvalid = true; }
                }
                const double Weight = static_cast<double>(DY) * DZ;
                TotalArea += Weight;
                if (Depth > C.S.ContactTolerance) { IntrudedArea += Weight; }
                MaxDepth = FMath::Max(MaxDepth, Depth);
                if (C.S.bDrawDebug && C.S.bDrawAllQueries && bHit)
                {
                    DrawDebugPoint(C.World, Hit.ImpactPoint, 4.f,
                        Depth > AllowedDepth + C.S.ContactTolerance ? FColor::Red : FColor::Yellow,
                        false, C.S.DebugDuration);
                }
            }
        }
        Result.EstimatedMaxIntrusionDepth = MaxDepth;
        Result.EstimatedIntrusionAreaFraction = TotalArea > 0. ? IntrudedArea / TotalArea : 1.f;
        const bool bAccepted = !bInvalid && MaxDepth <= AllowedDepth + C.S.ContactTolerance &&
            Result.EstimatedIntrusionAreaFraction <= C.S.MaxIntrusionAreaFraction;
        if (C.S.bDrawDebug)
        {
            DrawDebugCapsule(C.World, Center, HalfHeight, Radius, FQuat::Identity,
                bAccepted ? FColor::Green : FColor::Red, false, C.S.DebugDuration, 0, 0.5f);
            DrawDebugString(C.World, Center,
                FString::Printf(TEXT("depth %.2f/%.2f; area %.1f/%.1f%%%s"), MaxDepth,
                    AllowedDepth + C.S.ContactTolerance,
                    100.f * Result.EstimatedIntrusionAreaFraction,
                    100.f * C.S.MaxIntrusionAreaFraction,
                    bInvalid ? TEXT(" [occluded/ambiguous]") : TEXT("")),
                nullptr, bAccepted ? FColor::Green : FColor::Red, C.S.DebugDuration, false);
        }
        return bAccepted;
    }

    static void MakeVariants(const TArray<FContact>& Contacts, FContext& C,
        TArray<FVariant>& Variants)
    {
        for (int32 First = 0; First + 1 < Contacts.Num(); ++First)
        {
            for (int32 Last = First + 1; Last < Contacts.Num(); ++Last)
            {
                float Length = FVector::Distance(Contacts[First].Point, Contacts[Last].Point);
                if (Length < C.S.MinLength - C.S.BoundaryTolerance) { continue; }
                FVariant V;
                V.First = First;
                V.Last = Last;
                FVector End = Contacts[Last].Point;
                if (Length > C.S.MaxLength)
                {
                    // Intersect the last sampled interval with a sphere around the
                    // left endpoint: exact desired chord length before reprojection.
                    const FVector A = Contacts[Last - 1].Point - Contacts[First].Point;
                    if (A.Size() >= C.S.MaxLength) { continue; }
                    const FVector D = Contacts[Last].Point - Contacts[Last - 1].Point;
                    const double AA = D.SizeSquared();
                    const double BB = 2. * FVector::DotProduct(A, D);
                    const double CC = A.SizeSquared() - FMath::Square(C.S.MaxLength);
                    const double Disc = BB * BB - 4. * AA * CC;
                    if (AA < UE_SMALL_NUMBER || Disc < 0.) { continue; }
                    V.EndAlpha = FMath::Clamp((-BB + FMath::Sqrt(Disc)) / (2. * AA), 0., 1.);
                    End = FMath::Lerp(Contacts[Last - 1].Point, Contacts[Last].Point, V.EndAlpha);
                    Length = C.S.MaxLength;
                }
                const FVector Mid = (Contacts[First].Point + End) * 0.5;
                V.Rank = GeometryScore(C, Mid, Length);
                int32 InsertAt = 0;
                while (InsertAt < Variants.Num() && Variants[InsertAt].Rank <= V.Rank) { ++InsertAt; }
                if (InsertAt < C.S.MaxSegmentVariants)
                {
                    Variants.Insert(V, InsertAt);
                    if (Variants.Num() > C.S.MaxSegmentVariants)
                    {
                        C.bTruncated = true; Variants.RemoveAt(Variants.Num() - 1);
                    }
                }
                else { C.bTruncated = true; }
            }
        }
    }

    static void DrawResult(FContext& C, const FAGLSLedgeResult& R)
    {
        if (!C.S.bDrawDebug) { return; }
        DrawDebugLine(C.World, R.LeftTransform.GetLocation(), R.RightTransform.GetLocation(),
            FColor::Green, false, C.S.DebugDuration, 0, 3.f);
        for (const FTransform* T : { &R.LeftTransform, &R.CenterTransform, &R.RightTransform })
        {
            DrawDebugPoint(C.World, T->GetLocation(), 10.f, FColor::Yellow, false, C.S.DebugDuration);
            DrawDebugDirectionalArrow(C.World, T->GetLocation(),
                T->GetLocation() + T->GetUnitAxis(EAxis::X) * 20.f, 5.f,
                FColor::Cyan, false, C.S.DebugDuration, 0, 1.f);
        }
        DrawDebugString(C.World, R.CenterTransform.GetLocation() + Up * 20.f,
            FString::Printf(TEXT("L=%.1f  depth=%.2f  area=%.1f%%  queries=%d%s"),
                R.Length, R.EstimatedMaxIntrusionDepth, 100.f * R.EstimatedIntrusionAreaFraction,
                R.SceneQueryCount, R.bQueryBudgetExhausted ? TEXT(" [budget]") : TEXT("")),
            nullptr, FColor::White, C.S.DebugDuration, false);
    }
}

bool UAGLS_AdvancedLedgesFinder::TryFindLedgeForClimbing(
    ACharacter* Character, FVector SearchOrigin, FVector SearchDirection,
    FVector2D CapsuleScale, float CapsuleHorizontalOffsetFactor, float CapsuleVerticalOffset,
    const FAGLSLedgeSearchSettings& Settings, FAGLSLedgeResult& OutResult)
{
    using namespace AGLSLedgeFinder;
    OutResult = FAGLSLedgeResult();
    OutResult.Failure = EAGLSLedgeFailure::InvalidInput;
    if (!IsInGameThread() || !IsValid(Character) || !Character->GetWorld() ||
        !IsValid(Character->GetCapsuleComponent()) || SearchOrigin.ContainsNaN() ||
        SearchDirection.ContainsNaN() || CapsuleScale.ContainsNaN() ||
        CapsuleScale.X <= 0. || CapsuleScale.Y <= 0. ||
        !FMath::IsFinite(CapsuleHorizontalOffsetFactor) || CapsuleHorizontalOffsetFactor < 0.f ||
        !FMath::IsFinite(CapsuleVerticalOffset) || !ValidateSettings(Settings))
    {
        return false;
    }
    const FVector Forward = FlatUnit(SearchDirection);
    if (Forward.IsNearlyZero()) { return false; }
    const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    // Search bounds, silhouette layers and the agreed XY convention use world Z.
    if (FVector::DotProduct(Capsule->GetUpVector(), Up) < 0.9999) { return false; }
    float Radius = 0.f;
    float HalfHeight = 0.f;
    Capsule->GetScaledCapsuleSize(Radius, HalfHeight);
    Radius *= CapsuleScale.X;
    HalfHeight *= CapsuleScale.Y;
    if (!FMath::IsFinite(Radius) || !FMath::IsFinite(HalfHeight) ||
        Radius <= 0.f || HalfHeight < Radius) {
        return false;
    }
    ECollisionChannel Channel = ECC_Visibility;
    if (!ResolveChannel(Settings.TraceChannelName, Channel))
    {
        OutResult.Failure = EAGLSLedgeFailure::MissingCollisionChannel;
        return false;
    }
    FContext C(Character->GetWorld(), Character, Settings, Channel, SearchOrigin, Forward);
    TArray<FSeed> Seeds;
    Discover(C, Seeds);
    TArray<FContact> Processed;
    double BestScore = TNumericLimits<double>::Max();
    bool bFound = false;
    bool bCapsuleRejected = false;
    int32 Tested = 0;
    // Failed seed profiles are bounded independently of successful candidates.
    const int32 SeedLimit = FMath::Min(Seeds.Num(), Settings.MaxCandidates * 12);
    C.bTruncated |= SeedLimit < Seeds.Num();
    for (int32 SeedIndex = 0; SeedIndex < SeedLimit && Tested < Settings.MaxCandidates && !C.bExhausted; ++SeedIndex)
    {
        const FSeed& Seed = Seeds[SeedIndex];
        FContact Contact;
        if (!FindContact(C, Seed.Point, Seed.Outward, Settings.ProfileHeightRange, Contact)) { continue; }
        bool bDuplicate = false;
        for (const FContact& Previous : Processed)
        {
            if (FVector::Distance(Previous.Point, Contact.Point) < Settings.SampleStep &&
                Angle(Previous.Outward, Contact.Outward) < 10.f)
            {
                bDuplicate = true; break;
            }
        }
        if (bDuplicate) { continue; }
        Processed.Add(Contact);
        ++Tested;
        TArray<FContact> Left;
        TArray<FContact> Right;
        Follow(C, Contact, -1.f, Left);
        Follow(C, Contact, 1.f, Right);
        if (C.bExhausted) { break; }
        TArray<FContact> Contacts;
        for (int32 I = Left.Num() - 1; I >= 0; --I) { Contacts.Add(Left[I]); }
        Contacts.Add(Contact);
        Contacts.Append(Right);
        if (Contacts.Num() < 2) { continue; }
        TArray<AGLSLedgeFinder::FVariant> Variants;
        MakeVariants(Contacts, C, Variants);
        for (const AGLSLedgeFinder::FVariant& V : Variants)
        {
            FAGLSLedgeResult Candidate;
            if (!BuildSegment(C, Contacts, V, Candidate))
            {
                if (C.bExhausted) { break; }
                continue;
            }
            if (!EvaluateCapsule(C, Radius, HalfHeight, CapsuleHorizontalOffsetFactor,
                CapsuleVerticalOffset, Candidate))
            {
                bCapsuleRejected = true;
                if (C.bExhausted) { break; }
                continue;
            }
            const double Score = GeometryScore(C,
                Candidate.CenterTransform.GetLocation(), Candidate.Length) +
                static_cast<double>(Settings.IntrusionAreaWeightScale) * Radius *
                Candidate.EstimatedIntrusionAreaFraction +
                static_cast<double>(Settings.IntrusionDepthWeightScale) *
                Candidate.EstimatedMaxIntrusionDepth;
            if (Score < BestScore)
            {
                BestScore = Score;
                OutResult = MoveTemp(Candidate);
                bFound = true;
            }
            // Compare subsequent valid variants too: their capsule scores can
            // reverse the preliminary geometry-only ordering. Existing variant and
            // scene-query budgets bound the work; keep a fully evaluated best result.
        }
    }
    OutResult.bSearchTruncated = C.bTruncated || Tested >= Settings.MaxCandidates || C.bExhausted;
    OutResult.SceneQueryCount = C.Queries;
    OutResult.TestedCandidates = Tested;
    OutResult.bQueryBudgetExhausted = C.bExhausted;
    if (bFound)
    {
        OutResult.Failure = EAGLSLedgeFailure::None;
        DrawResult(C, OutResult);
        return true; // Only a completely evaluated candidate can reach this branch.
    }
    OutResult.Failure = C.bExhausted ? EAGLSLedgeFailure::QueryBudgetExceeded :
        (bCapsuleRejected ? EAGLSLedgeFailure::CapsuleRejected :
            (Seeds.IsEmpty() ? EAGLSLedgeFailure::NoCandidate : EAGLSLedgeFailure::NoUsableSegment));
    if (Settings.bDrawDebug)
    {
        DrawDebugString(C.World, SearchOrigin, FString::Printf(TEXT("Ledge failed: %d; queries=%d"),
            static_cast<int32>(OutResult.Failure), C.Queries), nullptr, FColor::Red,
            Settings.DebugDuration, false);
    }
    return false;
}
