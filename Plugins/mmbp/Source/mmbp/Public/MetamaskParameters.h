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

    FString ToJsonString() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

        for (const auto& Property : Properties)
        {
            JsonObject->SetStringField(Property.Key, Property.Value);
        }

        FString JsonString;
        TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

        return JsonString;
    }
};
