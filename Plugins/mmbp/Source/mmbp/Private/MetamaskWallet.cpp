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

void UMetamaskWallet::Request(FMetamaskEthereumRequest Request)
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
    UE_LOG(LogTemp, Log, TEXT("Wallet Ready"));
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

    DWalletReady.ExecuteIfBound();

    Transport->OnSuccess();
}

void UMetamaskWallet::InitializeState()
{
    FMetamaskEthereumRequest _Request{
        "eth_requestAccounts",
        {
            {}
        }
    };
    Request(_Request);
}

void UMetamaskWallet::OnSocketConnected()
{
    FString ChannelId = Session->SessionData.ChannelId;
    UE_LOG(LogTemp, Log, TEXT("Socket Connected"));
    UE_LOG(LogTemp, Log, TEXT("Channel ID: %s"), &ChannelId);
    
    Socket->On(MessageEventName, [this](FString response)
    {
        OnMessageReceived(response);
    });
    Socket->On(FString::Printf(TEXT("%s-%s"), &MessageEventName, &ChannelId), [this](FString response)
        {
            OnMessageReceived(response);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), &ClientsConnectedEventName, &ChannelId), [this](FString response)
        {
            OnClientsConnected(response);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), &ClientsDisconnectedEventName, &ChannelId), [this](FString response)
        {
            OnClientsDisconnected(response);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), &ClientsWaitingToJoinEventName, &ChannelId), [this](FString response)
        {
            OnClientsWaitingToJoin(response);
        });


    JoinChannel(ChannelId);
}

void UMetamaskWallet::onSocketDisconnected()
{
}

void UMetamaskWallet::JoinChannel(FString ChannelId)
{
    UE_LOG(LogTemp, Log, TEXT("Joining channel: %s"), &ChannelId);
    Socket->Emit(JoinChannelEventName, ChannelId);
}

void UMetamaskWallet::LeaveChannel(FString ChannelId)
{
    UE_LOG(LogTemp, Log, TEXT("Leaving channel: %s"), &ChannelId);
    Socket->Emit(LeaveChannelEventName, ChannelId);
}

void UMetamaskWallet::OnMessageReceived(FString Response)
{
    UE_LOG(LogTemp, Log, TEXT("Message received"));
    UE_LOG(LogTemp, Log, TEXT("%s"), &Response);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response);

    if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->Values.Num() > 0)
        {
            for (const auto& Pair : JsonObject->Values)
            {
                TSharedPtr<FJsonValue> JsonValue = Pair.Value;
                FString Key = Pair.Key;

                if (JsonValue->Type == EJson::Array && Key == "0")
                {
                    TArray<TSharedPtr<FJsonValue>> JsonArray = JsonValue->AsArray();

                    if (JsonArray.Num() > 0)
                    {
                        if (JsonArray[0]->Type == EJson::Object)
                        {
                            JsonObject = JsonArray[0]->AsObject();
                        }
                    }
                    break;
                }
            }

            FString MessageType;
            FString Message;
            const TSharedPtr<FJsonObject> *MessageObject;
            // Try to see if Message is JsonObject
            if (JsonObject->TryGetObjectField(TEXT("message"), MessageObject)) {
                if (JsonObject->TryGetStringField(TEXT("messageType"), MessageType))
                {
                    // Do nothing as it would extract value to MessageType, Try so it doesn't throw exception
                }
            }
            else if (JsonObject->TryGetStringField(TEXT("message"), Message))
            {}

            if (MessageType == "key_handshake_start")
            {
                KeysExchanged = false;
                Paused = false;
                FMetamaskKeyExchangeMessage KeyExchangeSYN{
                    "key_handshake_SYN",
                    Session->PublicKey()
                };
                SendMessage(KeyExchangeSYN.ToJsonObject(), false);
            }
            else if (!KeysExchanged)
            {
                if (MessageType == "key_handshake_SYNACK")
                {
                    UE_LOG(LogTemp, Log, TEXT("Wallet public key"));
                    if (MessageObject->Get()->TryGetStringField("pubkey", WalletPublicKey)) {
                        UE_LOG(LogTemp, Log, TEXT("%s"), &WalletPublicKey);
                        FMetamaskKeyExchangeMessage KeyExchangeACK{
                            "key_handshake_ACK",
                            Session->PublicKey()
                        };
                        SendMessage(KeyExchangeACK.ToJsonObject(), false);
                        KeysExchanged = true;

                        SendOriginatorInfo();
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Encrypted message received"));
                FString DecryptedJson;


            }
        }
    }
}

void UMetamaskWallet::OnOtpReceived(int32 Answer)
{
}

void UMetamaskWallet::OnClientsWaitingToJoin(FString Response)
{
}

void UMetamaskWallet::OnClientsConnected(FString Response)
{
}

void UMetamaskWallet::OnClientsDisconnected(FString Response)
{
}

void UMetamaskWallet::OnWalletAuthorized()
{
}

void UMetamaskWallet::OnWalletUnauthorized()
{
}

void UMetamaskWallet::OnEthereumRequestReceived(FString Id)
{
}

void UMetamaskWallet::OnAccountsChanged(FString Address)
{
}

void UMetamaskWallet::OnChainIdChanged(FString NewChainId)
{
}

void UMetamaskWallet::SendEthereumRequest(FString Id, FMetamaskEthereumRequest Request, bool OpenTransport)
{
}

bool UMetamaskWallet::ShouldOpenMM(FString Method)
{
    return false;
}
