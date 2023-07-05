#include "MetamaskHelper.h"

FString UMetamaskHelper::GenerateUUID()
{
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	UUIDv4::UUID uuid = uuidGenerator.getUUID();
	return FString(uuid.str().c_str());
}