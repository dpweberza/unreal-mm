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
#include "CryptoPP/include/base64.h"
#include "CryptoPP/include/filters.h"

/**
 * 
 */
class MMBP_API UECIES
{

public:
	static UECIES& GetInstance()
	{
		static UECIES Instance;
		return Instance;
	}

	UECIES(const UECIES&) = delete;
	UECIES& operator=(const UECIES&) = delete;
	FString Encrypt(FString PlainText);
	FString Decrypt(FString CipherText);

	FString EncryptWithKey(const FString& Plaintext, const FString& Base64PublicKey);

	FString GetPublicKeyAsString();

private:
	UECIES();
	~UECIES();
	CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey PrivateKey;
	CryptoPP::ECIES<CryptoPP::ECP>::PublicKey PublicKey;
	bool DecodeBase64Key(const FString& Base64Key, TArray<uint8>& OutKeyData);
};
