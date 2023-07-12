// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#pragma warning(disable: 4668)
#pragma warning(disable: 4541)

#include "CoreMinimal.h"
#include "Misc/Base64.h"
#include "CryptoPP/include/cryptlib.h"
#include "CryptoPP/include/integer.h"
#include "CryptoPP/include/eccrypto.h"
#include "CryptoPP/include/oids.h"
#include "CryptoPP/include/osrng.h"
#include "CryptoPP/include/hex.h"
#include "CryptoPP/include/base64.h"
#include "CryptoPP/include/aes.h"
#include "CryptoPP/include/gcm.h"
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

	FString Decrypt(const FString& CipherText);
	FString EncryptWithKey(const FString& Plaintext, const FString& WalletPublicKey);

	FString GetPublicKey();

	void SetPrivateKey(const FString& HexPrivateKey);

private:
	UECIES();
	~UECIES();
	CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey PrivateKey;
	CryptoPP::ECIES<CryptoPP::ECP>::PublicKey PublicKey;

	FString ProtectedPublicKey;

	const int32 UNCOMPRESSED_PUBLIC_KEY_SIZE;
	const int32 AES_IV_LENGTH;
	const int32 AES_TAG_LENGTH;
	const int32 SECRET_KEY_LENGTH;

	CryptoPP::ECIES<CryptoPP::ECP>::PublicKey ParsePublicKey(std::string PublicKeyString);
	CryptoPP::SecByteBlock Decapsulate(CryptoPP::ECIES<CryptoPP::ECP>::PublicKey ParsedPublicKey);
	CryptoPP::SecByteBlock Encapsulate(CryptoPP::ECIES<CryptoPP::ECP>::PublicKey InPublicKey);

	CryptoPP::SecByteBlock KDF(std::string Input);
	CryptoPP::SecByteBlock ZeroPad(CryptoPP::SecByteBlock InBlock, size_t DesiredLength);

	FString SecByteBlockToString(const CryptoPP::SecByteBlock& Block);
};
