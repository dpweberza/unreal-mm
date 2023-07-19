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
int32 UMetamaskWallet::KEY_HANDSHAKE_ATTEMPTS_THRESHOLD = 10;

void UMetamaskWallet::Initialize(UMetamaskSession* InSession, UMetamaskSocketWrapper* InSocket, FString InSocketUrl)
{
    Session = InSession;
    Socket = InSocket;
    SocketUrl = InSocketUrl;

    // Setting up callbacks
    Socket->OnConnectedCallback = [this]() {
        OnSocketConnected();
    };
    Socket->OnDisconnectedCallback = [this]() {
        onSocketDisconnected();
    };

    MetamaskAppLinkUrl = "https://metamask.app.link";
    MessageEventName = "message";
    JoinChannelEventName = "join_channel";
    LeaveChannelEventName = "leave_channel";
    ClientsConnectedEventName = "clients_connected";
    ClientsDisconnectedEventName = "clients_disconnected";
    ClientsWaitingToJoinEventName = "clients_waiting_to_join";

    KeyStartHandshakeAttempts = 0;
}

UMetamaskWallet::UMetamaskWallet()
{
}

UMetamaskWallet::~UMetamaskWallet()
{
    //Dispose();
    //Session = nullptr;
    //Socket = nullptr;
    //Instance = nullptr;
}

void UMetamaskWallet::Request(FMetamaskEthereumRequest Request)
{
    if (Request.Method == "eth_requestAccounts" && !Connected)
    {
        // Check if way for us to manage connection_state so this doesn't get called many times to do Socket->ConnectAsync
        Connect();
    }
    else if (!Connected)
    {
        UE_LOG(LogTemp, Log, TEXT("Wallet not connected"));
    }
    else {
        FString Id = UMetamaskHelper::GenerateUUID();
        SubmittedRequests.Add(Id, Request);
        SendEthereumRequest(Id, Request);
    }
}

FString UMetamaskWallet::Connect()
{
    UE_LOG(LogTemp, Log, TEXT("Connecting..."));
    TMap<FString, FString> SocketOptions = {
        {TEXT("UserAgent"), TEXT("UnrealMetamask")}
    };
    Socket->Initialize(SocketUrl, SocketOptions);
    Socket->ConnectAsync();

    Session->SessionData.ChannelId = UMetamaskHelper::GenerateUUID(); // ChannelId must be UUIDv4
    //Session->SessionData.ChannelId = "8332f3b8-9ddc-4e07-8f67-b9bbaa4aa2dc";
    FString ChannelId = Session->SessionData.ChannelId;

    ConnectionUrl = MetamaskAppLinkUrl + TEXT("/connect?channelId=") + FGenericPlatformHttp::UrlEncode(ChannelId) + TEXT("&pubkey=") + FGenericPlatformHttp::UrlEncode(Session->PublicKey());

    return ConnectionUrl;
}

FString UMetamaskWallet::GetConnectionUrl()
{
    return ConnectionUrl;
}

void UMetamaskWallet::Disconnect()
{
    UE_LOG(LogTemp, Log, TEXT("Disconnected"));
    Connected = false;
    Authorized = false;
    Paused = false;
    KeysExchanged = false;

    if (SubmittedRequests.Num() > 0)
    {
        SubmittedRequests.Empty();
    }

    WalletPublicKey = FString();
    SelectedAddress = FString();
    SelectedChainId = FString();

    KeyStartHandshakeAttempts = 0;

    if (Socket != nullptr)
    {
        Socket->DisconnectAsync();
    }
    if (OnMetamaskWalletDisconnected.IsBound())
    {
        OnMetamaskWalletDisconnected.ExecuteIfBound();
    }
}

void UMetamaskWallet::Dispose()
{
    if (Socket != nullptr)
    {
        if (!Session->SessionData.ChannelId.IsEmpty())
        {
            LeaveChannel(Session->SessionData.ChannelId);
        }
        Socket->Dispose();
    }
    Disconnect();
}

