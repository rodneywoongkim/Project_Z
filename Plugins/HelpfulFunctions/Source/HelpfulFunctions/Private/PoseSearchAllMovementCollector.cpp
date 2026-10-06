// Fill out your copyright notice in the Description page of Project Settings.


#include "PoseSearchAllMovementCollector.h"

UPoseSearchDatabasesCollector* UPoseSearchAllMovementCollector::PoseSearchFindCollectorForDatabase(
	UPoseSearchAllMovementCollector* MovementCollector,
	const TSoftObjectPtr<UPoseSearchDatabase>& InDatabase,
	bool ShouldSkipAlwaysLoaded,
	ESearchCase::Type SearchType,
	ESearchDir::Type SearchDir
)
{
	if (!MovementCollector) return nullptr;

	const FString DatabaseName = InDatabase.GetAssetName();

	TArray<UPoseSearchDatabasesCollector*> CollectorsToSearch;

	if (!ShouldSkipAlwaysLoaded) CollectorsToSearch = MovementCollector->AlwaysLoadedCollectors;
	if (MovementCollector->NotAlwaysLoadedCollectors.Num() > 0)
	{
		for (int s = 0; s < MovementCollector->NotAlwaysLoadedCollectors.Num(); s++)
		{
			UPoseSearchDatabasesCollector* AsLoadedCollector = MovementCollector->NotAlwaysLoadedCollectors[s].Get();
			if (AsLoadedCollector) { CollectorsToSearch.Add(AsLoadedCollector); }
		}
	}

	if (CollectorsToSearch.Num() > 0)
	{
		for (int i = 0; i < CollectorsToSearch.Num(); i++)
		{
			UPoseSearchDatabasesCollector* CurrentCollector = CollectorsToSearch[i];

			for (int ii = 0; ii < CurrentCollector->IdentifyingNamesPartsOfObjects.Num(); ii++)
			{
				const int32 FindResult = DatabaseName.Find(CurrentCollector->IdentifyingNamesPartsOfObjects[ii], SearchType, SearchDir, -1);
				if (FindResult >= 0)
				{
					return CurrentCollector;
				}
			}
		}
	}
	return nullptr;
}
