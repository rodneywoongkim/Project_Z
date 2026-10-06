// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PoseSearchDatabasesCollector.h"
#include "PoseSearchAllMovementCollector.generated.h"

/*
A DataAsset intended to store the complete movement setup for a character using PoseSearch and Soft References. By default, 
it allows for easier identification of which locomotion systems and assets are being used by a given Character.

This solution is still highly EXPERIMENTAL.*/
UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UPoseSearchAllMovementCollector : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All Movements Collector Settings")
	FName CollectorTagName = "none";

	//This allows you to force all collectors to load into memory at once. Typically, this solution is called at game start - BeginPlayEvent
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All Movements Collector Settings")
	bool ShouldLoadAllCollectorsOnStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All Movements Collector Settings")
	TArray<UPoseSearchDatabasesCollector*> AlwaysLoadedCollectors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All Movements Collector Settings")
	TArray<TSoftObjectPtr<UPoseSearchDatabasesCollector>> NotAlwaysLoadedCollectors;

	/*
	Here you can specify a list of PoseSearchDatabase assets that should always be available. Adding them as Hard References causes the 
	engine to keep these assets permanently loaded in memory, although it helps avoid loading too many databases at once from a single 
	collector.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All Movements Collector Settings")
	TArray<UPoseSearchDatabase*> AlwaysDatabasesInMemory;


	UFUNCTION(BlueprintCallable, Category = "Animation|Pose Search", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Pose Search Find Collector For Database", Keywords = "Pose,Search,Animation,Collector", AdvancedDisplay = 3))
	static UPoseSearchDatabasesCollector* PoseSearchFindCollectorForDatabase(
		UPoseSearchAllMovementCollector* MovementCollector, 
		const TSoftObjectPtr<UPoseSearchDatabase>& InDatabase, 
		bool ShouldSkipAlwaysLoaded = false, 
		ESearchCase::Type SearchType = ESearchCase::IgnoreCase,
		ESearchDir::Type SearchDir = ESearchDir::FromStart
	);

};
