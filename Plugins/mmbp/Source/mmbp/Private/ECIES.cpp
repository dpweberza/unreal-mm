// Fill out your copyright notice in the Description page of Project Settings.


#include "ECIES.h"

UECIES::UECIES(): 
    UNCOMPRESSED_PUBLIC_KEY_SIZE(1 + 32 + 32),
    AES_IV_LENGTH(16),
    AES_TAG_LENGTH(16),
    SECRET_KEY_LENGTH(32)
{

    // Instantiate a random number generator
    CryptoPP::AutoSeededRandomPool prng;

    //CryptoPP::ECIES<CryptoPP::ECP, CryptoPP::SHA256, CryptoPP::NoCofactorMultiplication, true, false>::Decryptor d0(prng, CryptoPP::ASN1::secp256k1());
    //CryptoPP::ECIES<CryptoPP::ECP, CryptoPP::SHA256, CryptoPP::NoCofactorMultiplication, true, false>::Encryptor e0(d0);

    CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> GroupParameters;
    GroupParameters.SetPointCompression(true);
    GroupParameters.Initialize(CryptoPP::ASN1::secp256k1());

    CryptoPP::Integer x(prng, CryptoPP::Integer::One(), GroupParameters.GetMaxExponent());
    CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>::Element Element = GroupParameters.ExponentiateBase(x);

    // Instantiate an ECIES private key
    CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privateKey;

    // Generate a new private key
    privateKey.Initialize(prng, CryptoPP::ASN1::secp256k1());

    // Instantiate an ECIES public key
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey publicKey;

    // Generate the corresponding public key from the private key
    privateKey.MakePublicKey(publicKey);
    this->PublicKey.AccessGroupParameters().SetPointCompression(true);

    // Set the keys in the member variables
    this->PrivateKey = privateKey;
    this->PublicKey = publicKey;


}

UECIES::~UECIES()
{}

FString UECIES::GetPublicKey()
{
    CryptoPP::ECP::Point Point = PublicKey.GetPublicElement();
    CryptoPP::SecByteBlock sx(Point.x.ByteCount());

    PublicKey.GetPublicElement().x.Encode(sx, PublicKey.GetPublicElement().x.ByteCount());

    std::string output = std::string((const char*)sx.data(), sx.size());
    std::string result;

    CryptoPP::StringSource(output, true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(result), false
        )
    );

    CryptoPP::Integer Remainder = PublicKey.GetPublicElement().y.Modulo(2);
    FString HexPublicKey = FString(UTF8_TO_TCHAR(result.c_str()));
    if (Remainder > 0) {
        return "03" + HexPublicKey;
    }
    else {
        return "02" + HexPublicKey;
    }
}

