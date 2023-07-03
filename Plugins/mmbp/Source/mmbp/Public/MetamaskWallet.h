// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "MetamaskSession.h"
#include "MetamaskTransport.h"
#include "MetamaskSocketWrapper.h"
#include "MetamaskParameters.h"
#include "MetamaskWallet.generated.h"

/**
 * 
 */
UCLASS()
class MMBP_API UMetamaskWallet: public UObject
{
	GENERATED_BODY()

public:
	UMetamaskWallet() = default;
	UMetamaskWallet(UMetamaskSession* session, UMetamaskTransport* transport, UMetamaskSocketWrapper* socket, FString socketUrl);
	~UMetamaskWallet();

	void Request(/* MetamaskEthereumRequest request, */);
	void Connect();
	void Disconnect();
	void Dispose();

	UMetamaskSession* Session;
	UMetamaskTransport* Transport;
	UMetamaskSocketWrapper* Socket;
	FString SelectedAddress;
	FString SelectedChainId;
	FString WalletPublicKey;
	bool IsConnected;
	bool IsPaused;
	bool IsAuthorized;

	void SetMetamaskTransport(UMetamaskTransport* transport);
	void SetMetamaskSocketWrapper(UMetamaskSocketWrapper* socket);

	static TArray<FString> MethodsToRedirect;

protected:
	void SendMessage(FMetamaskParameters Parameters, bool Encrypt);
	void OnWalletPaused();
	void OnWalletResume();
	void OnWalletReady();
	void InitializeState();
	void OnSocketConnected();
	void onSocketDisconnected();
	void JoinChannel(FString channelId);
	void LeaveChannel(FString channelId);
	void OnMessageReceived(FString response);
	void OnOtpReceived(int32 answer);
	void OnClientsWaitingToJoin(FString response);
	void OnClientsConnected(FString response);
	void OnClientsDisconnected(FString response);
	void OnWalletAuthorized();
	void OnWalletUnauthorized();
	void OnEthereumRequestReceived(FString id);
	void OnAccountsChanged(FString address);
	void OnChainIdChanged(FString newChainId);
	void SendEthereumRequest(FString id, /* MetamaskEthereumRequest request, */ bool openTransport);
	bool ShouldOpenMM(FString method);

private:
	FString SocketUrl;
	FString MetamaskAppLinkUrl;
	FString MessageEventName;
	FString JoinChannelEventName;
	FString LeaveChannelEventName;
	FString ClientsConnectedEventName;
	FString ClientsDisconnectedEventName;
	FString ClientsWaitingToJoinEventName;
};
