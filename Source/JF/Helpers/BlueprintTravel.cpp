// Copyright ChezyName. All Rights Reserved.


#include "BlueprintTravel.h"
#include "Engine/World.h"

void UBlueprintTravel::SeamlessTravel(const UObject* WorldContextObject, const TSoftObjectPtr<UWorld> Level)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	const FString LevelName = FString(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	if (World == nullptr)
	{
		return;
	}
	
	if(World->IsNetMode(NM_DedicatedServer) || World->IsNetMode(NM_ListenServer))
	{
		World->ServerTravel(LevelName + "?listen",true,false);
	}
}

FString UBlueprintTravel::GetObjectPath(TSoftClassPtr<UObject> SoftClass)
{
	return SoftClass.ToString();
}
