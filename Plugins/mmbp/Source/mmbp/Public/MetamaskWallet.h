// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "MetamaskSession.h"
#include "MetamaskTransport.h"
#include "MetamaskSocketWrapper.h"
#include "MetamaskParameters.h"
#include "MetamaskEthereumRequest.h"
#include "MetamaskEthereumResponse.h"
#include "MetamaskKeyExchangeMessage.h"

/**
 * 
 */

//DECLARE_DELEGATE(FDelegateWalletReady);
//DECLARE_DELEGATE(FDelegateWalletPaused);
//DECLARE_DELEGATE(FDelegateWalletConnected);
//DECLARE_DELEGATE(FDelegateWalletDisconnected);
//DECLARE_DELEGATE(FDelegateWalletAuthorized);
//DECLARE_DELEGATE(FDelegateWalletUnauthorized);
//DECLARE_DELEGATE_OneParam(FDelegateEthereumRequestFailed, FMetamaskEthereumResponse);
//DECLARE_DELEGATE_OneParam(FDelegateEthereumRequestResult, FMetamaskEthereumResponse);
//DECLARE_DELEGATE(FDelegateAccountChanged);
//DECLARE_DELEGATE(FDelegateChainIdChanged);

class MMBP_API UMetamaskWallet
{

public:
	UMetamaskWallet() = default;
	UMetamaskWallet(UMetamaskSession* session, UMetamaskTransport* transport, UMetamaskSocketWrapper* socket, FString socketUrl);
	~UMetamaskWallet();

	void Request(FMetamaskEthereumRequest Request);
	void Connect();
	void Disconnect();
	void Dispose();

	UMetamaskSession* Session;
	UMetamaskTransport* Transport;
	UMetamaskSocketWrapper* Socket;
	FString SelectedAddress;
	FString SelectedChainId;
	FString WalletPublicKey;
	bool IsConnected() { return this->Connected; };
	bool IsPaused() { return this->Paused; };
	bool IsAuthorized() { return this->Authorized; };

	void SetMetamaskTransport(UMetamaskTransport* transport);
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
	void OnMessageReceived(FString response);
	void OnOtpReceived(int32 answer);
	void OnClientsWaitingToJoin(FString response);
	void OnClientsConnected(FString response);
	void OnClientsDisconnected(FString response);
	void OnWalletAuthorized();
	void OnWalletUnauthorized();
	void OnEthereumRequestReceived(FString id, const TSharedPtr<FJsonObject> *DataObject);
	void OnEthereumRequestReceived(const TSharedPtr<FJsonObject>* DataObject);
	void OnAccountsChanged(FString address);
	void OnChainIdChanged(FString newChainId);
	void SendEthereumRequest(FString id, FMetamaskEthereumRequest request, bool openTransport);
	bool ShouldOpenMM(FString method);

	bool Connected;
	bool Paused;
	bool Authorized;
	bool KeysExchanged;

	/* Delegates */
	//FDelegateWalletReady DWalletReady;
	//FDelegateWalletPaused DWalletPaused;
	//FDelegateWalletConnected DWalletConnected;
	//FDelegateWalletDisconnected DWalletDisconnected;
	//FDelegateWalletAuthorized DWalletAuthorized;
	//FDelegateWalletUnauthorized DWalletUnauthorized;
	//FDelegateEthereumRequestFailed DEthereumRequestFailed;
	//FDelegateEthereumRequestResult DEthereumRequestResult;
	//FDelegateAccountChanged DAccountChanged;
	//FDelegateChainIdChanged DChainIdChanged;

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
};
