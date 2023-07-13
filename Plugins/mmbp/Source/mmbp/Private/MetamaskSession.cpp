#pragma once

#include "MetamaskSession.h"

UMetamaskSession::UMetamaskSession(): Ecies(UECIES::GetInstance())
{
	ProtectedPublicKey = Ecies.GetPublicKey();
	UE_LOG(LogTemp, Log, TEXT("Public key is %s"), *ProtectedPublicKey);

	// Test
	//Ecies.SetPrivateKey("cbb47872e2d0cbfce30ccadda165303bc52d02378ab30c87cd5777bcfc2a36ca");
	//FString encryptedText = Ecies.EncryptWithKey("helloworld", "04153c0be4a73c4c5ef24970f0b7a6b4ac6e11bd790a5bc80e83e084d566a08f7dad13122402c2a17537e29f8bfa9c844d89c8ae980a90e202df254891058a7efa");
	//FString encryptedText = "BDLNeGTU2fek1u8ld1pomX+yK2XfwEkZ/GySXg3OXZ3uf26cWd9BL911SzKOBUWkLYLZ7wnKRzXZkt5xvVULbhYQTyruLyUqA4uyZsy274Uvunc4NoNec7PGBaRueczHvL+tdp6OQJPZk+Y=";

	//UE_LOG(LogTemp, Log, TEXT("encryptedText: %s"), *encryptedText);

	//FString decryptedText = Ecies.Decrypt(encryptedText);

	//UE_LOG(LogTemp, Log, TEXT("decrypted: %s"), *decryptedText);
}

UMetamaskSession::UMetamaskSession(UECIES& InEcies, FMetamaskSessionData InSessionData) :
	Ecies(InEcies),
	SessionData(InSessionData)
{
	ProtectedPublicKey = Ecies.GetPublicKey();
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
		UE_LOG(LogTemp, Log, TEXT("Checking we have wallet public key: %s"), *WalletPublicKey);
		UE_LOG(LogTemp, Log, TEXT("Message is %s"), *JsonString);
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

FString UMetamaskSession::DecryptMessage(FString Message)
{
	return Ecies.Decrypt(Message);
}

FString UMetamaskSession::EncryptMessage(FString Message, FString WalletPublicKey)
{
	return Ecies.EncryptWithKey(Message, WalletPublicKey);
}
