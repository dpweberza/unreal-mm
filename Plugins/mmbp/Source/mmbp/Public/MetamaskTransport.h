#pragma once

#include "CoreMinimal.h"
#include "MetamaskTransport.generated.h"

/**
 *
 */
UCLASS()
class MMBP_API UMetamaskTransport : public UObject
{
	GENERATED_BODY()

public:
	UMetamaskTransport();
	~UMetamaskTransport();

	void OnSuccess();
	void OnFailure(FString Error);
	bool Connect(FString ConnectionUrl);
	void OnConnectRequest(FString ConnectionUrl);

	FString UserAgent;
};
