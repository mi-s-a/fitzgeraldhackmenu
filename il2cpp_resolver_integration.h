#pragma once

#include <cstdint>

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
    Il2CppResolverState GetIl2CppResolverState();
    std::uint64_t GetIl2CppUpdateTickCount();
}