FString UECIES::Decrypt(const FString& Base64Message)
{
    // Convert the FString to a std::string
    std::string base64Message = std::string(TCHAR_TO_UTF8(*Base64Message));
    std::string plaintext, ciphertext, message;
    CryptoPP::StringSource ss(base64Message, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(message)
        )
    );

    // Get the public key portion and generate a public key out of it
    std::string publicKeyString = message.substr(0, UNCOMPRESSED_PUBLIC_KEY_SIZE);

    //UE_LOG(LogTemp, Log, TEXT("Message size before b64: %d"), message.size());

    //// Print message in hex to double check if all elements are expected
    //std::string hexed;
    //CryptoPP::StringSource hexSS(message, true,
    //    new CryptoPP::HexEncoder(
    //        new CryptoPP::StringSink(hexed), false
    //    )
    //);
    //FString result = FString(UTF8_TO_TCHAR(hexed.c_str()));
    //UE_LOG(LogTemp, Log, TEXT("Message in hex: %s"), *result);


    //std::string b4 = publicKeyString.substr(0, 1);
    //CryptoPP::SecByteBlock b4s((const CryptoPP::byte*)b4.data(), b4.size());
    //SecByteBlockToString(b4s);
    //std::string x = publicKeyString.substr(1, SECRET_KEY_LENGTH);
    //CryptoPP::SecByteBlock xs((const CryptoPP::byte*)x.data(), x.size());
    //SecByteBlockToString(xs);
    //std::string y = publicKeyString.substr(1 + SECRET_KEY_LENGTH, SECRET_KEY_LENGTH);
    //CryptoPP::SecByteBlock ys((const CryptoPP::byte*)y.data(), y.size());
    //SecByteBlockToString(ys);

    //std::string pubKeyHex;
    //// Print message in hex to double check if all elements are expected
    //CryptoPP::StringSource pubcKeyHexSS(publicKeyString, true,
    //    new CryptoPP::HexEncoder(
    //        new CryptoPP::StringSink(pubKeyHex), false
    //    )
    //);
    //FString pubKey = FString(UTF8_TO_TCHAR(pubKeyHex.c_str()));
    //UE_LOG(LogTemp, Log, TEXT("Public key in decrypt: %s"), *pubKey);

    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey ParsedPublicKey = ParsePublicKey(publicKeyString);

    std::string iv = message.substr(UNCOMPRESSED_PUBLIC_KEY_SIZE, AES_IV_LENGTH);
    std::string tag = message.substr(UNCOMPRESSED_PUBLIC_KEY_SIZE + AES_IV_LENGTH, AES_TAG_LENGTH);
    ciphertext = message.substr(UNCOMPRESSED_PUBLIC_KEY_SIZE + AES_IV_LENGTH + AES_TAG_LENGTH);

    CryptoPP::SecByteBlock SharedSecret = Decapsulate(ParsedPublicKey);
    //UE_LOG(LogTemp, Log, TEXT("In Decrypt: SharedSecret"));
    //SecByteBlockToString(SharedSecret);

    CryptoPP::GCM<CryptoPP::AES>::Decryption d;
    d.SetKeyWithIV(SharedSecret.BytePtr(), SharedSecret.size(), (const CryptoPP::byte*)iv.data(), iv.size());

    CryptoPP::AuthenticatedDecryptionFilter df(d,
        new CryptoPP::StringSink(plaintext),
        CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
        AES_TAG_LENGTH
    );

    CryptoPP::StringSource stringSource(ciphertext + tag, true, new CryptoPP::Redirector(df));

    // Convert the std::string to an FString
    FString Plaintext = FString(UTF8_TO_TCHAR(plaintext.c_str()));

    //UE_LOG(LogTemp, Log, TEXT("Decrypted Message: %s"), *Plaintext);

    return Plaintext;
}

CryptoPP::ECIES<CryptoPP::ECP>::PublicKey UECIES::ParsePublicKey(std::string PublicKeyString)
{
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey PubKey;
    PubKey.AccessGroupParameters().Initialize(CryptoPP::ASN1::secp256k1());

    CryptoPP::ECP::Point point;

    PubKey.GetGroupParameters().GetCurve().DecodePoint(point, (const CryptoPP::byte*)PublicKeyString.data(), PublicKeyString.size());
    PubKey.SetPublicElement(point);

    //UE_LOG(LogTemp, Log, TEXT("ParsePublicKey"));
    //CryptoPP::SecByteBlock x(point.x.ByteCount());
    //CryptoPP::SecByteBlock y(point.y.ByteCount());
    //point.x.Encode(x, point.x.ByteCount());
    //point.y.Encode(y, point.y.ByteCount());
    //SecByteBlockToString(x);
    //SecByteBlockToString(y);

    return PubKey;
}

void UECIES::SetPrivateKey(const FString& HexPrivateKey)
{
    CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey NewPrivateKey;

    std::string hexPrivateKeyString = std::string(TCHAR_TO_UTF8(*HexPrivateKey));
    hexPrivateKeyString.insert(0, "0x");

    CryptoPP::Integer x(hexPrivateKeyString.c_str());
    NewPrivateKey.Initialize(CryptoPP::ASN1::secp256k1(), x);

    // Instantiate an ECIES public key
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey NewPublicKey;

    // Generate the corresponding public key from the private key
    NewPrivateKey.MakePublicKey(NewPublicKey);
    this->PublicKey.AccessGroupParameters().SetPointCompression(true);

    // Set the keys in the member variables
    this->PrivateKey = NewPrivateKey;
    this->PublicKey = NewPublicKey;
}

