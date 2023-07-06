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

void UMetamaskWallet::Initialize(UMetamaskSession* InSession, UMetamaskTransport* InTransport, UMetamaskSocketWrapper* InSocket, FString InSocketUrl)
{
    Session = InSession;
    Transport = InTransport;
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
}

UMetamaskWallet::UMetamaskWallet()
{
}

UMetamaskWallet::~UMetamaskWallet()
{
    Dispose();
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
        SendEthereumRequest(Id, Request, ShouldOpenMM(Request.Method));
    }
}

FString UMetamaskWallet::Connect()
{
    UE_LOG(LogTemp, Log, TEXT("Connecting..."));
    TMap<FString, FString> SocketOptions = {
        {TEXT("UserAgent"), Transport->UserAgent}
    };
    Socket->Initialize(SocketUrl, SocketOptions);
    Socket->ConnectAsync();

    Session->SessionData.ChannelId = UMetamaskHelper::GenerateUUID(); // ChannelId must be UUIDv4
    FString ChannelId = Session->SessionData.ChannelId;

    ConnectionUrl = MetamaskAppLinkUrl + TEXT("/connect?channelId=") + FGenericPlatformHttp::UrlEncode(ChannelId) + TEXT("&pubkey=") + FGenericPlatformHttp::UrlEncode(Session->PublicKey());

    if (!Transport->Connect(ConnectionUrl)) {
        UE_LOG(LogTemp, Log, TEXT("Opening transport for connection failed"));
    }

    return ConnectionUrl;
}

void UMetamaskWallet::Disconnect()
{
    UE_LOG(LogTemp, Log, TEXT("Disconnected"));
    Connected = false;
    Authorized = false;
    Paused = false;
    KeysExchanged = false;

    SubmittedRequests.Empty();

    WalletPublicKey = FString();
    SelectedAddress = FString();
    SelectedChainId = FString();

    Socket->DisconnectAsync();
    //DWalletDisconnected.ExecuteIfBound();
}

void UMetamaskWallet::Dispose()
{
    LeaveChannel(Session->SessionData.ChannelId);
    Disconnect();
    Socket->Dispose();
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
    FMetamaskMessage Message = Session->PrepareMessage(Data, Encrypt, WalletPublicKey);
    FString MessageString = Message.ToJsonString();
    UE_LOG(LogTemp, Log, TEXT("Send Message: Message is %s"), *MessageString);
    if (Paused)
    {
        UE_LOG(LogTemp, Log, TEXT("Send Message Paused"));
        //DWalletReady.BindLambda([MessageString, this]() {
        //    Socket->Emit(MessageEventName, MessageString);
        //    DWalletReady = FDelegateWalletReady();
        //});
    }
    else {
        UE_LOG(LogTemp, Log, TEXT("Send Message Emit"));
        Socket->Emit(MessageEventName, MessageString);
    }
}

void UMetamaskWallet::SendOriginatorInfo()
{
}

void UMetamaskWallet::OnWalletPaused()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Paused"));
    Paused = true;

    //DWalletPaused.ExecuteIfBound();
}

void UMetamaskWallet::OnWalletResume()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Resumed"));
    Paused = false;

    //DWalletConnected.ExecuteIfBound();

    InitializeState();

    FString Id = UMetamaskHelper::GenerateUUID();

    FMetamaskEthereumRequest Request{
        /*.Id =*/ Id,
        /*.Method =*/ "eth_requestAccounts",
        /*.Parameters =*/ {
            /*.Properties =*/ {}
        }
    };

    SubmittedRequests.Add(Id, Request);
    SendEthereumRequest(Id, Request, false);

    Connected = true;
}

