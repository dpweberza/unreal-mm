#pragma once

#include "CoreMinimal.h"
#include "MetamaskEthereumRequest.h"

/**
 *
 */
class MMBP_API UMetamaskTransport
{

public:
	UMetamaskTransport();
	~UMetamaskTransport();

	void OnSuccess();
	void OnFailure(FString Error);
	bool Connect(FString ConnectionUrl);
	void OnConnectRequest(FString ConnectionUrl);
	void OnRequest(FString Id, FMetamaskEthereumRequest Request);

	FString UserAgent;
};
