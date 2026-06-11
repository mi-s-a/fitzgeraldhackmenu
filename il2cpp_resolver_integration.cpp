#include "pch.h"

#include "auth.h"
#include "il2cpp_resolver_integration.h"

#include <atomic>

#define IL2CPP_ASSERT(x) ((void)0)
#include <IL2CPP_Resolver.hpp>

namespace
{
    std::atomic_bool g_initializationStarted{ false };
    std::atomic<fitzgeraldhackmenu::Il2CppResolverState> g_resolverState{
        fitzgeraldhackmenu::Il2CppResolverState::NotStarted
    };
    std::atomic_uint64_t g_updateTickCount{ 0 };

    void OnIl2CppUpdate()
    {
        ++g_updateTickCount;
    }

    DWORD WINAPI InitializeResolverThread(LPVOID)
    {
        fitzgeraldhackmenu::InitializeIl2CppResolver();
        return 0;
    }
}

namespace fitzgeraldhackmenu
{
    std::vector<void*> FindIl2CppComponents(const char* typeName)
    {
        std::vector<void*> results;

        auto objects = Unity::Object::FindObjectsOfType<Unity::CComponent>(typeName);
        if (!objects)
            return results;

        for (uintptr_t i = 0; i < objects->m_uMaxLength; ++i)
        {
            Unity::CComponent* component = objects->m_pValues[i];
            if (component)
                results.push_back(component);
        }

        return results;
    }

    int GetComponentInt(void* component, const char* fieldName)
    {
        if (!component)
            return 0;

        return reinterpret_cast<IL2CPP::CClass*>(component)->GetMemberValue<int>(fieldName);
    }

    void SetComponentString(void* component, const char* fieldName, Unity::System_String* value)
    {
        if (!component)
            return;

        reinterpret_cast<IL2CPP::CClass*>(component)->SetMemberValue<Unity::System_String*>(fieldName, value);
    }

    void CallComponentMethodWithInt(void* component, const char* methodName, int value, Unity::System_String* text)
    {
        if (!component)
            return;

        void* args[] = { &value, text };
        reinterpret_cast<IL2CPP::CClass*>(component)->CallMethod<void>(methodName, args);
    }

    void* GetComponentGameObject(void* component)
    {
        return reinterpret_cast<Unity::CComponent*>(component)->GetGameObject();
    }

    bool InitializeIl2CppResolver(bool waitForGameAssembly, int maxSecondsWait)
    {
        g_resolverState = Il2CppResolverState::WaitingForGameAssembly;

        if (!IL2CPP::Initialize(waitForGameAssembly, maxSecondsWait))
        {
            g_resolverState = Il2CppResolverState::Failed;
            return false;
        }

        IL2CPP::Callback::Initialize();
        IL2CPP::Callback::OnUpdate::Add(reinterpret_cast<void*>(&OnIl2CppUpdate));
        install();

        g_resolverState = Il2CppResolverState::Initialized;
        return true;
    }

    Unity::System_String* NewIl2CppString(const char* text)
    {
        return IL2CPP::String::New(text);
    }

    void StartIl2CppResolverInitialization()
    {
        bool expected = false;
        if (!g_initializationStarted.compare_exchange_strong(expected, true))
        {
            return;
        }

        HANDLE thread = CreateThread(nullptr, 0, InitializeResolverThread, nullptr, 0, nullptr);
        if (thread != nullptr)
        {
            CloseHandle(thread);
        }
    }

    void* ResolveIl2CppMethodPointer(const char* className, const char* methodName, int argumentCount)
    {
        return IL2CPP::Class::Utils::GetMethodPointer(className, methodName, argumentCount);
    }

    Il2CppResolverState GetIl2CppResolverState()
    {
        return g_resolverState.load();
    }

    std::uint64_t GetIl2CppUpdateTickCount()
    {
        return g_updateTickCount.load();
    }
}
