// Fill out your copyright notice in the Description page of Project Settings.


#include "ECIES.h"

UECIES::UECIES()
{
    // Instantiate a random number generator
    CryptoPP::AutoSeededRandomPool rng;

    // Instantiate an ECIES private key
    CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privateKey;

    // Generate a new private key
    privateKey.Initialize(rng, CryptoPP::ASN1::secp256r1());

    // Instantiate an ECIES public key
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey publicKey;

    // Generate the corresponding public key from the private key
    privateKey.MakePublicKey(publicKey);

    // Set the keys in the member variables
    this->PrivateKey = privateKey;
    this->PublicKey = publicKey;
}

FString UECIES::Encrypt(FString Plaintext)
{
    // Convert the FString to a std::string
    std::string plaintext = TCHAR_TO_UTF8(*Plaintext);

    std::string ciphertext;
    CryptoPP::AutoSeededRandomPool rng;

    try
    {
        CryptoPP::ECIES<CryptoPP::ECP>::Encryptor encryptor(this->PublicKey);

        CryptoPP::StringSource ss(plaintext, true,
            new CryptoPP::PK_EncryptorFilter(rng, encryptor,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(ciphertext)
                )
            )
        );
    }
    catch (const CryptoPP::Exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Encryption failed: %s"), *FString(e.what()));
        return "";
    }

    FString EncodedCiphertext = FString(UTF8_TO_TCHAR(ciphertext.c_str()));

    FString Recovered = Decrypt(EncodedCiphertext);

    return EncodedCiphertext;
}

FString UECIES::Decrypt(FString Ciphertext)
{
    std::string ciphertext = std::string(TCHAR_TO_UTF8(*Ciphertext));

    std::string plaintext;
    CryptoPP::AutoSeededRandomPool rng;

    try
    {
        CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(this->PrivateKey);

        new CryptoPP::StringSource(ciphertext, true,
            new CryptoPP::Base64Decoder(
                new CryptoPP::PK_DecryptorFilter(rng, decryptor,
                    new CryptoPP::StringSink(plaintext)
                )
            )
        );
    }
    catch (const CryptoPP::Exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Decryption failed: %s"), *FString(e.what()));
        return "";
    }

    return FString(UTF8_TO_TCHAR(plaintext.c_str()));
}

FString UECIES::GetPublicKeyAsString()
{
    std::string publicKeyStr;
    CryptoPP::StringSink publicKeySink(publicKeyStr);
    this->PublicKey.Save(publicKeySink);

    std::string output;

    // Convert the public key binary to hexadecimal string
    CryptoPP::StringSource(
        publicKeyStr, true,
        new CryptoPP::Base64Encoder(new CryptoPP::StringSink(output))
    );

    FString hexFormat = FString(UTF8_TO_TCHAR(output.c_str()));
    return hexFormat;
}

bool UECIES::DecodeBase64Key(const FString& Base64Key, TArray<uint8>& OutKeyData)
{
    FString DecodedString;
    if (!FBase64::Decode(Base64Key, DecodedString))
    {
        // Failed to decode the Base64 string
        return false;
    }

    OutKeyData.Empty();
    OutKeyData.Append((uint8*)DecodedString.GetCharArray().GetData(), DecodedString.Len());

    return true;
}

FString UECIES::EncryptWithKey(const FString& Plaintext, const FString& Base64PublicKey)
{
    // Convert the base64 public key to std::string
    std::string base64PublicKey = std::string(TCHAR_TO_UTF8(*Base64PublicKey));

    std::string decodedString;
    CryptoPP::StringSource ss(base64PublicKey, true,
        new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decodedString))
    );

    // Load the public key from std::string
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey publicKey;
    publicKey.Load(
        CryptoPP::StringSource(decodedString, true).Ref()
    );

    // Convert the FString to a std::string
    std::string plaintext = std::string(TCHAR_TO_UTF8(*Plaintext));

    std::string ciphertext;
    CryptoPP::AutoSeededRandomPool rng;

    try
    {
        CryptoPP::ECIES<CryptoPP::ECP>::Encryptor encryptor(publicKey);

        CryptoPP::StringSource(plaintext, true,
            new CryptoPP::PK_EncryptorFilter(rng, encryptor,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(ciphertext)
                )
            )
        );
    }
    catch (const CryptoPP::Exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Encryption failed: %s"), *FString(e.what()));
        return "";
    }

    FString EncodedCiphertext = FString(UTF8_TO_TCHAR(ciphertext.c_str()));

    FString Recovered = Decrypt(EncodedCiphertext);

    return EncodedCiphertext;
}