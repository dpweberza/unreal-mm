#include "MetamaskSocketWrapper.h"

UMetamaskSocketWrapper::UMetamaskSocketWrapper()
{
}

UMetamaskSocketWrapper::~UMetamaskSocketWrapper()
{
}

void UMetamaskSocketWrapper::Initialize(FString SocketUrl, TMap<FString, FString> SocketOptions)
{

}

void UMetamaskSocketWrapper::ConnectAsync()
{}

void UMetamaskSocketWrapper::DisconnectAsync()
{}

void UMetamaskSocketWrapper::Dispose()
{}

void UMetamaskSocketWrapper::Emit(FString EventName, FString Message)
{
}

void UMetamaskSocketWrapper::On(FString EventName, const TFunction<void(FString)>& Callback)
{
}
