// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "MetamaskSession.h"
#include "MetamaskSocketWrapper.h"
#include "MetamaskParameters.h"
#include "MetamaskEthereumRequest.h"
#include "MetamaskEthereumResponse.h"
#include "MetamaskKeyExchangeMessage.h"
#include "MetamaskHelper.h"
#include "MetamaskWallet.generated.h"
/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FOnMetamaskWalletReady);
DECLARE_DELEGATE(FOnMetamaskWalletPaused);
DECLARE_DELEGATE(FOnMetamaskWalletConnected);
DECLARE_DELEGATE(FOnMetamaskWalletDisconnected);
DECLARE_DELEGATE(FOnMetamaskWalletAuthorized);
DECLARE_DELEGATE(FOnMetamaskWalletUnauthorized);
DECLARE_DELEGATE(FOnMetamaskAccountChanged);
DECLARE_DELEGATE(FOnMetamaskChainIdChanged);

UCLASS(Blueprintable)
class MMBP_API UMetamaskWallet: public UObject
{
	GENERATED_BODY()

public:
	UMetamaskWallet();
	~UMetamaskWallet();

	void Initialize(UMetamaskSession* session, UMetamaskSocketWrapper* socket, FString socketUrl);
	void Request(FMetamaskEthereumRequest Request);
	FString Connect();
	void Disconnect();
	void Dispose();

	UMetamaskSession* Session;
	UMetamaskSocketWrapper* Socket;
	FString SelectedAddress;
	FString SelectedChainId;
	FString WalletPublicKey;
	bool IsConnected() { return this->Connected; };
	bool IsPaused() { return this->Paused; };
	bool IsAuthorized() { return this->Authorized; };

	/* Delegates, Callbacks */
	FOnMetamaskWalletConnected OnMetamaskWalletConnected;
	FOnMetamaskWalletDisconnected OnMetamaskWalletDisconnected;
	FOnMetamaskWalletReady OnMetamaskWalletReady;
	FOnMetamaskWalletPaused OnMetamaskWalletPaused;
	FOnMetamaskWalletAuthorized OnMetamaskWalletAuthorized;
	FOnMetamaskWalletUnauthorized OnMetamaskWalletUnauthorized;
	FOnMetamaskAccountChanged OnMetamaskAccountChanged;
	FOnMetamaskChainIdChanged OnMetamaskChainIdChanged;

	FDelegateHandle ReadyHandle;

	FString GetConnectionUrl();

	void SetMetamaskSocketWrapper(UMetamaskSocketWrapper* socket);

	static TArray<FString> MethodsToRedirect;

protected:
	void SendMessage(TSharedPtr<FJsonObject> Data, bool Encrypt);
	void SendOriginatorInfo();
	void OnWalletPaused();
	void OnWalletResume();
	void OnWalletReady();
	void InitializeState();
	void OnSocketConnected();
	void onSocketDisconnected();
	void JoinChannel(FString channelId);
	void LeaveChannel(FString channelId);
	void OnMessageReceived(FString response, const TSharedPtr<FJsonValue>& JsonValue);
	void OnOtpReceived(int32 answer);
	void OnClientsWaitingToJoin(FString response, TSharedPtr<FJsonValue> JsonValue);
	void OnClientsConnected(FString response, TSharedPtr<FJsonValue> JsonValue);
	void OnClientsDisconnected(FString response, TSharedPtr<FJsonValue> JsonValue);
	void OnWalletAuthorized();
	void OnWalletUnauthorized();
	void OnEthereumRequestReceived(FString id, const TSharedPtr<FJsonObject> *DataObject);
	void OnEthereumRequestReceived(const TSharedPtr<FJsonObject>* DataObject);
	void OnAccountsChanged(FString address);
	void OnChainIdChanged(FString newChainId);
	void SendEthereumRequest(FString id, FMetamaskEthereumRequest request);

	bool Connected;
	bool Paused;
	bool Authorized;
	bool KeysExchanged;

	FString SocketUrl;
	FString MetamaskAppLinkUrl;
	FString MessageEventName;
	FString JoinChannelEventName;
	FString LeaveChannelEventName;
	FString ClientsConnectedEventName;
	FString ClientsDisconnectedEventName;
	FString ClientsWaitingToJoinEventName;
	FString ConnectionUrl;

	TMap<FString, FMetamaskEthereumRequest> SubmittedRequests;

private:
	int32 KeyStartHandshakeAttempts;
	static int32 KEY_HANDSHAKE_ATTEMPTS_THRESHOLD;
};
