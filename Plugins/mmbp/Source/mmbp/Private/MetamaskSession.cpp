#include "MetamaskSession.h"

UMetamaskSession::UMetamaskSession()
{
}

UMetamaskSession::~UMetamaskSession()
{
}

FString UMetamaskSession::PrepareMessage(TSharedPtr<FJsonObject> Data, bool Encrypt, FString WalletPublicKey)
{
	return FString();
}

FString UMetamaskSession::PublicKey()
{
	return ProtectedPublicKey;
}

FString UMetamaskSession::DecryptMessage(FString Message)
{
	return FString();
}
