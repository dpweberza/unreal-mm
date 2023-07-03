#pragma once

#include "CoreMinimal.h"
#include "MetamaskSession.generated.h"

/**
 *
 */
UCLASS()
class MMBP_API UMetamaskSession : public UObject
{
	GENERATED_BODY()

public:
	UMetamaskSession();
	~UMetamaskSession();

	void PrepareMessage(TMap<FString, FString> Data, bool Encrypt, FString WalletPublicKey);
};
