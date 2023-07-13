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

    UPROPERTY(EditAnywhere, BlueprintReadwrite)
        TArray<FMetamaskParameters> Params;

	FMetamaskEthereumRequest() = default;

    FMetamaskEthereumRequest(const FString& InId, const FString& InMethod, TArray<FMetamaskParameters> InParams)
        : Id(InId)
        , Method(InMethod)
    {
        Params.SetNum(InParams.Num());
        for (int32 Index = 0; Index < InParams.Num(); ++Index)
        {
            Params[Index] = InParams[Index];
        }
    }

    FMetamaskEthereumRequest(const FString& InMethod, TArray<FMetamaskParameters> InParams)
        : Method(InMethod)
    {
        Params.SetNum(InParams.Num());
        for (int32 Index = 0; Index < InParams.Num(); ++Index)
        {
            Params[Index] = InParams[Index];
        }
    }

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("id", Id);
        JsonObject->SetStringField("method", Method);
        JsonObject->SetStringField("jsonrpc", JsonRpc);

        TArray<TSharedPtr<FJsonValue, ESPMode::NotThreadSafe>, FDefaultAllocator> JsonArray;
        for (const FMetamaskParameters& InParams : Params)
        {
            TSharedPtr<FJsonValueObject> JsonValueObject = MakeShared<FJsonValueObject>(InParams.ToJsonObject());
            JsonArray.Add(JsonValueObject);
        }
        JsonObject->SetArrayField("params", JsonArray);
        
        return JsonObject;
    }

    bool FromJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
    {
        if (JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue, ESPMode::NotThreadSafe>, FDefaultAllocator> *JsonArray;
            JsonObject->TryGetStringField("id", Id);
            JsonObject->TryGetStringField("method", Method);
            JsonObject->TryGetArrayField("params", JsonArray);

            for (const TSharedPtr<FJsonValue, ESPMode::NotThreadSafe>& JsonValue : *JsonArray)
            {
                if (JsonValue->Type == EJson::Object)
                {
                    const TSharedPtr<FJsonObject>& JsonObject = JsonValue->AsObject();

                    // Create an FMetamaskParameters object
                    FMetamaskParameters Parameters;
                    Parameters.FromJsonObject(&JsonObject);
                    // Add the FMetamaskParameters object to the Params array
                    Params.Add(Parameters);
                }
            }

            return true;
        }
        return false;
    }
};