void UMetamaskWallet::SetMetamaskSocketWrapper(UMetamaskSocketWrapper* socket)
{
    this->Socket = socket;
}

void UMetamaskWallet::SendMessage(TSharedPtr<FJsonObject> Data, bool Encrypt)
{
    FMetamaskMessage Message = Session->PrepareMessage(Data, Encrypt, WalletPublicKey);
    if (Paused)
    {
        UE_LOG(LogTemp, Log, TEXT("Send Message Paused"));
        ReadyHandle = OnMetamaskWalletReady.AddLambda([Message, this]() {
            Socket->Emit(MessageEventName, Message);
            OnMetamaskWalletReady.Remove(ReadyHandle);
        });
    }
    else {
        UE_LOG(LogTemp, Log, TEXT("Send Message Emit"));
        Socket->Emit(MessageEventName, Message);
    }
}

void UMetamaskWallet::SendOriginatorInfo()
{
    TSharedPtr<FJsonObject> Request = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> OriginatorInfo = MakeShareable(new FJsonObject());

    OriginatorInfo->SetStringField("title", "example");
    OriginatorInfo->SetStringField("url", "example.com");
    OriginatorInfo->SetStringField("platform", "unreal");
    OriginatorInfo->SetStringField("apiVersion", "1.0");

    Request->SetStringField("type", "originator_info");
    Request->SetObjectField("originatorInfo", OriginatorInfo);

    SendMessage(Request, true);
}

void UMetamaskWallet::OnWalletPaused()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Paused"));
    Paused = true;

    OnMetamaskWalletPaused.ExecuteIfBound();
}

void UMetamaskWallet::OnWalletResume()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Resumed"));
    Paused = false;

    OnMetamaskWalletConnected.ExecuteIfBound();

    InitializeState();

    FString Id = UMetamaskHelper::GenerateUUID();

    FMetamaskEthereumRequest Request;
    Request.Id = Id;
    Request.Method = "eth_requestAccounts";

    SubmittedRequests.Add(Id, Request);
    SendEthereumRequest(Id, Request);

    Connected = true;
}

void UMetamaskWallet::OnWalletReady()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Ready"));
    Paused = false;

    OnMetamaskWalletConnected.ExecuteIfBound();

    Connected = true;

    InitializeState();

    FString Id = UMetamaskHelper::GenerateUUID();

    FMetamaskEthereumRequest Request;
    Request.Id = Id;
    Request.Method = "eth_requestAccounts";

    SubmittedRequests.Add(Id, Request);
    SendEthereumRequest(Id, Request);

    OnMetamaskWalletReady.Broadcast();
}

void UMetamaskWallet::InitializeState()
{
    FString Id = UMetamaskHelper::GenerateUUID();
    FMetamaskEthereumRequest _Request;
    _Request.Id = Id;
    _Request.Method = "eth_requestAccounts";

    Request(_Request);
}

