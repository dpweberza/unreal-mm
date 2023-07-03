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
    Socket->DSocketConnected.BindUObject(this, &UMetamaskWallet::OnSocketConnected);
    Socket->DSocketDisconnected.BindUObject(this, &UMetamaskWallet::onSocketDisconnected);
    MetamaskAppLinkUrl = "https://metamask.app.link";
    MessageEventName = "message";
    JoinChannelEventName = "join_channel";
    LeaveChannelEventName = "leave_channel";
    ClientsConnectedEventName = "clients_connected";
    ClientsDisconnectedEventName = "clients_disconnected";
    ClientsWaitingToJoinEventName = "clients_waiting_to_join";
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

void UMetamaskWallet::SendMessage(TSharedPtr<FJsonObject> Data, bool Encrypt)
{
    FString Message = Session->PrepareMessage(Data, Encrypt, WalletPublicKey);
    if (Paused)
    {
        DWalletReady.BindLambda([Message, this]() {
            Socket->Emit(MessageEventName, Message);
            DWalletReady = FDelegateWalletReady();
        });
    }
    else {
        Socket->Emit(MessageEventName, Message);
    }
}

void UMetamaskWallet::SendOriginatorInfo()
{
}

void UMetamaskWallet::OnWalletPaused()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Paused"));
    Paused = true;

    DWalletPaused.ExecuteIfBound();
}

void UMetamaskWallet::OnWalletResume()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Resumed"));
    Paused = false;

    DWalletConnected.ExecuteIfBound();

    InitializeState();

    FString Id = FGuid::NewGuid().ToString();

    FMetamaskEthereumRequest Request{
        /*.Id =*/ Id,
        /*.Method =*/ "eth_requestAccounts",
        /*.Parameters =*/ {
            /*.Properties =*/ {}
        }
    };

    SendEthereumRequest(Id, Request, false);

    Connected = true;
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

void UMetamaskWallet::SendEthereumRequest(FString id, FMetamaskEthereumRequest Request, bool openTransport)
{
}

bool UMetamaskWallet::ShouldOpenMM(FString method)
{
    return false;
}
