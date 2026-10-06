// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "PoseSearchDatabasesCollector.generated.h"

/*
A data asset designed to optimize memory usage related to PoseSearch databases. The UPoseSearchDatabasesCollector class stores 
PoseSearchDatabases as Soft References, which should most commonly be associated with a specific movement style, such as Walking, 
Running, or Crouching. Not all databases need to remain loaded in memory at all times. Instead of loading each asset individually, 
UPoseSearchDatabasesCollector allows all required databases that are not currently in memory to be loaded at once.

Keep in mind that this solution is only effective if the PoseSearchDatabases are not included in a Normalization asset that is 
already loaded in memory, or if they are not referenced somewhere through Hard References.

The entire Async Loading Databases system is highly EXPERIMENTAL.
*/
UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UPoseSearchDatabasesCollector : public UDataAsset
{
	GENERATED_BODY()
	
public:

	/*
	A list containing name fragments, usually from databases stored in the SoftRefPoseSearchDatabases variable. 
	It allows identifying which UPoseSearchDatabasesCollector an unloaded asset currently belongs to.
	
	For example, if a UPoseSearchDatabasesCollector contains three databases: 
	- PSD_Standing_Walk_Starts, 
	- PSD_Standing_Walk_Loops, 
	- PSD_Standing_Walk_Stops
	
	then the identifying fragment could be 'Walk' or 'Standing'.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Collector Settings")
	TArray<FString> IdentifyingNamesPartsOfObjects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Collector Settings")
	TArray<FName> IdentifyingTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Collector Settings")
	bool ShouldInterruptMotionMatchAfterLoad = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Collector Settings")
	TArray<TSoftObjectPtr<UPoseSearchDatabase>> SoftRefPoseSearchDatabases;


};
