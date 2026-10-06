// Fill out your copyright notice in the Description page of Project Settings.


#include "FootContactAnimModifier.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimTypes.h"
#include "AnimationRuntime.h"
#include "ReferenceSkeleton.h"
#include "Misc/MemStack.h"
#include "AnimPose.h"

#define LOCTEXT_NAMESPACE "FootContactAnimModifier"

void UFootContactAnimModifier::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
    if (!AnimationSequence)
    {
        return;
    }

    BuildContactCurve(AnimationSequence, LeftFootBoneName, LeftContactCurveName, LeftPhaseOffset, LeftContactCurveColor);
    BuildContactCurve(AnimationSequence, RightFootBoneName, RightContactCurveName, RightPhaseOffset, RightContactCurveColor);
}

void UFootContactAnimModifier::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
    if (!AnimationSequence)
    {
        return;
    }

    //RemoveCurveIfExists(AnimationSequence, LeftContactCurveName);
    //RemoveCurveIfExists(AnimationSequence, RightContactCurveName);
}

void UFootContactAnimModifier::RemoveCurveIfExists(UAnimSequence* AnimationSequence, FName CurveName) const
{
    if (!AnimationSequence || CurveName.IsNone())
    {
        return;
    }

    IAnimationDataController& Controller = AnimationSequence->GetController();
    const FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);

    Controller.OpenBracket(LOCTEXT("RemoveFootContactCurve", "Remove foot contact curve"), true);
    Controller.RemoveCurve(CurveId, true);
    Controller.CloseBracket(true);
}

bool UFootContactAnimModifier::SampleBoneComponentSpace(
    const UAnimSequence* AnimationSequence,
    FName BoneName,
    float Time,
    FTransform& OutComponentSpaceTransform) const
{
    FMemMark Mark(FMemStack::Get());

    if (!AnimationSequence || BoneName.IsNone())
    {
        return false;
    }

    const USkeleton* Skeleton = AnimationSequence->GetSkeleton();
    if (!Skeleton)
    {
        return false;
    }

    const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
    const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
    if (BoneIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("FootContactAnimModifier: Bone '%s' not found in skeleton."), *BoneName.ToString());
        return false;
    }

    // UWAGA:
    // Ten wariant u¿ywa GetBonePose/GetAnimationPose. W zale¿noœci od dok³adnego branchu UE 5.7
    // sygnatura mo¿e minimalnie ró¿niæ siê miêdzy wersjami. Sam algorytm pozostaje bez zmian.
    TArray<FBoneIndexType> RequiredBoneIndexArray;
    RequiredBoneIndexArray.Reserve(BoneIndex + 1);

    for (int32 Index = 0; Index <= BoneIndex; ++Index)
    {
        RequiredBoneIndexArray.Add(static_cast<FBoneIndexType>(Index));
    }

    FBoneContainer BoneContainer;
    BoneContainer.InitializeTo(
        RequiredBoneIndexArray,
        UE::Anim::FCurveFilterSettings(),
        *Skeleton);

    FCompactPose CompactPose;
    CompactPose.SetBoneContainer(&BoneContainer);
    CompactPose.ResetToRefPose();

    FBlendedCurve Curve;
    Curve.InitFrom(BoneContainer);

    UE::Anim::FStackAttributeContainer Attributes;
    FAnimationPoseData PoseData(CompactPose, Curve, Attributes);

    FAnimExtractContext ExtractContext(Time, false);
    AnimationSequence->GetAnimationPose(PoseData, ExtractContext);

    const FCompactPoseBoneIndex CompactBoneIndex = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex));
    if (CompactBoneIndex == INDEX_NONE)
    {
        return false;
    }

    TArray<FTransform> ComponentSpaceTransforms;
    ComponentSpaceTransforms.SetNum(BoneContainer.GetCompactPoseNumBones());

    for (const FCompactPoseBoneIndex PoseBoneIndex : CompactPose.ForEachBoneIndex())
    {
        const FCompactPoseBoneIndex ParentIndex = BoneContainer.GetParentBoneIndex(PoseBoneIndex);

        if (ParentIndex != INDEX_NONE)
        {
            ComponentSpaceTransforms[PoseBoneIndex.GetInt()] =
                CompactPose[PoseBoneIndex] * ComponentSpaceTransforms[ParentIndex.GetInt()];
        }
        else
        {
            ComponentSpaceTransforms[PoseBoneIndex.GetInt()] = CompactPose[PoseBoneIndex];
        }
    }

    OutComponentSpaceTransform = ComponentSpaceTransforms[CompactBoneIndex.GetInt()];
    return true;
}

