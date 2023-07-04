#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskSessionData.generated.h"

USTRUCT(BlueprintType)
struct FMetamaskSessionData
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString AppName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString AppUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ChannelId;

	FMetamaskSessionData()
	{
		ChannelId = FGuid::NewGuid().ToString();
	}
};