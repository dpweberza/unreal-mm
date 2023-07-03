#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskMessage.generated.h"

USTRUCT(BlueprintType)
struct FMetamaskMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    FMetamaskMessage() = default;
    FMetamaskMessage(FString InId, FString InMessage) :
        Id(InId),
        Message(InMessage)
    {}

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("id", Id);
        JsonObject->SetStringField("message", Message);
        return JsonObject;
    }

    bool FromJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
    {
        if (JsonObject.IsValid())
        {
            JsonObject->TryGetStringField("id", Id);
            JsonObject->TryGetStringField("message", Message);
            return true;
        }
        return false;
    }
};
