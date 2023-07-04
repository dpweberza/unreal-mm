#pragma once

#include "CoreMinimal.h"
#include "MetamaskSocketWrapper.generated.h"

/**
 *
 */
DECLARE_DELEGATE(FDelegateSocketConnected);
DECLARE_DELEGATE(FDelegateSocketDisconnected);

UCLASS()
class MMBP_API UMetamaskSocketWrapper : public UObject
{
	GENERATED_BODY()

public:
	UMetamaskSocketWrapper();
	~UMetamaskSocketWrapper();

	FDelegateSocketConnected DSocketConnected;
	FDelegateSocketDisconnected DSocketDisconnected;

	void Emit(FString EventName, FString Message);
	void On(FString EventName, const TFunction<void(FString)>& Callback);

private:

};