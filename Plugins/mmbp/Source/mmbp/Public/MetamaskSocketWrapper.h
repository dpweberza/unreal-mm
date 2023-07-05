#pragma once

#include "CoreMinimal.h"
#include "SocketIONative.h"
#include "SocketIOClient.h"

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

	void Emit(FString EventName, FString Message);
	void On(FString EventName, const TFunction<void(const FString&, const TSharedPtr<FJsonValue>&)>& Callback);
	void Initialize(FString SocketUrl, TMap<FString, FString> SocketOptions);
	void ConnectAsync();
	void DisconnectAsync();
	void Dispose();

private:
	TSharedPtr<FSocketIONative> Socket;
	void OnConnected(const FString& SessionId);
};