FString UECIES::EncryptWithKey(const FString& Plaintext, const FString& HexPublicKey)
{
    // Convert the hex public key to std::string
    std::string hexPublicKey = std::string(TCHAR_TO_UTF8(*HexPublicKey));
    std::string publicKey;

    CryptoPP::StringSource(hexPublicKey, true,
        new CryptoPP::HexDecoder(
            new CryptoPP::StringSink(publicKey)
        )
    );

    //UE_LOG(LogTemp, Log, TEXT("encrypt with key: %s"), *HexPublicKey);
    CryptoPP::ECIES<CryptoPP::ECP>::PublicKey ParsedPublicKey = ParsePublicKey(publicKey);

    // Convert the FString to a std::string
    std::string plaintext = std::string(TCHAR_TO_UTF8(*Plaintext));
    std::string ciphertext;

    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock IV(AES_IV_LENGTH);
    prng.GenerateBlock(IV.begin(), IV.size());
    
    // testing - IV
    //std::string hexIv = "9dc25c52abad2f03e958470628e50c33";
    //std::string ivByte;
    //CryptoPP::StringSource(hexIv, true,
    //    new CryptoPP::HexDecoder(
    //        new CryptoPP::StringSink(ivByte)
    //    )
    //);
    //CryptoPP::SecByteBlock IV((const CryptoPP::byte*)ivByte.data(), ivByte.size());
    // end testing - IV

    CryptoPP::SecByteBlock SharedSecret = Encapsulate(ParsedPublicKey);
    //UE_LOG(LogTemp, Log, TEXT("In Encrypt: SharedSecret"));
    //SecByteBlockToString(SharedSecret);

    try
    {
        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(SharedSecret.BytePtr(), SharedSecret.size(), IV, IV.size());

        CryptoPP::StringSource stringSource(plaintext, true,
            new CryptoPP::AuthenticatedEncryptionFilter(e,
               new CryptoPP::StringSink(ciphertext)
            )
        );
    }
    catch (const CryptoPP::Exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Encryption failed: %s"), *FString(e.what()));
        return "";
    }

    CryptoPP::SecByteBlock x(PublicKey.GetPublicElement().x.ByteCount());
    CryptoPP::SecByteBlock y(PublicKey.GetPublicElement().y.ByteCount());

    std::string b4 = "04";
    std::string b4d;
    CryptoPP::StringSource(b4, true,
        new CryptoPP::HexDecoder(
            new CryptoPP::StringSink(b4d)
        )
    );

    CryptoPP::SecByteBlock Byte04((const CryptoPP::byte*)b4d.data(), b4d.size());

    PublicKey.GetPublicElement().x.Encode(x, PublicKey.GetPublicElement().x.ByteCount());
    PublicKey.GetPublicElement().y.Encode(y, PublicKey.GetPublicElement().y.ByteCount());

    //UE_LOG(LogTemp, Log, TEXT("Encrypt possible pubkey"));
    //SecByteBlockToString(Byte04);
    //SecByteBlockToString(x);
    //SecByteBlockToString(y);

    //// Print message in hex to double check if all elements are expected
    //std::string hexedCipher;
    //CryptoPP::StringSource hexSScc(ciphertext, true,
    //    new CryptoPP::HexEncoder(
    //        new CryptoPP::StringSink(hexedCipher), false
    //    )
    //);
    //UE_LOG(LogTemp, Log, TEXT("ciphertext: %s"), *FString(UTF8_TO_TCHAR(hexedCipher.c_str())));
    //FString resultcc = FString(UTF8_TO_TCHAR(hexedCipher.c_str()));
    //UE_LOG(LogTemp, Log, TEXT("Message in hex: %s"), *resultcc);
    //UE_LOG(LogTemp, Log, TEXT("b4 size: %d"), Byte04.size());
    //UE_LOG(LogTemp, Log, TEXT("x size: %d"), x.size());
    //UE_LOG(LogTemp, Log, TEXT("y size: %d"), y.size());
    //UE_LOG(LogTemp, Log, TEXT("IV size: %d"), IV.size());

    std::string message = 
        std::string((const char*)Byte04.data(), Byte04.size()) + 
        std::string((const char*)x.data(), x.size()) +
        std::string((const char*)y.data(), y.size()) +
        std::string((const char*)IV.data(), IV.size()) + 
        ciphertext.substr(ciphertext.size() - AES_TAG_LENGTH) + 
        ciphertext.substr(0, ciphertext.size() - AES_TAG_LENGTH);

    std::string finalMessage;

    CryptoPP::StringSource stringSource(message, true,
        new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(finalMessage), false
        )
    );

    //// Print message in hex to double check if all elements are expected
    //std::string hexed;
    //CryptoPP::StringSource hexSS(message, true,
    //    new CryptoPP::HexEncoder(
    //        new CryptoPP::StringSink(hexed), false
    //    )
    //);
    //FString result = FString(UTF8_TO_TCHAR(hexed.c_str()));
    //UE_LOG(LogTemp, Log, TEXT("Message in hex: %s"), *result);

    FString EncodedCiphertext = FString(UTF8_TO_TCHAR(finalMessage.c_str()));
    //UE_LOG(LogTemp, Log, TEXT("Message size before b64: %d"), message.size());

    return EncodedCiphertext;
}

