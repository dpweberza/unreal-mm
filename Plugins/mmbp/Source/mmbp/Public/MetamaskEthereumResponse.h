#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "UObject/NoExportTypes.h"
#include "MetamaskEthereumRequest.h"

struct FMetamaskEthereumResponse
{
    FMetamaskEthereumRequest *Request;

    const TSharedPtr<FJsonObject> *Result;

    FMetamaskEthereumResponse() = default;

    FMetamaskEthereumResponse(FMetamaskEthereumRequest *InRequest, const TSharedPtr<FJsonObject> *InResult) :
        Request(InRequest),
        Result(InResult)
    {}
};
