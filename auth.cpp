#include "pch.h"
#include "auth.h"
#include "debug_console.h"
#include "il2cpp_resolver_integration.h"
#include <algorithm>
#include <atomic>
#include <string>
#include <fstream> // Required for file operations
#include <Unity/Structures/il2cpp.hpp>
#include <Unity/Structures/System_String.hpp>
#include <Unity/Structures/il2cppArray.hpp>
#include <Unity/Structures/il2cppDictionary.hpp>
#include <MinHook.h>

namespace
{
    using RoomDictionary = Unity::il2cppDictionary<Unity::System_String*, Unity::il2cppObject*>;
    using RoomIdMethod = void(__fastcall*)(void* instance, RoomDictionary* values, const Unity::il2cppMethodInfo* methodInfo);

    std::atomic_bool g_roomIdLoggerInstalled{ false };
    RoomIdMethod g_originalRoomIdMethod = nullptr;

    const char* gettype(Unity::il2cppObject* value)
    {
        return value && value->m_pClass && value->m_pClass->m_pName ? value->m_pClass->m_pName : "null";
    }

    std::string previewit(const std::string& key, Unity::il2cppObject* value)
    {
        if (!value) return "NULL";

        const char* typeName = gettype(value);
        if (strcmp(typeName, "String") == 0)
        {
            return reinterpret_cast<Unity::System_String*>(value)->ToString();
        }

        return std::string("<") + typeName + " @ 0x" + std::to_string(reinterpret_cast<std::uintptr_t>(value)) + ">";
    }

    void HelloILoveYou(RoomDictionary* values) {
        if (!values) return fitzgeraldhackmenu::Log("no");

        auto* entries = values->GetEntry();
        if (!entries) return fitzgeraldhackmenu::Log("oh no");

        const int count = values->m_iCount > 256 ? 256 : values->m_iCount;
        for (int index = 0; index < count; ++index)
        {
            auto& entry = entries[index];
            if (entry.m_iHashCode < 0 || !entry.m_tKey) continue;

            const std::string key = entry.m_tKey->ToString();
            Unity::il2cppObject* value = entry.m_tValue;
            const std::string preview = previewit(key, value);
            if (key == "AccountId")
            {
                Unity::System_String* managedStr = fitzgeraldhackmenu::NewIl2CppString(fitzgeraldhackmenu::accountid.c_str());
                entry.m_tValue = reinterpret_cast<Unity::il2cppObject*>(managedStr);
            }
            else if (key == "Role" || key == "InvitedRole")
            {
                if (value && strcmp(gettype(value), "Int32") == 0)
                {
                    *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(value) + sizeof(Unity::il2cppObject)) = 30;
                }
            }
        }
    }

    void __fastcall hook(void* instance, RoomDictionary* values, const Unity::il2cppMethodInfo* methodInfo) {
        HelloILoveYou(values);

        if (g_originalRoomIdMethod) {
            g_originalRoomIdMethod(instance, values, methodInfo);
        }
    }
}

namespace fitzgeraldhackmenu {
    std::string accountid = "1";

    // Helper function to handle the file logic
    void LoadOrSaveAccountId() {
        std::fstream file("accountid.txt", std::ios::in);

        if (file.is_open()) {
            // File exists, read the contents into accountid
            std::string tempId;
            if (std::getline(file, tempId) && !tempId.empty()) {
                accountid = tempId;
                Log("Loaded Account ID from file: %s", accountid.c_str());
            }
            file.close();
        }
        else {
            // File does not exist, create it and write the default accountid
            file.clear();
            file.open("accountid.txt", std::ios::out);
            if (file.is_open()) {
                file << accountid;
                file.close();
                Log("Created accountid.txt with default ID: %s", accountid.c_str());
            }
            else {
                Log("Failed to create accountid.txt");
            }
        }
    }

    void install() {
        // Initialize the account ID from the text file before doing anything else
        LoadOrSaveAccountId();

        bool expected = false;
        if (!g_roomIdLoggerInstalled.compare_exchange_strong(expected, true))
        {
            return;
        }

        void* target = ResolveIl2CppMethodPointer("CGCEKBCIHJC", "PPGFHEDFBEA", 1);
        if (!target) return;

        const MH_STATUS initStatus = MH_Initialize();
        if (initStatus != MH_OK && initStatus != MH_ERROR_ALREADY_INITIALIZED)
            return;

        const MH_STATUS createStatus = MH_CreateHook(target, &hook, reinterpret_cast<void**>(&g_originalRoomIdMethod));
        if (createStatus != MH_OK)
            return;

        const MH_STATUS enableStatus = MH_EnableHook(target);
        if (enableStatus != MH_OK)
            return;

        Log("installed CGCEKBCIHJC.PPGFHEDFBEA at 0x%p", target);
    }
}