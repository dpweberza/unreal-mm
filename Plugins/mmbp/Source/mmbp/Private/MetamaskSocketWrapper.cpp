#include "MetamaskSocketWrapper.h"

UMetamaskSocketWrapper::UMetamaskSocketWrapper()
{
}

UMetamaskSocketWrapper::~UMetamaskSocketWrapper()
{
}

void UMetamaskSocketWrapper::Initialize(FString SocketUrl, TMap<FString, FString> SocketOptions)
{
	UE_LOG(LogTemp, Log, TEXT("Initializing Socket with url: %s"), *SocketUrl);
	Socket = ISocketIOClientModule::Get().NewValidNativePointer(true);
	Socket->AddressAndPort = SocketUrl;
	Socket->OnConnectedCallback = [this](FString SessionId) {
		OnConnected(SessionId);
	}; 
	Socket->OnDisconnectedCallback = [this](const ESIOConnectionCloseReason Reason) {
		OnDisconnected();
	};
	Socket->VerboseLog = true;
	UE_LOG(LogTemp, Log, TEXT("Finished initializing socket"));
}

void UMetamaskSocketWrapper::ConnectAsync()
{
	UE_LOG(LogTemp, Log, TEXT("Socket Connecting async: %s"), &Socket->AddressAndPort);
	Socket->Connect(Socket->AddressAndPort);
}

void UMetamaskSocketWrapper::OnConnected(const FString& SessionId)
{
	UE_LOG(LogTemp, Log, TEXT("Socket OnConnected: %s"), *SessionId);
	OnConnectedCallback();
}

void UMetamaskSocketWrapper::OnDisconnected()
{
	OnDisconnectedCallback();
}

void UMetamaskSocketWrapper::DisconnectAsync()
{
	OnDisconnectedCallback();
}

void UMetamaskSocketWrapper::Dispose()
{
	Socket->ClearCallbacks();
}

void UMetamaskSocketWrapper::Emit(FString EventName, FString Message)
{
	Socket->Emit(EventName, Message);
}

void UMetamaskSocketWrapper::On(FString EventName, const TFunction<void(const FString&, const TSharedPtr<FJsonValue>&)>& Callback)
{
	UE_LOG(LogTemp, Log, TEXT("Adding listener for event: %s"), *EventName);
	Socket->OnEvent(EventName, Callback);
}