void UFootContactAnimModifier::BuildContactCurve(
    UAnimSequence* AnimationSequence,
    FName FootBoneName,
    FName CurveName, 
    float PhaseOffset, 
    FLinearColor CurveColor) const
{
    if (!AnimationSequence || FootBoneName.IsNone() || CurveName.IsNone())
    {
        return;
    }

    const float PlayLength = AnimationSequence->GetPlayLength();
    if (PlayLength <= 0.0f || SampleRate <= 0.0f)
    {
        return;
    }

    const float Dt = 1.0f / SampleRate;
    const int32 NumSamples = FMath::Max(1, FMath::FloorToInt(PlayLength * SampleRate) + 1);

    TArray<FSample> Samples;
    Samples.SetNum(NumSamples);

    float MinFootZ = TNumericLimits<float>::Max();

    for (int32 Index = 0; Index < NumSamples; ++Index)
    {
        const float Time = FMath::Min(Index * Dt, PlayLength);

        FTransform FootCS;
        FTransform RootCS;

        const bool bFootOk = SampleBoneComponentSpace(AnimationSequence, FootBoneName, Time, FootCS);
        const bool bRootOk = ExtractRootMotion(AnimationSequence, RootBoneName, Time, RootCS);

        if (!bFootOk)
        {
            return;
        }

        Samples[Index].Time = Time;
        Samples[Index].FootCS = FootCS.GetLocation();
        Samples[Index].RootCS = bRootOk ? RootCS.GetLocation() : FVector::ZeroVector;

        MinFootZ = FMath::Min(MinFootZ, Samples[Index].FootCS.Z);
    }

    const float GroundZ = MinFootZ;
    const float HeightLimit = GroundZ + HeightTolerance;

    TArray<uint8> ContactMask;
    ContactMask.SetNumZeroed(NumSamples);

    TArray<double> Speeds;

    for (int32 Index = 0; Index < NumSamples; ++Index)
    {
        const int32 PrevIndex = FMath::Max(Index - 1, 0);
        const int32 NextIndex = FMath::Min(Index + 1, NumSamples - 1);

        const double TimeDelta = FMath::Max(Samples[NextIndex].Time - Samples[PrevIndex].Time, KINDA_SMALL_NUMBER);

        // Pozycja stopy skorygowana o root motion.
        // Dziêki temu stopa "stoj¹ca na ziemi" ma ma³¹ prêdkoœæ, nawet gdy root przesuwa siê do przodu.
        const FVector PrevRel = Samples[PrevIndex].FootCS - Samples[PrevIndex].RootCS;
        const FVector NextRel = Samples[NextIndex].FootCS - Samples[NextIndex].RootCS;
        const FVector VelocityRel = (NextRel - PrevRel) / TimeDelta;
        const FVector FootSpeed = (Samples[NextIndex].FootCS - Samples[PrevIndex].FootCS) / TimeDelta;

        const double PlanarSpeed = FVector(VelocityRel.X, VelocityRel.Y, 0.0f).Size();
        const double VerticalSpeed = FMath::Abs(VelocityRel.Z);

        const bool bLowEnough = Samples[Index].FootCS.Z <= HeightLimit;
        const bool bStableEnough = PlanarSpeed <= MaxPlanarSpeed;
        const bool bNotMovingUpDownTooFast = VerticalSpeed <= MaxVerticalSpeed;

        float FootSpeedTollerance = FMath::GetMappedRangeValueClamped(UseFootSpeedTolleranceRange, MaxFootBoneSpeed, ((Samples[NextIndex].RootCS - Samples[PrevIndex].RootCS)/ TimeDelta).Size2D());

        const bool bFootSpeedEnough = FVector(FootSpeed.X, FootSpeed.Y, 0.0f).Size() <= FootSpeedTollerance;

        Speeds.Add(FootSpeedTollerance); // DEBUGOWQNIE ------------------------------------------------------------------------------------------

        ContactMask[Index] = (bLowEnough && bStableEnough && bNotMovingUpDownTooFast && bFootSpeedEnough) ? 1 : 0;
    }

    const int32 MinContactFrames = FMath::RoundToInt(MinContactDuration * SampleRate);
    const int32 MinGapFrames = FMath::RoundToInt(MinGapDuration * SampleRate);

    RemoveShortContacts(ContactMask, MinContactFrames);
    FillShortGaps(ContactMask, MinGapFrames);

    const int32 ContactPaddingFrames = FMath::RoundToInt(ContactPaddingTime * SampleRate);
    ApplyContactPadding(ContactMask, ContactPaddingFrames);

    const int32 BlendFrames = FMath::Max(0, FMath::RoundToInt(BlendTime * SampleRate));

    TArray<float> SmoothedValues;
    BuildSmoothedContactValuesFromSegments(
        ContactMask,
        BlendFrames,
        bBlendInward,
        SmoothedValues);

    TArray<FRichCurveKey> Keys;
    Keys.Reserve(NumSamples);

    for (int32 Index = 0; Index < NumSamples; ++Index)
    {
        const float Time = Samples[Index].Time;

        // Positive PhaseOffset przesuwa krzyw¹ w prawo, czyli póŸniej w czasie.
        const float SourceTime = Time - PhaseOffset;

        const float Value = SampleFloatArrayByTime(
            SmoothedValues,
            SourceTime,
            PlayLength,
            SampleRate,
            bWrapPhaseOffset);

        FRichCurveKey Key;
        Key.Time = Time;
        Key.Value = FMath::Clamp(Value, 0.0f, 1.0f);

        // Wa¿ne:
        // Nie u¿ywaæ Auto/Cubic dla gêsto próbkowanej, rêcznie wyg³adzonej krzywej.
        // Cubic mo¿e generowaæ niechciane falowanie.
        Key.InterpMode = RCIM_Linear;
        Key.TangentMode = RCTM_Auto;

        //if (Speeds.IsValidIndex(Index) == true) Key.Value = Speeds[Index];
        
        
        Keys.Add(Key);
    }

    IAnimationDataController& Controller = AnimationSequence->GetController();
    const FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);

    Controller.OpenBracket(LOCTEXT("GenerateFootContactCurve", "Generate foot contact curve"), true);

    // Usuñ star¹ wersjê, ¿eby SetCurveKeys nie zostawi³o starych danych.
    Controller.RemoveCurve(CurveId, true);
    Controller.AddCurve(CurveId, AACF_DefaultCurve, true);
    Controller.SetCurveKeys(CurveId, Keys, true);

    // Ustawienie koloru widocznego w Curve Editor / Animation Editor.
    Controller.SetCurveColor(CurveId, CurveColor, true);

    Controller.CloseBracket(true);
}