void UMetamaskWallet::OnSocketConnected()
{
    FString ChannelId = Session->SessionData.ChannelId;
    UE_LOG(LogTemp, Log, TEXT("Socket Connected"));
    //UE_LOG(LogTemp, Log, TEXT("Channel ID: %s"), *ChannelId);
    
    Socket->On(MessageEventName, [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
    {
        OnMessageReceived(Response, JsonValue);
    });
    Socket->On(FString::Printf(TEXT("%s-%s"), *MessageEventName, *ChannelId), [this](FString Response, const TSharedPtr<FJsonValue>& JsonValue)
        {
            OnMessageReceived(Response, JsonValue);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), *ClientsConnectedEventName, *ChannelId), [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
        {
            OnClientsConnected(Response, JsonValue);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), *ClientsDisconnectedEventName, *ChannelId), [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
        {
            OnClientsDisconnected(Response, JsonValue);
        });
    Socket->On(FString::Printf(TEXT("%s-%s"), *ClientsWaitingToJoinEventName, *ChannelId), [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
        {
            OnClientsWaitingToJoin(Response, JsonValue);
        });


    JoinChannel(ChannelId);
}

void UMetamaskWallet::onSocketDisconnected()
{
}

void UMetamaskWallet::JoinChannel(FString ChannelId)
{
    UE_LOG(LogTemp, Log, TEXT("Joining channel: %s"), *ChannelId);
    Socket->Emit(JoinChannelEventName, ChannelId);
}

void UMetamaskWallet::LeaveChannel(FString ChannelId)
{
    UE_LOG(LogTemp, Log, TEXT("Leaving channel: %s"), *ChannelId);
    Socket->Emit(LeaveChannelEventName, ChannelId);
}

void UMetamaskWallet::OnMessageReceived(FString Response, const TSharedPtr<FJsonValue> &JsonValue)
{
    //UE_LOG(LogTemp, Log, TEXT("Message received"));
    //UE_LOG(LogTemp, Log, TEXT("%s"), *Response);

    // We will check to see if key handshake attempts goes into a loop and exceeds threshold, we disconnect;
    if (KeyStartHandshakeAttempts > UMetamaskWallet::KEY_HANDSHAKE_ATTEMPTS_THRESHOLD)
    {
        Disconnect();
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;

    // Serialize JsonValue to see what it is
    if (JsonValue.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("JsonValue valid json"));
        if (JsonValue->Type == EJson::Array) {
            UE_LOG(LogTemp, Log, TEXT("JsonValue is array"));
            TArray<TSharedPtr<FJsonValue>> ArrayValue = JsonValue->AsArray();

            if (ArrayValue.Num() > 0)
            {
                UE_LOG(LogTemp, Log, TEXT("Using first element of json array"));
                JsonObject = ArrayValue[0]->AsObject();
            }
        }

        if (JsonValue->Type == EJson::Object)
        {
            UE_LOG(LogTemp, Log, TEXT("JsonValue is object"));
            JsonObject = JsonValue->AsObject();
            FString MessageType;
            FString Message;
            const TSharedPtr<FJsonObject>* MessageObject;
            // Try to see if Message is JsonObject
            FString JsonString;
            TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
            FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

            //UE_LOG(LogTemp, Log, TEXT("Object is %s"), *JsonString);

            if (JsonObject->TryGetObjectField(TEXT("message"), MessageObject)) {
                if (MessageObject->Get()->TryGetStringField(TEXT("type"), MessageType))
                {
                    // Do nothing as it would extract value to MessageType, Try so it doesn't throw exception
                }
            }
            else if (JsonObject->TryGetStringField(TEXT("message"), Message))
            {
            }

            //UE_LOG(LogTemp, Log, TEXT("Message is %s"), *Message);
            //UE_LOG(LogTemp, Log, TEXT("MessageType is %s"), *MessageType);

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
                        UE_LOG(LogTemp, Log, TEXT("%s"), *WalletPublicKey);
                        FMetamaskKeyExchangeMessage KeyExchangeACK{
                            "key_handshake_ACK",
                            Session->PublicKey()
                        };
                        SendMessage(KeyExchangeACK.ToJsonObject(), false);
                        KeysExchanged = true;
                        SendOriginatorInfo();
                        KeyStartHandshakeAttempts = 0; // Reset here as it is successful
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Encrypted message received"));
                FString DecryptedJsonString = Session->DecryptMessage(Message);
                if (DecryptedJsonString.IsEmpty())
                {
                    UE_LOG(LogTemp, Log, TEXT("Could not decrypt message, restarting key exchange"));
                    KeysExchanged = false;
                    FMetamaskKeyExchangeMessage KeyExchangeSYN{
                        "key_handshake_SYN",
                        Session->PublicKey()
                    };
                    SendMessage(KeyExchangeSYN.ToJsonObject(), false);
                    return;
                }

                UE_LOG(LogTemp, Log, TEXT("%s"), *DecryptedJsonString); // FIXME

                TSharedPtr<FJsonObject> DecryptedJsonObject;
                TSharedRef<TJsonReader<>> DecryptedJsonReader = TJsonReaderFactory<>::Create(DecryptedJsonString);

                if (FJsonSerializer::Deserialize(DecryptedJsonReader, DecryptedJsonObject) && DecryptedJsonObject.IsValid())
                {
                    FString DecryptedMessageType;
                    if (DecryptedJsonObject->TryGetStringField(TEXT("type"), DecryptedMessageType)) {
                        if (DecryptedMessageType == "pause")
                        {
                            OnWalletPaused();
                            return;
                        }
                        else if (DecryptedMessageType == "otp")
                        {
                            int32 OtpAnswer;
                            if (DecryptedJsonObject->TryGetNumberField(TEXT("otpAnswer"), OtpAnswer)) {
                                OnOtpReceived(OtpAnswer);
                            }
                            else {
                                UE_LOG(LogTemp, Log, TEXT("Could not get parse otp"));
                                return;
                            }
                        }
                        else if (DecryptedMessageType == "ready")
                        {
                            OnWalletReady();
                            return;
                        }

                        if (!Connected && DecryptedMessageType == "wallet_info" && Paused == true)
                        {
                            OnWalletResume();
                            return;
                        }
                    }

                    const TSharedPtr<FJsonObject>* DataObject;
                    UE_LOG(LogTemp, Log, TEXT("Trying get data object from message"));
                    if (DecryptedJsonObject->TryGetObjectField(TEXT("data"), DataObject))
                    {
                        UE_LOG(LogTemp, Log, TEXT("Got the data object"));
                        FString Id;
                        if (DataObject->Get()->TryGetStringField(TEXT("id"), Id))
                        {
                            OnEthereumRequestReceived(Id, DataObject);
                        }
                        else {
                            OnEthereumRequestReceived(DataObject);
                        }
                    }
                    else if (DecryptedJsonObject->TryGetObjectField(TEXT("walletinfo"), DataObject)) {
                        OnEthereumRequestReceived(DataObject);
                    }
                }
            }
        }
    }
}

void UMetamaskWallet::OnOtpReceived(int32 Answer)
{
    //UE_LOG(LogTemp, Log, TEXT("Displaying OTP Answer: %d"), Answer);
}

void UMetamaskWallet::OnClientsWaitingToJoin(FString Response, TSharedPtr<FJsonValue> JsonValue)
{
    UE_LOG(LogTemp, Log, TEXT("Clients waiting to join"));
}

void UMetamaskWallet::OnClientsConnected(FString Response, TSharedPtr<FJsonValue> JsonValue)
{
    UE_LOG(LogTemp, Log, TEXT("Clients connected"));
    
    if (!KeysExchanged)
    {
        UE_LOG(LogTemp, Log, TEXT("Exchanging keys"));
        FMetamaskKeyExchangeMessage KeyExchangeSYN{
            "key_handshake_SYN",
            Session->PublicKey()
        };
        //SendMessage(KeyExchangeSYN.ToJsonObject(), false);
    }
}

void UMetamaskWallet::OnClientsDisconnected(FString Response, TSharedPtr<FJsonValue> JsonValue)
{
    UE_LOG(LogTemp, Log, TEXT("Clients disconnected"));
    if (!Paused)
    {
        Connected = false;
        KeysExchanged = false;
        Disconnect();
    }
}

void UMetamaskWallet::OnWalletAuthorized()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Authorized"));
    if (!Authorized)
    {
        Authorized = true;
        OnMetamaskWalletAuthorized.ExecuteIfBound();
    }
}

