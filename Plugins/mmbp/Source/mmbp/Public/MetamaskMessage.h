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

    TSharedPtr<FJsonObject> DataObject;

    FMetamaskMessage() = default;
    FMetamaskMessage(FString InId, FString InMessage) :
        Id(InId),
        Message(InMessage)
    {}

    TSharedPtr<FJsonObject> ToJsonObject() const
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("id", Id);
        if (!Message.IsEmpty()) {
            JsonObject->SetStringField("message", Message);
        }
        else {
            JsonObject->SetObjectField("message", DataObject);
        }
        return JsonObject;
    }

    bool FromJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
    {
        if (JsonObject.IsValid())
        {
            JsonObject->TryGetStringField("id", Id);
            JsonObject->TryGetStringField("message", Message);
            if (Message.IsEmpty())
            {
                const TSharedPtr<FJsonObject> *DataObjectPtr;
                JsonObject->TryGetObjectField("message", DataObjectPtr);
                if (DataObjectPtr->IsValid())
                {
                    DataObject = DataObjectPtr->ToSharedRef();
                }
            }
            return true;
        }
        return false;
    }

    FString ToJsonString()
    {
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField("id", Id);
        if (!Message.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("Message not empty"));
            JsonObject->SetStringField("message", Message);
        }
        else {
            UE_LOG(LogTemp, Log, TEXT("Message is empty"));
            JsonObject->SetObjectField("message", DataObject);
        }

        FString JsonString;
        TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

        JsonString.ReplaceInline(TEXT("\r\n"), TEXT(""));
        JsonString.ReplaceInline(TEXT("\n"), TEXT(""));
        JsonString.ReplaceInline(TEXT("\t"), TEXT(""));
        JsonString.ReplaceInline(TEXT(" "), TEXT(""));

        return JsonString;
    }
};
