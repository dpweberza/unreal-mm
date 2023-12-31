// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MetamaskWallet.h"
#include "MetamaskSession.h"
#include "MetamaskSocketWrapper.h"
#include "MetamaskEthereumRequest.h"
#include "MetamaskHelper.h"
#include "mmbpBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

DECLARE_DYNAMIC_DELEGATE(FWalletDisconnectDelegate);
DECLARE_DYNAMIC_DELEGATE(FWalletConnectDelegate);

UCLASS()
class UmmbpBPLibrary : public UBlueprintFunctionLibrary
{
	FWalletDisconnectDelegate OnWalletDisconnect;
	FWalletConnectDelegate OnWalletConnect;

	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "mmbp sample test testing"), Category = "MetaMask")
	static float mmbpSampleFunction(float Param);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate QR Code", Keywords = "MetaMask"), Category = "MetaMask")
		static UTexture2D* GenerateQrCode(FString TextToConvert	);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize Wallet", Keywords = "MetaMask"), Category = "MetaMask")
		static UMetamaskWallet* InitializeWallet();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Connect Wallet", Keywords = "MetaMask"), Category = "MetaMask")
		static FString ConnectWallet(UMetamaskWallet* Wallet);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SendTransaction", Keywords = "MetaMask"), Category = "MetaMask")
		static void SendTransaction(UMetamaskWallet* Wallet);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IsWalletAuthorized", Keywords = "MetaMask"), Category = "MetaMask")
		static bool IsWalletAuthorized(UMetamaskWallet* Wallet);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetupCallbacks", Keywords = "MetaMask"), Category = "MetaMask")
		static void SetupCallbacks(
			UMetamaskWallet* Wallet, 
			const FWalletDisconnectDelegate& OnWalletDisconnect,
			const FWalletConnectDelegate& OnWalletConnect
		);
};
