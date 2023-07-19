#include "MetamaskSocketWrapper.h"

UMetamaskSocketWrapper::UMetamaskSocketWrapper()
{
}

UMetamaskSocketWrapper::~UMetamaskSocketWrapper()
{
}

void UMetamaskSocketWrapper::Initialize(FString InSocketUrl, TMap<FString, FString> SocketOptions)
{
	UE_LOG(LogTemp, Log, TEXT("Initializing Socket with url: %s"), *SocketUrl);
	Socket = ISocketIOClientModule::Get().NewValidNativePointer(InSocketUrl.Contains("https"));
	Socket->OnConnectedCallback = [this](FString SessionId) {
		OnConnected(SessionId);
	}; 
	Socket->OnDisconnectedCallback = [this](const ESIOConnectionCloseReason Reason) {
		OnDisconnected();
	};
	Socket->VerboseLog = true;
	SocketUrl = InSocketUrl;
	UE_LOG(LogTemp, Log, TEXT("Finished initializing socket"));
}

void UMetamaskSocketWrapper::ConnectAsync()
{
	UE_LOG(LogTemp, Log, TEXT("Socket Connecting async: %s"), *SocketUrl);
	Socket->Connect(SocketUrl);
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

void UMetamaskSocketWrapper::Emit(FString EventName, FMetamaskMessage Message)
{
	//UE_LOG(LogTemp, Log, TEXT("Socket Emit: %s %s"), *EventName, *Message.ToJsonString());
	Socket->Emit(EventName, Message.ToJsonObject());
}

void UMetamaskSocketWrapper::Emit(FString EventName, FString Message)
{
	//UE_LOG(LogTemp, Log, TEXT("Socket Emit: %s %s"), *EventName, *Message);
	Socket->Emit(EventName, Message);
}

void UMetamaskSocketWrapper::On(FString EventName, const TFunction<void(const FString&, const TSharedPtr<FJsonValue>&)>& Callback)
{
	UE_LOG(LogTemp, Log, TEXT("Adding listener for event: %s"), *EventName);
	Socket->OnEvent(EventName, Callback);
}
