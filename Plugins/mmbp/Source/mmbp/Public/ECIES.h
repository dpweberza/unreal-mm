// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#pragma warning(disable: 4668)
#pragma warning(disable: 4541)

#include "CoreMinimal.h"
#include "Misc/Base64.h"
#include <locale>
#include <codecvt>
#include "string.h"
#include "CryptoPP/include/cryptlib.h"
#include "CryptoPP/include/eccrypto.h"
#include "CryptoPP/include/oids.h"
#include "CryptoPP/include/osrng.h"
#include "CryptoPP/include/hex.h"
#include "CryptoPP/include/filters.h"
#include "ECIES.generated.h"

/**
 * 
 */
UCLASS()
class MMBP_API UECIES: public UObject
{
	GENERATED_BODY()

public:
	UECIES();
	FString Encrypt(FString PlainText);
	FString Decrypt(FString CipherText);

	FString GetPrivateKeyAsString();
	FString GetPublicKeyAsString();

private:
	CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey PrivateKey;
	CryptoPP::ECIES<CryptoPP::ECP>::PublicKey PublicKey;
	bool DecodeBase64Key(const FString& Base64Key, TArray<uint8>& OutKeyData);
};
