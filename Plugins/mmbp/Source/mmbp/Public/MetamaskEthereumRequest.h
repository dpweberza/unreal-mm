#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskParameters.h"
#include "MetamaskEthereumRequest.generated.h"

USTRUCT(BlueprintType)
struct FMetamaskEthereumRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JsonRpc = "2.0";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Method;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FMetamaskParameters Params; // Use the appropriate Unreal Engine type for the 'params' property

	FMetamaskEthereumRequest() = default;

	FMetamaskEthereumRequest(const FString& InId, const FString& InMethod, FMetamaskParameters InParameters)
		: Id(InId)
		, Method(InMethod)
		, Params(InParameters)
	{
	}

    FMetamaskEthereumRequest(const FString& InMethod, FMetamaskParameters InParameters)
        : Method(InMethod)
        , Params(InParameters)
    {
}

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("id", Id);
        JsonObject->SetStringField("method", Method);
        JsonObject->SetObjectField("params", Params.ToJsonObject());
        return JsonObject;
    }

    bool FromJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
    {
        if (JsonObject.IsValid())
        {
            const TSharedPtr<FJsonObject> *Properties;
            JsonObject->TryGetStringField("id", Id);
            JsonObject->TryGetStringField("method", Method);
            JsonObject->TryGetObjectField("params", Properties);

            Params.FromJsonObject(Properties);

            return true;
        }
        return false;
    }
};