void UFootContactAnimModifier::RemoveShortContacts(TArray<uint8>& InOutMask, int32 MinFrames)
{
    if (MinFrames <= 1)
    {
        return;
    }

    int32 Index = 0;
    while (Index < InOutMask.Num())
    {
        if (InOutMask[Index] == 0)
        {
            ++Index;
            continue;
        }

        const int32 Start = Index;
        while (Index < InOutMask.Num() && InOutMask[Index] != 0)
        {
            ++Index;
        }

        const int32 End = Index - 1;
        const int32 Length = End - Start + 1;

        if (Length < MinFrames)
        {
            for (int32 I = Start; I <= End; ++I)
            {
                InOutMask[I] = 0;
            }
        }
    }
}

void UFootContactAnimModifier::FillShortGaps(TArray<uint8>& InOutMask, int32 MinFrames)
{
    if (MinFrames <= 1)
    {
        return;
    }

    int32 Index = 0;
    while (Index < InOutMask.Num())
    {
        if (InOutMask[Index] != 0)
        {
            ++Index;
            continue;
        }

        const int32 Start = Index;
        while (Index < InOutMask.Num() && InOutMask[Index] == 0)
        {
            ++Index;
        }

        const int32 End = Index - 1;
        const int32 Length = End - Start + 1;

        const bool bHasContactBefore = Start > 0 && InOutMask[Start - 1] != 0;
        const bool bHasContactAfter = Index < InOutMask.Num() && InOutMask[Index] != 0;

        if (bHasContactBefore && bHasContactAfter && Length < MinFrames)
        {
            for (int32 I = Start; I <= End; ++I)
            {
                InOutMask[I] = 1;
            }
        }
    }
}

