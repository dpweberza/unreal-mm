#pragma once

#include "MetamaskSession.h"

UMetamaskSession::UMetamaskSession(): Ecies(UECIES::GetInstance())
{
	ProtectedPublicKey = Ecies.GetPublicKeyAsHexString();
	UE_LOG(LogTemp, Log, TEXT("Public key is %s"), *ProtectedPublicKey);
}

UMetamaskSession::UMetamaskSession(UECIES& InEcies, FMetamaskSessionData InSessionData) :
	Ecies(InEcies),
	SessionData(InSessionData)
{
	ProtectedPublicKey = Ecies.GetPublicKeyAsHexString();
	UE_LOG(LogTemp, Log, TEXT("Public key is %s"), *ProtectedPublicKey);
}

UMetamaskSession::~UMetamaskSession()
{
}

FMetamaskMessage UMetamaskSession::PrepareMessage(TSharedPtr<FJsonObject> &Data, bool Encrypt, FString WalletPublicKey)
{
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Data.ToSharedRef(), JsonWriter);

	FMetamaskMessage Message;

	Message.Id = SessionData.ChannelId;
	if (Encrypt)
	{
		Message.Message = Ecies.EncryptWithKey(JsonString, WalletPublicKey);
	}
	else {
		Message.DataObject = Data;
	}

	return Message;
}

FString UMetamaskSession::PublicKey()
{
	return ProtectedPublicKey;
}

bool UMetamaskSession::DecryptMessage(FString Message, FString DecryptedJsonString)
{
	return true;
}

FString UMetamaskSession::EncryptMessage(FString Message, FString WalletPublicKey)
{
	return Ecies.EncryptWithKey(Message, WalletPublicKey);
}
