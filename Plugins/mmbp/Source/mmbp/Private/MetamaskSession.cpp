#pragma once

#include "MetamaskSession.h"

UMetamaskSession::UMetamaskSession(): Ecies(UECIES::GetInstance())
{
}

UMetamaskSession::UMetamaskSession(UECIES& InEcies, FMetamaskSessionData InSessionData) :
	Ecies(InEcies),
	SessionData(InSessionData)
{
}

UMetamaskSession::~UMetamaskSession()
{
}

FMetamaskMessage UMetamaskSession::PrepareMessage(TSharedPtr<FJsonObject> &Data, bool Encrypt, FString WalletPublicKey)
{
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Data.ToSharedRef(), JsonWriter);

	FString ChannelId;
	FMetamaskMessage Message;
	if (Data->TryGetStringField(TEXT("ChannelId"), ChannelId))
	{
		Message.Id = ChannelId;
		if (Encrypt)
		{
			Message.Message = Ecies.EncryptWithKey(JsonString, WalletPublicKey);
		}
		else {
			Message.DataObject = Data;
		}
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