float UFootContactAnimModifier::SmoothContactValue(const TArray<uint8>& Mask, int32 Index, int32 BlendFrames)
{
    if (!Mask.IsValidIndex(Index))
    {
        return 0.0f;
    }

    if (BlendFrames <= 0)
    {
        return Mask[Index] ? 1.0f : 0.0f;
    }

    const bool bCurrent = Mask[Index] != 0;

    int32 DistanceToChange = BlendFrames + 1;

    for (int32 Offset = 1; Offset <= BlendFrames; ++Offset)
    {
        const int32 Prev = Index - Offset;
        const int32 Next = Index + Offset;

        if (Mask.IsValidIndex(Prev) && (Mask[Prev] != 0) != bCurrent)
        {
            DistanceToChange = FMath::Min(DistanceToChange, Offset);
        }

        if (Mask.IsValidIndex(Next) && (Mask[Next] != 0) != bCurrent)
        {
            DistanceToChange = FMath::Min(DistanceToChange, Offset);
        }
    }

    if (DistanceToChange > BlendFrames)
    {
        return bCurrent ? 1.0f : 0.0f;
    }

    const float Alpha = static_cast<float>(DistanceToChange) / static_cast<float>(BlendFrames + 1);

    // SmoothStep daje podobny efekt do wartoœci z przyk³adu:
    // blisko 0/1 na koñcach, miêkko w œrodku.
    const float Smooth = Alpha * Alpha * (3.0f - 2.0f * Alpha);

    return bCurrent ? Smooth : 1.0f - Smooth;
}

float UFootContactAnimModifier::SmoothStep01(float Alpha)
{
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    return Alpha * Alpha * (3.0f - 2.0f * Alpha);
}

void UFootContactAnimModifier::BuildSmoothedContactValues(
    const TArray<uint8>& Mask,
    int32 BlendFrames,
    bool bStartBlendAfterTransition,
    TArray<float>& OutValues)
{
    const int32 Num = Mask.Num();
    OutValues.SetNumZeroed(Num);

    if (Num <= 0)
    {
        return;
    }

    // Najpierw czysta wersja 0/1.
    for (int32 Index = 0; Index < Num; ++Index)
    {
        OutValues[Index] = Mask[Index] ? 1.0f : 0.0f;
    }

    if (BlendFrames <= 0)
    {
        return;
    }

    for (int32 EdgeIndex = 1; EdgeIndex < Num; ++EdgeIndex)
    {
        const uint8 PreviousState = Mask[EdgeIndex - 1];
        const uint8 NextState = Mask[EdgeIndex];

        if (PreviousState == NextState)
        {
            continue;
        }

        const float FromValue = PreviousState ? 1.0f : 0.0f;
        const float ToValue = NextState ? 1.0f : 0.0f;

        int32 StartIndex;
        int32 EndIndex;

        if (bStartBlendAfterTransition)
        {
            // EdgeIndex jest pocz¹tkiem nowego stanu.
            // Wyg³adzenie zaczyna siê na wykrytym skoku i idzie do przodu.
            StartIndex = EdgeIndex;
            EndIndex = FMath::Min(EdgeIndex + BlendFrames, Num - 1);
        }
        else
        {
            // Wyg³adzenie koñczy siê na wykrytym skoku.
            // Czyli zaczyna siê przed przejœciem.
            StartIndex = FMath::Max(EdgeIndex - BlendFrames, 0);
            EndIndex = EdgeIndex;
        }

        const int32 Range = FMath::Max(EndIndex - StartIndex, 1);

        for (int32 Index = StartIndex; Index <= EndIndex; ++Index)
        {
            const float RawAlpha = static_cast<float>(Index - StartIndex) / static_cast<float>(Range);
            const float Alpha = SmoothStep01(RawAlpha);

            const float BlendedValue = FMath::Lerp(FromValue, ToValue, Alpha);

            OutValues[Index] = BlendedValue;
        }
    }
}

