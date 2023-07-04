#include "MetamaskTransport.h"

UMetamaskTransport::UMetamaskTransport()
{
}

UMetamaskTransport::~UMetamaskTransport()
{
}

bool UMetamaskTransport::Connect(FString ConnectionUrl)
{
	return true;
}

void UMetamaskTransport::OnSuccess()
{
}

void UMetamaskTransport::OnFailure(FString Error)
{
}

void UMetamaskTransport::OnConnectRequest(FString ConnectionUrl)
{}
