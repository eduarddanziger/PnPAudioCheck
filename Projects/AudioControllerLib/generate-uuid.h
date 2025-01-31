#pragma once

#include <sstream>
#pragma comment(lib, "rpcrt4.lib") 
#include <rpc.h>
/*
* Generate a Universal Unique Identifier using windows api.
* Note: throw exceptions if uuid wasn't created successfully.
*/
// ReSharper disable once CppInconsistentNaming
inline std::string generate_uuid()
{
    UUID uuid;
    RPC_CSTR  uuidStr;

    if (UuidCreate(&uuid) != RPC_S_OK)
    {
        std::ostringstream os; os << "Couldn't create uuid, error code" << GetLastError() << ".";
        throw std::runtime_error(os.str());
    }

    if (UuidToStringA(&uuid, &uuidStr) != RPC_S_OK)
    {
        std::ostringstream os; os << "Couldn't convert uuid to string, error code" << GetLastError() << ".";
        throw std::runtime_error(os.str());
    }

    const std::string uuidOut = reinterpret_cast<const char*>(uuidStr);

    RpcStringFreeA(&uuidStr);
    return uuidOut;
}

inline std::wstring generate_w_uuid()
{
    UUID uuid;
    RPC_WSTR  uuidStr;

    if (UuidCreate(&uuid) != RPC_S_OK)
    {
        std::ostringstream os; os << "Couldn't create uuid, error code" << GetLastError() << ".";
        throw std::runtime_error(os.str());
    }

    if (UuidToStringW(&uuid, &uuidStr) != RPC_S_OK)
    {
        std::ostringstream os; os << "Couldn't convert uuid to string, error code" << GetLastError() << ".";
        throw std::runtime_error(os.str());
    }

    const std::wstring uuidOut = reinterpret_cast<const wchar_t*>(uuidStr);

    RpcStringFreeW(&uuidStr);
    return uuidOut;
}