float UFootContactAnimModifier::SampleFloatArrayByTime(
    const TArray<float>& Values,
    float Time,
    float Duration,
    float InSampleRate,
    bool bWrap)
{
    if (Values.Num() == 0 || Duration <= 0.0f || InSampleRate <= 0.0f)
    {
        return 0.0f;
    }

    if (bWrap)
    {
        Time = FMath::Fmod(Time, Duration);

        if (Time < 0.0f)
        {
            Time += Duration;
        }
    }
    else
    {
        Time = FMath::Clamp(Time, 0.0f, Duration);
    }

    const float FloatIndex = Time * InSampleRate;
    const int32 IndexA = FMath::Clamp(FMath::FloorToInt(FloatIndex), 0, Values.Num() - 1);
    const int32 IndexB = FMath::Clamp(IndexA + 1, 0, Values.Num() - 1);

    const float Alpha = FMath::Clamp(FloatIndex - static_cast<float>(IndexA), 0.0f, 1.0f);

    return FMath::Lerp(Values[IndexA], Values[IndexB], Alpha);
}

bool UFootContactAnimModifier::ExtractRootMotion(const UAnimSequence* AnimationSequence, FName BoneName, float Time, FTransform& OutComponentSpaceTransform) const
{
    FAnimPoseEvaluationOptions Options;
    FAnimPose CurrentPose;
    UAnimPoseExtensions::GetAnimPoseAtTime(AnimationSequence, Time, Options, CurrentPose);

    if (CurrentPose.IsValid())
    {
        OutComponentSpaceTransform = UAnimPoseExtensions::GetBonePose(CurrentPose, BoneName, EAnimPoseSpaces::World);
        return true;
    }

    return false;
}


void UFootContactAnimModifier::ApplyContactPadding(TArray<uint8>& InOutMask, int32 PaddingFrames)
{
    const int32 Num = InOutMask.Num();

    if (Num <= 0 || PaddingFrames == 0)
    {
        return;
    }

    TArray<uint8> Result;
    Result.Init(0, Num);

    int32 Index = 0;

    while (Index < Num)
    {
        if (InOutMask[Index] == 0)
        {
            ++Index;
            continue;
        }

        const int32 OriginalStart = Index;

        while (Index < Num && InOutMask[Index] != 0)
        {
            ++Index;
        }

        const int32 OriginalEnd = Index - 1;

        const bool bTouchesAnimStart = OriginalStart == 0;
        const bool bTouchesAnimEnd = OriginalEnd == Num - 1;

        int32 NewStart = OriginalStart;
        int32 NewEnd = OriginalEnd;

        if (PaddingFrames > 0)
        {
            // Dodatnia wartoœæ = zwê¿enie kontaktu.
            //
            // Wa¿ne:
            // Je¿eli segment dotyka pocz¹tku animacji, nie przesuwamy jego lewej krawêdzi.
            // Je¿eli segment dotyka koñca animacji, nie przesuwamy jego prawej krawêdzi.
            if (!bTouchesAnimStart)
            {
                NewStart += PaddingFrames;
            }

            if (!bTouchesAnimEnd)
            {
                NewEnd -= PaddingFrames;
            }
        }
        else
        {
            // Ujemna wartoœæ = rozszerzenie kontaktu.
            const int32 ExpandFrames = FMath::Abs(PaddingFrames);

            //
            // Dla rozszerzania te¿ nie modyfikujemy krawêdzi przy pocz¹tku/koñcu animacji.
            // W praktyce clamp i tak by ograniczy³ zakres, ale jawny warunek jest czytelniejszy
            // i zapobiega póŸniejszym efektom ubocznym przy wrapowaniu.
            //
            if (!bTouchesAnimStart)
            {
                NewStart -= ExpandFrames;
            }

            if (!bTouchesAnimEnd)
            {
                NewEnd += ExpandFrames;
            }
        }

        NewStart = FMath::Clamp(NewStart, 0, Num - 1);
        NewEnd = FMath::Clamp(NewEnd, 0, Num - 1);

        if (NewStart <= NewEnd)
        {
            for (int32 I = NewStart; I <= NewEnd; ++I)
            {
                Result[I] = 1;
            }
        }
    }

    InOutMask = MoveTemp(Result);
}


