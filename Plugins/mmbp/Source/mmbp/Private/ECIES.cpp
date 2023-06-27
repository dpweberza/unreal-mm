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
                new CryptoPP::HexEncoder(
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
    FString Base64Encoded = FBase64::Encode(EncodedCiphertext);

    FString Recovered = Decrypt(Base64Encoded);

    return Base64Encoded;
}

FString UECIES::Decrypt(FString Ciphertext)
{
    // Decode the Base64-encoded and hex-encoded ciphertext
    FString DecodedCiphertext;
    if (!FBase64::Decode(Ciphertext, DecodedCiphertext))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decode ciphertext."));
        return "";
    }

    DecodedCiphertext = DecodedCiphertext.TrimStartAndEnd();

    // Convert the TArray<uint8> to a std::string
    std::string ciphertext = std::string(TCHAR_TO_UTF8(*DecodedCiphertext));

    std::string plaintext;
    CryptoPP::AutoSeededRandomPool rng;

    try
    {
        CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(this->PrivateKey);

        new CryptoPP::StringSource(ciphertext, true,
            new CryptoPP::HexDecoder(
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
        new CryptoPP::HexEncoder(new CryptoPP::StringSink(output))
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

FString UECIES::EncryptWithKey(const FString& Plaintext, const FString& HexPublicKey)
{
    // Convert the hexadecimal public key to std::string
    std::string hexString = std::string(TCHAR_TO_UTF8(*HexPublicKey));

    std::string decodedString;
    CryptoPP::StringSource ss(hexString, true,
        new CryptoPP::HexDecoder(new CryptoPP::StringSink(decodedString))
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
                new CryptoPP::HexEncoder(
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
    FString Base64Encoded = FBase64::Encode(EncodedCiphertext);

    FString Recovered = Decrypt(Base64Encoded);

    return Base64Encoded;
}

FString UECIES::DecryptWithKey(const FString& Ciphertext, const FString& Base64PrivateKey)
{
    // Decode the Base64-encoded private key
    TArray<uint8> PrivateKeyData;
    if (!FBase64::Decode(Base64PrivateKey, PrivateKeyData))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decode private key."));
        return "";
    }

    // Convert the FString to a std::string
    std::string ciphertext = std::string(TCHAR_TO_UTF8(*Ciphertext));

    std::string plaintext;
    CryptoPP::AutoSeededRandomPool rng;

    try
    {
        CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privateKey;
        CryptoPP::ArraySource privateKeySource(
            PrivateKeyData.GetData(), PrivateKeyData.Num(), true
        );
        privateKey.Load(privateKeySource);

        CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(privateKey);

        new CryptoPP::StringSource(ciphertext, true,
            new CryptoPP::HexDecoder(
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

    // Convert the std::string to an FString
    FString Plaintext = FString(UTF8_TO_TCHAR(plaintext.c_str()));

    return Plaintext;
}