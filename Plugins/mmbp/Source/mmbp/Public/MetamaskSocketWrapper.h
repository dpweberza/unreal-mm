#pragma once

#include "CoreMinimal.h"
#include "SocketIONative.h"
#include "SocketIOClient.h"
#include "MetamaskMessage.h"
#include <thread>
#include <chrono>

/**
 *
 */

class MMBP_API UMetamaskSocketWrapper
{

public:
	UMetamaskSocketWrapper();
	~UMetamaskSocketWrapper();

	TFunction<void()> OnConnectedCallback;
	TFunction<void()> OnDisconnectedCallback;

	void Emit(FString EventName, FMetamaskMessage Message);
	void Emit(FString EventName, FString Message);
	void On(FString EventName, const TFunction<void(const FString&, const TSharedPtr<FJsonValue>&)>& Callback);
	void Initialize(FString SocketUrl, TMap<FString, FString> SocketOptions);
	void ConnectAsync();
	void DisconnectAsync();
	void Dispose();

private:
	TSharedPtr<FSocketIONative> Socket;
	void OnConnected(const FString& SessionId);
	void OnDisconnected();

	FString SocketUrl;
};
