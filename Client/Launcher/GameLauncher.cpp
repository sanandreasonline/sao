// Game EXE loader
// Author(s):       iFarbod <ifarbod@outlook.com>
//                  NTAuthority
//
// Copyright (c) 2015-2017 Project CtNorth
//
// Distributed under the MIT license (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT)

#include "Precompiled.hpp"
#include "Main.hpp"
#include <winternl.h>
#include <Hooking/HookingUtils.hpp>
#include <Container/Str.hpp>

using namespace Util;

static LONG NTAPI HandleVariant(PEXCEPTION_POINTERS exceptionInfo)
{
    SetForegroundWindow(GetDesktopWindow());
    return (exceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

void GameLauncher::InvokeEntryPoint(void(*entryPoint)())
{
    // SEH call to prevent STATUS_INVALID_HANDLE
    __try
    {
        // and call the entry point
        entryPoint();
    }
    __except (HandleVariant(GetExceptionInformation()))
    {
    }
}

void GameLauncher::Launch(const char* gamePath)
{
    // load the game executable data in temporary memory
    FILE* gameFile = _wfopen(WString{ gamePath }.CString(), L"rb");

    if (!gameFile)
    {
        return;
    }

    // find the file length and allocate a related buffer
    u32 length;
    u8* data;

    fseek(gameFile, 0, SEEK_END);
    length = ftell(gameFile);

    data = new u8[length];

    // seek back to the start and read the file
    fseek(gameFile, 0, SEEK_SET);
    fread(data, 1, length, gameFile);

    // close the file, and continue on
    fclose(gameFile);

    // load the executable into our module context
    HMODULE exeModule = GetModuleHandleW(nullptr);

    ExecutableLoader exeLoader(data);

    exeLoader.SetLibraryLoader([] (const char* libName)
    {
        OutputDebugStringA(va("[LibraryLoader] %s\n", libName));

        if (String(libName).ToLower() == "vorbisfile.dll")
        {
            return LoadLibraryW(L"sdvf.dll");
        }

        if (String(libName).ToLower() == "eax.dll")
        {
            return LoadLibraryW(L"sde.dll");
        }

        return LoadLibraryA(libName);
    });

    exeLoader.LoadIntoModule(exeModule);

    // free the old binary
    delete[] data;

    DWORD oldProtect;

    // apply memory protection
    VirtualProtect((void*)0x401000, 0x456000, PAGE_EXECUTE_READ, &oldProtect); // .text
    VirtualProtect((void*)0x857000, 0x1000, PAGE_EXECUTE_READ, &oldProtect); // _rwcseg
    VirtualProtect((void*)0x858000, 0x4C000, PAGE_READONLY, &oldProtect); // .idata/.rdata
    VirtualProtect((void*)0x8A4000, 0x40C000, PAGE_READWRITE, &oldProtect); // .data/.idata/.data/_rwdseg
    VirtualProtect((void*)0xCB0000, 0x1000, PAGE_READWRITE, &oldProtect); // .rsrc

    // Use our icon
    Util::MemPatch<u8>(0x7486A5, 1);

    // Patch IsAlreadyRunning
    Util::MakeRET0(0x7468E0);
    // Change CdStream semaphore to allow more than 2 SA instances
    //Util::StrCpy_(0x858AD4, "semaphore");

    LoadLibraryA(CLIENT_CORE_NAME DEBUG_SUFFIX LIB_EXTENSION);

    // get the entry point
    void(*entryPoint)();
    entryPoint = (void(*)())exeLoader.GetEntryPoint();

    // call the entry point
    AddVectoredExceptionHandler(0, HandleVariant);
    return InvokeEntryPoint(entryPoint);
}
