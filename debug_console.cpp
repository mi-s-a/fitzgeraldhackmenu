#include "pch.h"

#include "debug_console.h"

#include <cstdarg>
#include <cstdio>

namespace
{
    bool g_consoleOpen = false;
}

namespace fitzgeraldhackmenu
{
    void OpenDebugConsole()
    {
        if (g_consoleOpen)
        {
            return;
        }

        if (AllocConsole())
        {
            FILE* stream = nullptr;
            freopen_s(&stream, "CONOUT$", "w", stdout);
            freopen_s(&stream, "CONOUT$", "w", stderr);
            freopen_s(&stream, "CONIN$", "r", stdin);
            SetConsoleTitleW(L"fitzge");
            g_consoleOpen = true;
            Log("console opened");
        }
    }

    void CloseDebugConsole()
    {
        if (!g_consoleOpen)
        {
            return;
        }

        Log("console closing");
        FreeConsole();
        g_consoleOpen = false;
    }

    void PrintBanner()
    {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console == INVALID_HANDLE_VALUE || console == nullptr)
        {
            return;
        }

        CONSOLE_SCREEN_BUFFER_INFO info = {};
        const bool hasInfo = GetConsoleScreenBufferInfo(console, &info) != FALSE;
        const WORD previousAttributes = hasInfo ? info.wAttributes : FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        const WORD cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

        SetConsoleTextAttribute(console, cyan);
        printf("\n");
        printf("   ___ _ _                           _     _\n");
        printf("  / __(_) |_ ______ _  ___ _ __ __ _| | __| |\n");
        printf(" / _\\ | | __|_  / _` |/ _ \\ '__/ _` | |/ _` |\n");
        printf("/ /   | | |_ / / (_| |  __/ | | (_| | | (_| |\n");
        printf("\\/    |_|\\__/___\\__, |\\___|_|  \\__,_|_|\\__,_|\n");
        printf("                |___/\n\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printf("              meow.net 2021 menu\n");
        printf("             by fitzgerald & zion\n");
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

        SetConsoleTextAttribute(console, previousAttributes);
        fflush(stdout);
    }

    void Log(const char* format, ...)
    {
        char message[1024] = {};

        va_list args;
        va_start(args, format);
        vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
        va_end(args);

        printf("[fitzgeraldhackmenu] %s\n", message);
        fflush(stdout);

        char debugMessage[1100] = {};
        snprintf(debugMessage, sizeof(debugMessage), "fitzgeraldhackmenu: %s\n", message);
        OutputDebugStringA(debugMessage);
    }
}
