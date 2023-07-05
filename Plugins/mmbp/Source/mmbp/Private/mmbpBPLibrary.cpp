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

void UmmbpBPLibrary::InitializeWallet()
{
	UMetamaskSession* Session = new UMetamaskSession();
	UMetamaskTransport* Transport = new UMetamaskTransport();
	UMetamaskSocketWrapper* Socket = new UMetamaskSocketWrapper();
	FString SocketUrl = TEXT("http://localhost:3000");
	UMetamaskWallet* Wallet = new UMetamaskWallet(Session, Transport, Socket, SocketUrl);
	UE_LOG(LogTemp, Log, TEXT("Initialised Wallet"));
	Wallet->Connect();
}
