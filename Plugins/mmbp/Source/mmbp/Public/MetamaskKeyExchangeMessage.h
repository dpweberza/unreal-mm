#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskKeyExchangeMessage.generated.h"

USTRUCT(BlueprintType)
struct FMetamaskKeyExchangeMessage
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString PubKey;

    FMetamaskKeyExchangeMessage() = default;
    FMetamaskKeyExchangeMessage(FString InType, FString InPubKey) :
        Type(InType),
        PubKey(InPubKey)
    {}

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("type", Type);
        JsonObject->SetStringField("pubkey", PubKey);
        return JsonObject;
    }

    bool FromJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
    {
        if (JsonObject.IsValid())
        {
            JsonObject->TryGetStringField("type", Type);
            JsonObject->TryGetStringField("pubkey", PubKey);
            return true;
        }
        return false;
    }
};
