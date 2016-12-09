// Game EXE loader
// Author(s):       iFarbod <ifarbod@outlook.com>
//                  NTAuthority
//
// Copyright (c) 2015-2016 The San Andreas Online Open Source Project
//
// Distributed under the MIT license (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT)

#include "Main.h"
#include <winternl.h>

static LONG NTAPI HandleVariant(PEXCEPTION_POINTERS exceptionInfo)
{
    SetForegroundWindow(GetDesktopWindow());
    return (exceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

void Game::InvokeEntryPoint(void(*entryPoint)())
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

void Game::Launch(const char* gamePath)
{
	wchar_t temp[MAX_PATH];
	mbstowcs(temp, gamePath, MAX_PATH);
	// load the game executable data in temporary memory
	FILE* gameFile = _wfopen(temp, L"rb");

	if (!gameFile)
	{
		return;
	}

	// find the file length and allocate a related buffer
	uint32_t length;
	uint8_t* data;

	fseek(gameFile, 0, SEEK_END);
	length = ftell(gameFile);

	data = new uint8_t[length];

	// seek back to the start and read the file
	fseek(gameFile, 0, SEEK_SET);
	fread(data, 1, length, gameFile);

	// close the file, and continue on
	fclose(gameFile);

	// load the executable into our module context
	HMODULE exeModule = GetModuleHandle(nullptr);

	ExecutableLoader exeLoader(data);

	exeLoader.SetLibraryLoader([] (const char* libName)
	{
		OutputDebugStringA(va("[LibraryLoader] %s\n", libName));

		return LoadLibraryA(libName);
	});

	exeLoader.LoadIntoModule(exeModule);

	// free the old binary
	delete[] data;

	DWORD
		oldProtect,
		oldProtect2;

	// apply memory protection
	VirtualProtect((void*)0x401000, 0x456000, PAGE_EXECUTE_READ, &oldProtect); // .text
	VirtualProtect((void*)0x857000, 0x1000, PAGE_EXECUTE_READ, &oldProtect); // _rwcseg
	VirtualProtect((void*)0x858000, 0x4C000, PAGE_READONLY, &oldProtect); // .idata/.rdata
	VirtualProtect((void*)0x8A4000, 0x40C000, PAGE_READWRITE, &oldProtect); // .data/.idata/.data/_rwdseg
	VirtualProtect((void*)0xCB0000, 0x1000, PAGE_READWRITE, &oldProtect); // .rsrc
	VirtualProtect((void*)0xCB1000, 0x8C6000, PAGE_EXECUTE_READWRITE, &oldProtect); // .text/.init/.data/.HOODLUM

	// use our icon
	Unprotect(0x7486A5, 1);
	*(BYTE *)0x7486A5 = 1;

	// patch IsAlreadyRunning
	VirtualProtect((void*)0x7468E0, 3, PAGE_READWRITE, &oldProtect);
	memcpy((void*)0x7468E0, "\x33\xC0\xC3", 3);
	VirtualProtect((void*)0x7468E0, 3, oldProtect, &oldProtect2);

	LoadLibrary("Core_d.dll");

	// get the entry point
	void(*entryPoint)();
	entryPoint = (void(*)())exeLoader.GetEntryPoint();

	// call the entry point
	AddVectoredExceptionHandler(0, HandleVariant);
	return InvokeEntryPoint(entryPoint);
}