void UMetamaskWallet::OnWalletReady()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet Ready"));
    Paused = false;

    //DWalletConnected.ExecuteIfBound();

    InitializeState();

    FString Id = UMetamaskHelper::GenerateUUID();

    FMetamaskEthereumRequest Request{
        /*.Id =*/ Id,
        /*.Method =*/ "eth_requestAccounts",
        /*.Parameters =*/ {
            /*.Properties =*/ {}
        }
    };

    SubmittedRequests.Add(Id, Request);
    SendEthereumRequest(Id, Request, false);

    Connected = true;

    //DWalletReady.ExecuteIfBound();

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
    UE_LOG(LogTemp, Log, TEXT("Channel ID: %s"), *ChannelId);
    
    Socket->On(MessageEventName, [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
    {
        OnMessageReceived(Response, JsonValue);
    });
    Socket->On(FString::Printf(TEXT("%s-%s"), *MessageEventName, *ChannelId), [this](FString Response, TSharedPtr<FJsonValue> JsonValue)
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

void UMetamaskWallet::OnMessageReceived(FString Response, TSharedPtr<FJsonValue> JsonValue)
{
    UE_LOG(LogTemp, Log, TEXT("Message received"));
    UE_LOG(LogTemp, Log, TEXT("%s"), *Response);

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
                JsonValue = ArrayValue[0];
            }
        }

        if (JsonValue->Type == EJson::Object)
        {
            UE_LOG(LogTemp, Log, TEXT("JsonValue is object"));
            TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
            FString MessageType;
            FString Message;
            const TSharedPtr<FJsonObject>* MessageObject;
            // Try to see if Message is JsonObject
            FString JsonString;
            TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
            FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

            UE_LOG(LogTemp, Log, TEXT("Object is %s"), *JsonString);

            if (JsonObject->TryGetObjectField(TEXT("message"), MessageObject)) {
                if (MessageObject->Get()->TryGetStringField(TEXT("type"), MessageType))
                {
                    // Do nothing as it would extract value to MessageType, Try so it doesn't throw exception
                }
            }
            else if (JsonObject->TryGetStringField(TEXT("message"), Message))
            {
            }

            UE_LOG(LogTemp, Log, TEXT("Message is %s"), *Message);
            UE_LOG(LogTemp, Log, TEXT("MessageType is %s"), *MessageType);

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
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Encrypted message received"));
                FString DecryptedJsonString;
                if (!Session->DecryptMessage(Message, DecryptedJsonString))
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

                UE_LOG(LogTemp, Log, TEXT("%"), *DecryptedJsonString); // FIXME

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

                        const TSharedPtr<FJsonObject>* DataObject;
                        if (DecryptedJsonObject->TryGetObjectField(TEXT("data"), DataObject))
                        {
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
}

void UMetamaskWallet::OnOtpReceived(int32 Answer)
{
    UE_LOG(LogTemp, Log, TEXT("Displaying OTP Answer: %d"), Answer);
}

void UMetamaskWallet::OnClientsWaitingToJoin(FString Response, TSharedPtr<FJsonValue> JsonValue)
{
    UE_LOG(LogTemp, Log, TEXT("Clients waiting to join"));
    Transport->OnConnectRequest(ConnectionUrl);
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
        SendMessage(KeyExchangeSYN.ToJsonObject(), false);
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
    if (!Authorized)
    {
        Authorized = true;
        //DWalletAuthorized.ExecuteIfBound();
    }
}

void UMetamaskWallet::OnWalletUnauthorized()
{
    Authorized = false;
    //DWalletUnauthorized.ExecuteIfBound();
    Disconnect();
}

void UMetamaskWallet::OnEthereumRequestReceived(FString Id, const TSharedPtr<FJsonObject> *DataObject)
{
    FMetamaskEthereumRequest *Request = SubmittedRequests.Find(Id);

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

            Transport->OnFailure(Error);
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
    FString Method;
    if (DataObject->Get()->TryGetStringField(TEXT("method"), Method))
    {
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
    //DAccountChanged.ExecuteIfBound();
    if (Paused)
    {
        OnWalletReady();
    }
}

void UMetamaskWallet::OnChainIdChanged(FString NewChainId)
{
    UE_LOG(LogTemp, Log, TEXT("Chain Id changed"));
    SelectedChainId = NewChainId;
    //DChainIdChanged.ExecuteIfBound();
}

void UMetamaskWallet::SendEthereumRequest(FString Id, FMetamaskEthereumRequest Request, bool OpenTransport)
{
    Request.Id = Id;
    UE_LOG(LogTemp, Log, TEXT("Sending a new request"));

    SendMessage(Request.ToJsonObject(), true);

    if (OpenTransport)
    {
        Transport->OnRequest(Id, Request);
    }
}

bool UMetamaskWallet::ShouldOpenMM(FString Method)
{
    if (Method == "eth_requestAccounts" && SelectedAddress.IsEmpty())
    {
        return true;
    }
    return false;
}
