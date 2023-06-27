// Copyright Epic Games, Inc. All Rights Reserved.

#include "mmbpBPLibrary.h"
#include "mmbp.h"
#include "qrcodegen.hpp"

using qrcodegen::QrCode;

UmmbpBPLibrary::UmmbpBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UmmbpBPLibrary::mmbpSampleFunction(float Param)
{
	return -1;
}


UTexture2D* UmmbpBPLibrary::GenerateQrCode(FString TextToConvert)
{
	QrCode qr = QrCode::encodeText(TCHAR_TO_UTF8(*TextToConvert), QrCode::Ecc::LOW);

	uint8 size = qr.getSize();
	TArray<FColor> pixels;
	pixels.SetNumZeroed(size * size);

	FColor black = FColor::Black;
	FColor white = FColor::White;

	for (uint8 x = 0; x < size; x++)
	{
		for (uint8 y = 0; y < size; y++)
		{
			FColor color = qr.getModule(x, y) ? white : black;
			pixels[x + y * size] = color;
		}
	}

	UTexture2D* texture = UTexture2D::CreateTransient(size, size, EPixelFormat::PF_B8G8R8A8, "QRCode");
	void* data = texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(data, pixels.GetData(), size * size * 4);
	texture->PlatformData->Mips[0].BulkData.Unlock();
	texture->UpdateResource();

	texture->Filter = TextureFilter::TF_Nearest;

	UE_LOG(LogTemp, Warning, TEXT("Printing QR Code Texture"));

	return texture;
}

UECIES* UmmbpBPLibrary::Initialise()
{
	UECIES* Ecies = NewObject<UECIES>();
	return Ecies;
}

FString UmmbpBPLibrary::Encrypt(UECIES* Ecies, FString PlainText)
{
	return Ecies->Encrypt(PlainText);
}

FString UmmbpBPLibrary::GetPublicKey(UECIES* Ecies)
{
	return Ecies->GetPublicKeyAsString();
}

FString UmmbpBPLibrary::EncryptWithKey(UECIES* Ecies, FString PlainText, FString PublicKey)
{
	return Ecies->EncryptWithKey(PlainText, PublicKey);
}

FString UmmbpBPLibrary::Decrypt(UECIES* Ecies, FString CipherText)
{
	return Ecies->Decrypt(CipherText);
}
