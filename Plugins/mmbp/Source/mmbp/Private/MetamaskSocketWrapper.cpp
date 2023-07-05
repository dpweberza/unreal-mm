#include "MetamaskSocketWrapper.h"

UMetamaskSocketWrapper::UMetamaskSocketWrapper()
{
}

UMetamaskSocketWrapper::~UMetamaskSocketWrapper()
{
}

void UMetamaskSocketWrapper::Initialize(FString SocketUrl, TMap<FString, FString> SocketOptions)
{
	UE_LOG(LogTemp, Log, TEXT("Initializing Socket"));
	Socket = ISocketIOClientModule::Get().NewValidNativePointer();
	Socket->AddressAndPort = SocketUrl;
	Socket->OnConnectedCallback = [this](FString SessionId) {
		OnConnected(SessionId);
	};
	UE_LOG(LogTemp, Log, TEXT("Finished initializing socket"));
}

void UMetamaskSocketWrapper::ConnectAsync()
{
	UE_LOG(LogTemp, Log, TEXT("Socket Connecting async: %s"), &Socket->AddressAndPort);
	Socket->Connect(Socket->AddressAndPort);
}

void UMetamaskSocketWrapper::OnConnected(const FString& SessionId)
{
	UE_LOG(LogTemp, Log, TEXT("Socket OnConnected: %s"), &SessionId);
	OnConnectedCallback();
}

void UMetamaskSocketWrapper::DisconnectAsync()
{}

void UMetamaskSocketWrapper::Dispose()
{}

void UMetamaskSocketWrapper::Emit(FString EventName, FString Message)
{
}

void UMetamaskSocketWrapper::On(FString EventName, const TFunction<void(const FString&, const TSharedPtr<FJsonValue>&)>& Callback)
{
	Socket->OnEvent(EventName, Callback);
}
