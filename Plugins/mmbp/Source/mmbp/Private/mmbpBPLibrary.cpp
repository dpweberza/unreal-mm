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
			FColor color = qr.getModule(x, y) ? black : white;
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

UMetamaskWallet* UmmbpBPLibrary::InitializeWallet()
{
	UMetamaskSession* Session = new UMetamaskSession();
	UMetamaskSocketWrapper* Socket = new UMetamaskSocketWrapper();
	FString SocketUrl = TEXT("https://metamask-sdk-socket.metafi.codefi.network");
	//FString SocketUrl = TEXT("http://localhost:3000");
	UMetamaskWallet* Wallet = NewObject<UMetamaskWallet>();
	Wallet->Initialize(Session, Socket, SocketUrl, "Shardbound", "www.shardbound.com");
	UE_LOG(LogTemp, Log, TEXT("Initialised Wallet"));

	return Wallet;
}

FString UmmbpBPLibrary::ConnectWallet(UMetamaskWallet* Wallet)
{
	if (Wallet != nullptr)
	{
		return Wallet->Connect();
	}
	return FString();
}

void UmmbpBPLibrary::SendTransaction(UMetamaskWallet* Wallet)
{
	if (Wallet != nullptr)
	{
		FString Id = UMetamaskHelper::GenerateUUID();
		TMap<FString, FString> Properties;
		Properties.Add("to", Wallet->SelectedAddress);
		Properties.Add("from", Wallet->SelectedAddress);
		Properties.Add("value", "0");
		FMetamaskParameters Params{ Properties };
		FMetamaskEthereumRequest Request{
			Id,
			"eth_sendTransaction",
			{
				Params
			}
		};
		Wallet->Request(Request);
	}
}

bool UmmbpBPLibrary::IsWalletAuthorized(UMetamaskWallet* Wallet)
{
	if (Wallet != nullptr) {
		return Wallet->IsAuthorized();
	}
	UE_LOG(LogTemp, Log, TEXT("WalletAuthorized - Wallet pointer is null"));
	return false;
}

void UmmbpBPLibrary::SetupCallbacks(
	UMetamaskWallet* Wallet,
	const FWalletDisconnectDelegate& WalletDisconnect,
	const FWalletConnectDelegate& WalletConnect
)
{
	Wallet->OnMetamaskWalletConnected.BindLambda([WalletConnect, Wallet]() {
		WalletConnect.ExecuteIfBound();
	});
	Wallet->OnMetamaskWalletDisconnected.BindLambda([WalletDisconnect, Wallet]() {
		WalletDisconnect.ExecuteIfBound();
	});
	Wallet->OnMetamaskWalletReady.AddLambda([]() {

	});
}
