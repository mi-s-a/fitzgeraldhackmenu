#pragma once

#include <cstdint>

namespace Unity
{
    struct System_String;
}

namespace fitzgeraldhackmenu
{
    enum class Il2CppResolverState
    {
        NotStarted,
        WaitingForGameAssembly,
        Initialized,
        Failed
    };

    bool InitializeIl2CppResolver(bool waitForGameAssembly = true, int maxSecondsWait = 60);
    void StartIl2CppResolverInitialization();
    void* ResolveIl2CppMethodPointer(const char* className, const char* methodName, int argumentCount = -1);
    Il2CppResolverState GetIl2CppResolverState();
    std::uint64_t GetIl2CppUpdateTickCount();
    Unity::System_String* NewIl2CppString(const char* text);
}