CryptoPP::SecByteBlock UECIES::Encapsulate(CryptoPP::ECIES<CryptoPP::ECP>::PublicKey InPublicKey)
{
    CryptoPP::Integer PrivExp = PrivateKey.GetPrivateExponent();
    CryptoPP::ECP::Point SharedSecretPoint = InPublicKey.GetGroupParameters().GetCurve().ScalarMultiply(InPublicKey.GetPublicElement(), PrivExp);

    std::string b4 = "04";
    std::string b4d;
    CryptoPP::StringSource ss(b4, true,
        new CryptoPP::HexDecoder(
            new CryptoPP::StringSink(b4d)
        )
    );

    CryptoPP::SecByteBlock sx(SharedSecretPoint.x.ByteCount());
    CryptoPP::SecByteBlock sy(SharedSecretPoint.y.ByteCount());
    CryptoPP::SecByteBlock Byte04((const CryptoPP::byte*)b4d.data(), b4d.size());
    CryptoPP::SecByteBlock rx(PublicKey.GetPublicElement().x.ByteCount());
    CryptoPP::SecByteBlock ry(PublicKey.GetPublicElement().y.ByteCount());

    CryptoPP::SecByteBlock KDFBlock(sx.size() + sy.size() + Byte04.size() * 2 + rx.size() + ry.size());

    SharedSecretPoint.x.Encode(sx, SharedSecretPoint.x.ByteCount());
    SharedSecretPoint.y.Encode(sy, SharedSecretPoint.y.ByteCount());
    PublicKey.GetPublicElement().x.Encode(rx, PublicKey.GetPublicElement().x.ByteCount());
    PublicKey.GetPublicElement().y.Encode(ry, PublicKey.GetPublicElement().y.ByteCount());

    //UE_LOG(LogTemp, Log, TEXT("Encapsulate"));
    //SecByteBlockToString(sx);
    //SecByteBlockToString(sy);
    //SecByteBlockToString(rx);
    //SecByteBlockToString(ry);

    int l = PublicKey.GetGroupParameters().GetCurve().GetField().GetModulus().ByteCount();
    //UE_LOG(LogTemp, Log, TEXT("modulus byte count: %d"), l);

    std::string finalMessage =
        std::string((const char*)Byte04.data(), Byte04.size()) +
        std::string((const char*)rx.data(), rx.size()) +
        std::string((const char*)ry.data(), ry.size()) +
        std::string((const char*)Byte04.data(), Byte04.size()) +
        std::string((const char*)sx.data(), sx.size()) +
        std::string((const char*)sy.data(), sy.size());


    CryptoPP::SecByteBlock SharedSecret = KDF(finalMessage);

    return SharedSecret;
}

