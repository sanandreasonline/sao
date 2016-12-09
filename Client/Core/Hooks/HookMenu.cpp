// Menu hooks
// Author(s):       iFarbod <ifarbod@outlook.com>
//
// Copyright (c) 2015-2016 The San Andreas Online Open Source Project
//
// Distributed under the MIT license (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT)

#include "Precompiled.hpp"
#include <Hooking/HookingUtils.hpp>
#include <Hooking/HookFunction.hpp>

using namespace Util;

static bool menuFirstProcessed = false;
void ProcessFrontEndMenu()
{
    if (!menuFirstProcessed)
    {
        // Start the game now - unpause the timers
        MemPatch<u8>(0xB7CB49, 0);
        MemPatch<u8>(0xBA67A4, 0);
        MemPatch<u32>(0xC8D4C0, 8); // gGameState
        menuFirstProcessed = true;
    }

    ThisCall<void>(0x573A60); // Call original - Sets Render States
}

static Util::HookFunction hookFunction([]()
{
    // Hook menu process
    MakeCALL(0x57C2BC, ProcessFrontEndMenu);
});