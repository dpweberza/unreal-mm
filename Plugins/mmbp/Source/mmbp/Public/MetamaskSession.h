#pragma once

#include "CoreMinimal.h"
#include "MetamaskSessionData.h"
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

	FString PrepareMessage(TSharedPtr<FJsonObject> Data, bool Encrypt, FString WalletPublicKey);

	FMetamaskSessionData SessionData;

	FString PublicKey();

	bool DecryptMessage(FString Message, FString DecryptedJsonMessage);

protected:
	FString ProtectedPublicKey;
};