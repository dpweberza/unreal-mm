// Fill out your copyright notice in the Description page of Project Settings.


#include "MetamaskWallet.h"

TArray<FString> UMetamaskWallet::MethodsToRedirect = {
        "eth_sendTransaction",
        "eth_signTransaction",
        "eth_sign",
        "personal_sign",
        "eth_signTypedData",
        "eth_signTypedData_v3",
        "eth_signTypedData_v4",
        "wallet_watchAsset",
        "wallet_addEthereumChain",
        "wallet_switchEthereumChain"
};

UMetamaskWallet::UMetamaskWallet(UMetamaskSession* session, UMetamaskTransport* transport, UMetamaskSocketWrapper* socket, FString socketUrl):
    Session(session),
    Transport(transport),
    Socket(socket),
    SocketUrl(socketUrl)
{
    this->Socket->DSocketConnected.BindUObject(this, &UMetamaskWallet::OnSocketConnected);
    this->Socket->DSocketDisconnected.BindUObject(this, &UMetamaskWallet::onSocketDisconnected);
    this->MetamaskAppLinkUrl = "https://metamask.app.link";
    this->MessageEventName = "message";
    this->JoinChannelEventName = "join_channel";
    this->LeaveChannelEventName = "leave_channel";
    this->ClientsConnectedEventName = "clients_connected";
    this->ClientsDisconnectedEventName = "clients_disconnected";
    this->ClientsWaitingToJoinEventName = "clients_waiting_to_join";
}

UMetamaskWallet::~UMetamaskWallet()
{
}

void UMetamaskWallet::Request()
{
}

void UMetamaskWallet::Connect()
{
}

void UMetamaskWallet::Disconnect()
{
}

void UMetamaskWallet::Dispose()
{
}

void UMetamaskWallet::SetMetamaskTransport(UMetamaskTransport* transport)
{
    this->Transport = transport;
}

void UMetamaskWallet::SetMetamaskSocketWrapper(UMetamaskSocketWrapper* socket)
{
    this->Socket = socket;
}

void UMetamaskWallet::SendMessage(FMetamaskParameters Parameters, bool Encrypt)
{
}

void UMetamaskWallet::OnWalletPaused()
{
}

void UMetamaskWallet::OnWalletResume()
{
}

void UMetamaskWallet::OnWalletReady()
{
}

void UMetamaskWallet::InitializeState()
{
}

void UMetamaskWallet::OnSocketConnected()
{
}

void UMetamaskWallet::onSocketDisconnected()
{
}

void UMetamaskWallet::JoinChannel(FString channelId)
{
}

void UMetamaskWallet::LeaveChannel(FString channelId)
{
}

void UMetamaskWallet::OnMessageReceived(FString response)
{
}

void UMetamaskWallet::OnOtpReceived(int32 answer)
{
}

void UMetamaskWallet::OnClientsWaitingToJoin(FString response)
{
}

void UMetamaskWallet::OnClientsConnected(FString response)
{
}

void UMetamaskWallet::OnClientsDisconnected(FString response)
{
}

void UMetamaskWallet::OnWalletAuthorized()
{
}

void UMetamaskWallet::OnWalletUnauthorized()
{
}

void UMetamaskWallet::OnEthereumRequestReceived(FString id)
{
}

void UMetamaskWallet::OnAccountsChanged(FString address)
{
}

void UMetamaskWallet::OnChainIdChanged(FString newChainId)
{
}

void UMetamaskWallet::SendEthereumRequest(FString id, bool openTransport)
{
}

bool UMetamaskWallet::ShouldOpenMM(FString method)
{
    return false;
}