void UFootContactAnimModifier::BuildSmoothedContactValuesFromSegments(
    const TArray<uint8>& Mask,
    int32 BlendFrames,
    bool bBlendInward,
    TArray<float>& OutValues)
{
    const int32 Num = Mask.Num();

    OutValues.SetNumZeroed(Num);

    if (Num <= 0)
    {
        return;
    }

    if (BlendFrames <= 0)
    {
        for (int32 I = 0; I < Num; ++I)
        {
            OutValues[I] = Mask[I] ? 1.0f : 0.0f;
        }

        return;
    }

    int32 Index = 0;

    while (Index < Num)
    {
        if (Mask[Index] == 0)
        {
            ++Index;
            continue;
        }

        const int32 ContactStart = Index;

        while (Index < Num && Mask[Index] != 0)
        {
            ++Index;
        }

        const int32 ContactEnd = Index - 1;
        const int32 ContactLength = ContactEnd - ContactStart + 1;

        if (ContactLength <= 0)
        {
            continue;
        }

        if (bBlendInward)
        {
            // Wyg³adzenie mieœci siê wewn¹trz kontaktu.
            // Segment 1 zostaje czêœciowo "zjedzony" na pocz¹tku i koñcu.

            const int32 EffectiveBlendFrames = FMath::Min(BlendFrames, FMath::Max(1, ContactLength / 2));

            const int32 FadeInStart = ContactStart;
            const int32 FadeInEnd = FMath::Min(ContactStart + EffectiveBlendFrames, ContactEnd);

            const int32 FadeOutStart = FMath::Max(ContactEnd - EffectiveBlendFrames, ContactStart);
            const int32 FadeOutEnd = ContactEnd;

            // Fade in: 0 -> 1 wewn¹trz kontaktu.
            for (int32 I = FadeInStart; I <= FadeInEnd; ++I)
            {
                const float Alpha = static_cast<float>(I - FadeInStart) /
                    static_cast<float>(FMath::Max(FadeInEnd - FadeInStart, 1));

                OutValues[I] = FMath::Max(OutValues[I], SmoothStep01(Alpha));
            }

            // Plateau 1 pomiêdzy fade in i fade out.
            const int32 PlateauStart = FadeInEnd + 1;
            const int32 PlateauEnd = FadeOutStart - 1;

            for (int32 I = PlateauStart; I <= PlateauEnd; ++I)
            {
                if (OutValues.IsValidIndex(I))
                {
                    OutValues[I] = 1.0f;
                }
            }

            // Fade out: 1 -> 0 wewn¹trz kontaktu.
            for (int32 I = FadeOutStart; I <= FadeOutEnd; ++I)
            {
                const float Alpha = static_cast<float>(I - FadeOutStart) /
                    static_cast<float>(FMath::Max(FadeOutEnd - FadeOutStart, 1));

                const float Value = 1.0f - SmoothStep01(Alpha);
                OutValues[I] = FMath::Max(OutValues[I], Value);
            }
        }
        else
        {
            // Wyg³adzenie idzie na zewn¹trz kontaktu.
            // Ca³y oryginalny kontakt pozostaje jako plateau 1.

            for (int32 I = ContactStart; I <= ContactEnd; ++I)
            {
                OutValues[I] = 1.0f;
            }

            // Fade in przed kontaktem: 0 -> 1.
            const int32 FadeInStart = FMath::Max(ContactStart - BlendFrames, 0);
            const int32 FadeInEnd = ContactStart;

            for (int32 I = FadeInStart; I <= FadeInEnd; ++I)
            {
                const float Alpha = static_cast<float>(I - FadeInStart) /
                    static_cast<float>(FMath::Max(FadeInEnd - FadeInStart, 1));

                OutValues[I] = FMath::Max(OutValues[I], SmoothStep01(Alpha));
            }

            // Fade out po kontakcie: 1 -> 0.
            const int32 FadeOutStart = ContactEnd;
            const int32 FadeOutEnd = FMath::Min(ContactEnd + BlendFrames, Num - 1);

            for (int32 I = FadeOutStart; I <= FadeOutEnd; ++I)
            {
                const float Alpha = static_cast<float>(I - FadeOutStart) /
                    static_cast<float>(FMath::Max(FadeOutEnd - FadeOutStart, 1));

                const float Value = 1.0f - SmoothStep01(Alpha);
                OutValues[I] = FMath::Max(OutValues[I], Value);
            }
        }
    }
}


#undef LOCTEXT_NAMESPACE