CryptoPP::SecByteBlock UECIES::Decapsulate(CryptoPP::ECIES<CryptoPP::ECP>::PublicKey InPublicKey) 
{
    CryptoPP::Integer PrivExp = PrivateKey.GetPrivateExponent();
    CryptoPP::ECP::Point SharedSecretPoint = PrivateKey.GetGroupParameters().GetCurve().ScalarMultiply(InPublicKey.GetPublicElement(), PrivExp);
    

    std::string b4 = "04";
    std::string b4d;
    CryptoPP::StringSource ss(b4, true,
        new CryptoPP::HexDecoder(
            new CryptoPP::StringSink(b4d)
        )
    );

    CryptoPP::SecByteBlock sx(SharedSecretPoint.x.ByteCount());
    CryptoPP::SecByteBlock sy(SharedSecretPoint.y.ByteCount());
    CryptoPP::SecByteBlock Byte04((const CryptoPP::byte*)b4d.data(), b4d.size());
    CryptoPP::SecByteBlock rx(InPublicKey.GetPublicElement().x.ByteCount());
    CryptoPP::SecByteBlock ry(InPublicKey.GetPublicElement().y.ByteCount());
    CryptoPP::SecByteBlock KDFBlock(sx.size() + sy.size() + Byte04.size() * 2 + rx.size() + ry.size());

    SharedSecretPoint.x.Encode(sx, SharedSecretPoint.x.ByteCount());
    SharedSecretPoint.y.Encode(sy, SharedSecretPoint.y.ByteCount());
    InPublicKey.GetPublicElement().x.Encode(rx, InPublicKey.GetPublicElement().x.ByteCount());
    InPublicKey.GetPublicElement().y.Encode(ry, InPublicKey.GetPublicElement().y.ByteCount());

    //UE_LOG(LogTemp, Log, TEXT("Decapsulate"));
    //SecByteBlockToString(sx);
    //SecByteBlockToString(sy);
    //SecByteBlockToString(rx);
    //SecByteBlockToString(ry);

    int l = InPublicKey.GetGroupParameters().GetCurve().GetField().GetModulus().ByteCount();
    //UE_LOG(LogTemp, Log, TEXT("modulus byte count: %d"), l);

    std::string finalMessage =
        std::string((const char*)Byte04.data(), Byte04.size()) +
        std::string((const char*)rx.data(), rx.size()) +
        std::string((const char*)ry.data(), ry.size()) +
        std::string((const char*)Byte04.data(), Byte04.size()) +
        std::string((const char*)sx.data(), sx.size()) +
        std::string((const char*)sy.data(), sy.size());
    
    CryptoPP::SecByteBlock SharedSecret = KDF(finalMessage);

    return SharedSecret;
}

CryptoPP::SecByteBlock UECIES::KDF(std::string Input)
{
    CryptoPP::SecByteBlock derived(CryptoPP::SHA256::DIGESTSIZE);
    CryptoPP::HKDF<CryptoPP::SHA256> hkdf;
    hkdf.DeriveKey(derived, derived.size(), (const CryptoPP::byte*)Input.data(), Input.size(), NULL, NULL, NULL, NULL);
    return derived;
}

CryptoPP::SecByteBlock UECIES::ZeroPad(CryptoPP::SecByteBlock InBlock, size_t DesiredLength)
{
    if (InBlock.size() >= DesiredLength) {
        return InBlock;  // No need to zero-pad
    }

    int size = DesiredLength - InBlock.size();
    CryptoPP::byte* byteArray = new CryptoPP::byte[size];
    memset(byteArray, 0x00, size);

    CryptoPP::SecByteBlock ZeroPadBlock(byteArray, sizeof(byteArray));
    ZeroPadBlock.Append(InBlock);
    return ZeroPadBlock;
}

FString UECIES::SecByteBlockToString(const CryptoPP::SecByteBlock& Block)
{
    std::string blockString = std::string((const char*)Block.data(), Block.size());
    std::string hexed;

    CryptoPP::StringSource ss(blockString, true,
        new CryptoPP::HexEncoder(
            new CryptoPP::StringSink(hexed), false
        )
    );

    FString result = FString(UTF8_TO_TCHAR(hexed.c_str()));
    UE_LOG(LogTemp, Log, TEXT("sec: %s"), *result);

    return result;
}