#pragma once

#include "CoreMinimal.h"
#include "MetamaskSessionData.h"
#include "MetamaskMessage.h"
#include "Json.h"
#include "ECIES.h"

/**
 *
 */
class MMBP_API UMetamaskSession
{

public:
	UMetamaskSession();
	UMetamaskSession(UECIES& Ecies, FMetamaskSessionData SessionData);
	~UMetamaskSession();

	FMetamaskMessage PrepareMessage(TSharedPtr<FJsonObject> &Data, bool Encrypt, FString WalletPublicKey);

	FMetamaskSessionData SessionData;

	FString PublicKey();

	FString DecryptMessage(FString Message);
	FString EncryptMessage(FString Message, FString WalletPublicKey);

protected:
	FString ProtectedPublicKey;
	UECIES& Ecies;
};