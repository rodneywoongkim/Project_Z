

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "InteractionWidgetCondition.h"
#include "InteractiveActorsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractiveActorsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELPFULFUNCTIONS_API IInteractiveActorsInterface
{
	GENERATED_BODY()

public:

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_InteractionTag(FGameplayTagContainer& ReturnTag);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_CurrentVelocity(FVector& ReturnVelocity);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Widget")
	void BPI_IA_Get_OverridedWidget(TSoftClassPtr<UUserWidget>& ReturnSoftClass);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Widget")
	void BPI_IA_Get_WidgetParams(
		ACharacter* PlayerChar, 
		FName& Text01, 
		FName& Text02, 
		float& Float01, 
		FLinearColor& Color01, 
		FLinearColor& Color02, 
		UObject*& Object01, 
		UObject*& Object02,
		FString& Text03,
		FString& Text04
	);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_InteractionType(int& InteractionType);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_RequiredAbilityOnOverlap(bool& Require) const;

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Widget")
	void BPI_IA_Get_WidgetWorldPosition(FVector& ReturnPosition);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (Most of important function! SHOULD be overrided in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Conditions", meta = (AdvancedDisplay = 2))
	void BPI_IA_Get_CheckDurningAbilityRun
	(
		bool& CheckBasicStates,
		bool& UseCorrectAngle,
		FVector2D& AngleArc,
		FVector2D& MaxPositionZ,
		float& MaxDistance,
		bool& CheckWallHit,
		bool& UseOtherTrace,
		AActor*& ToIgnores
	);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ 
	Blueprint Interface - Interactive Actor (Most of important function! SHOULD be overrided in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Conditions", meta = (AdvancedDisplay = 2))
	void BPI_IA_Get_CheckDurningOverlap
	(
		bool& CheckBasicStates,
		FVector2D& MaxPositionZ,
		float& MaxDistance,
		bool& CheckWallHit,
		bool& UseOtherTrace
	);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Widget")
	void BPI_IA_Get_CreatedWidgetInstance(UUserWidget*& WidgetInstance) const;

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_ConfigHoldingOption(bool& ActorCanBeHold, CALS_OverlayState& OverlayMatch);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Conditions")
	void BPI_IA_Get_ObjectCollisionOverlap(bool& IsOverlaping, UPARAM(ref) TArray<AActor*>& ToIgnore);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get|Conditions", meta = (DisplayName = "BPI IA Get ActorStartedInteraction"))
	void BPI_AI_Get_ActorStartedInteraction(bool& Started);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get", meta = (DisplayName = "BPI IA Get ObjectTracingOrigin"))
	void BPI_AI_Get_ObjectTracingOrigin(FVector& PositionWS);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set")
	void BPI_IA_Set_CreatedWidgetInstance(UUserWidget* WidgetInstance);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (AdvancedDisplay = 2, DisplayName = "BPI IA Set HitByBullet"))
	void BPI_AI_Set_HitByBullet(FHitResult HitInfo, ACharacter* FromCharacter, AActor* FromActor);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (DisplayName = "BPI IA Set StartInteraction"))
	void BPI_AI_Set_StartInteraction(bool Start);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (DisplayName = "BPI IA Set StartInteractionTypeB"))
	void BPI_AI_Set_StartInteractionTypeB(bool Start);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (DisplayName = "BPI IA Set StartPlayerCollisionBlock"))
	void BPI_AI_Set_StartPlayerCollisionBlock(bool BlockCollision, float TimeToBlock);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (DisplayName = "BPI IA Set ActivateFunctionality"))
	void BPI_AI_Set_ActivateFunctionality(bool Activate, ACharacter* Target);


	virtual void BPI_IA_Get_CanDisplayWidget(bool& CanDisplay) const;
	virtual void BPI_IA_Get_DestroyWhenAbilityRun(bool& Destroy) const;
	virtual TSubclassOf<UInteractionWidgetCondition> BPI_IA_Get_AddtiveConditionClass() const;

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set")
	void BPI_IA_Set_CanDisplayWidget(bool CanDisplay);


	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱
	Blueprint Interface - Interactive Actor (This function NOT require override in child class)
	⚠︎ Declarated for AGLS v2.0*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_TransformsProperties(FTransform& ReturnTransformA, FTransform& ReturnTransformB, FTransform& ReturnTransformC);

	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱
	⚠︎ Declarated for AGLS v2.0*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Get")
	void BPI_IA_Get_FloatPropertyValue(FName PropertyName, float DefValue, float& PropertyValue);


	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ - This declaration first appear for AGLS v2.0*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set")
	void BPI_IA_Set_UpdateInstanceAttachState(int AttachStateIndex, AActor* Owner);


	/*⇲ 𝙄𝙉𝙏𝙀𝙍𝘼𝘾𝙏𝙄𝙑𝙀 ↹ 𝘈𝘊𝘛𝘖𝘙𝘚 ↹ 𝐈𝐍𝐓𝐄𝐑𝐅𝐀𝐂𝐄² ⇱ - This declaration first appear for AGLS v2.0
	This function was created primarily to initialize instances of the 'BP_PickablePropItem' class. 
	Using it allows you to avoid creating a hard reference.*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactive Actors Interface|Set", meta = (AdvancedDisplay = 1))
	void BPI_IA_Set_CustomSpawnConfiguration(bool& Updated, AActor* Owner, uint8 StateIndex01, uint8 StateIndex02, int IntParam01, FName StringParam01, bool BoolParam01);

};