void UMetamaskWallet::OnWalletUnauthorized()
{
    Authorized = false;
    OnMetamaskWalletUnauthorized.ExecuteIfBound();
    Disconnect();
}

void UMetamaskWallet::OnEthereumRequestReceived(FString Id, const TSharedPtr<FJsonObject> *DataObject)
{
    UE_LOG(LogTemp, Log, TEXT("OnEthereumRequestReceived"));
    FMetamaskEthereumRequest* Request = nullptr;
    if (SubmittedRequests.Num() > 0)
    {
        Request = SubmittedRequests.Find(Id);
    }

    if (Request != nullptr)
    {
        FString Error;
        FString Result;
        const TArray<TSharedPtr<FJsonValue>> *ResultArr;
        const TSharedPtr<FJsonObject>* ResultObject;

        if (DataObject->Get()->TryGetStringField("error", Error))
        {
            if (Request->Method == "eth_requestAccounts")
            {
                OnWalletUnauthorized();
            }

            FMetamaskEthereumResponse Response;
            Response.Request = Request;
            Response.Result = DataObject;
            //DEthereumRequestFailed.ExecuteIfBound(Response);
        }
        else {
            if (DataObject->Get()->TryGetStringField("result", Result) && Request->Method == "eth_chainId")
            {
                OnChainIdChanged(Result);
            }
            else if (DataObject->Get()->TryGetArrayField("result", ResultArr) && Request->Method == "eth_requestAccounts")
            {
                OnWalletAuthorized();
                OnAccountsChanged(ResultArr->GetData()->Get()->AsString());
            }
            else if (DataObject->Get()->TryGetObjectField("result", ResultObject) && Request->Method == "metamask_getProviderState")
            {
                FString ChainId;
                const TArray<TSharedPtr<FJsonValue>>* Accounts;

                if (ResultObject->Get()->TryGetStringField(TEXT("chainId"), ChainId))
                {
                    OnChainIdChanged(ChainId);
                }

                if (ResultObject->Get()->TryGetArrayField(TEXT("accounts"), Accounts))
                {
                    OnAccountsChanged(Accounts->GetData()->Get()->AsString());
                }
            }
            FMetamaskEthereumResponse Response;
            Response.Request = Request;
            Response.Result = DataObject;
            //DEthereumRequestResult.ExecuteIfBound(Response);
        }
    }
    else {
        UE_LOG(LogTemp, Log, TEXT("Could not find request associated with id: %s"), *Id);
    }
}

