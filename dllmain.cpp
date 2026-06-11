// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "debug_console.h"
#include "dx11_imgui_hook.h"
#include "il2cpp_resolver_integration.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        fitzgeraldhackmenu::OpenDebugConsole();
        fitzgeraldhackmenu::PrintBanner();
        fitzgeraldhackmenu::Log("DLL_PROCESS_ATTACH");
        fitzgeraldhackmenu::StartIl2CppResolverInitialization();
        fitzgeraldhackmenu::StartDx11ImGuiHook();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        fitzgeraldhackmenu::Log("DLL_PROCESS_DETACH");
        fitzgeraldhackmenu::StopDx11ImGuiHook();
        fitzgeraldhackmenu::CloseDebugConsole();
        break;
    }
    return TRUE;
}

