#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskParameters.generated.h"

USTRUCT(BlueprintType)
struct FMetamaskParameters
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TMap<FString, FString> Properties;

    FMetamaskParameters() = default;
    FMetamaskParameters(TMap<FString, FString> InProperties) : Properties(InProperties)
    {}

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

        for (const auto& Property : Properties)
        {
            JsonObject->SetStringField(Property.Key, Property.Value);
        }

        return JsonObject;
    }

    void FromJsonObject(const TSharedPtr<FJsonObject> *JsonObject)
    {
        if (JsonObject->IsValid())
        {
            for (const auto& Field : JsonObject->Get()->Values)
            {
                FString Key = Field.Key;
                FString Value = Field.Value->AsString();
                Properties.Add(Key, Value);
            }
        }
    }
};