void UMetamaskWallet::OnEthereumRequestReceived(const TSharedPtr<FJsonObject>* DataObject)
{
    UE_LOG(LogTemp, Log, TEXT("OnEthereumRequestReceived - DataObject"));
    FString Method;
    if (DataObject->Get()->TryGetStringField(TEXT("method"), Method))
    {
        //UE_LOG(LogTemp, Log, TEXT("Method %s"), *Method);
        const TSharedPtr<FJsonObject>* ParamsObject;
        const TArray<TSharedPtr<FJsonValue>>* ParamsArr;
        if (Method == "metamask_accountsChanged")
        {
            if (DataObject->Get()->TryGetArrayField(TEXT("params"), ParamsArr))
            {
                OnAccountsChanged(ParamsArr->GetData()->Get()->AsString());
            }
        }
        else if (Method == "metamask_chainChanged")
        {
            if (DataObject->Get()->TryGetObjectField(TEXT("params"), ParamsObject))
            {
                FString ChainId;
                if (ParamsObject->Get()->TryGetStringField(TEXT("chainId"), ChainId))
                {
                    OnChainIdChanged(ChainId);
                }
            }
        }
    }
}

void UMetamaskWallet::OnAccountsChanged(FString Address)
{
    UE_LOG(LogTemp, Log, TEXT("Account changed"));
    SelectedAddress = Address;
    OnMetamaskAccountChanged.ExecuteIfBound();
    if (Paused)
    {
        OnWalletReady();
    }
}

void UMetamaskWallet::OnChainIdChanged(FString NewChainId)
{
    UE_LOG(LogTemp, Log, TEXT("Chain Id changed"));
    SelectedChainId = NewChainId;
    OnMetamaskChainIdChanged.ExecuteIfBound();
}

void UMetamaskWallet::SendEthereumRequest(FString Id, FMetamaskEthereumRequest Request)
{
    Request.Id = Id;
    UE_LOG(LogTemp, Log, TEXT("Sending a new request"));

    SendMessage(Request.ToJsonObject(), true);
}
