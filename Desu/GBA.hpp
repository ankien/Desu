#pragma once
#include <cstdint>
#include <filesystem>
#include <cstdio>
#include <SDL.h>
#include "core/ARM7TDMI.hpp"
#include "hardware/GBAMemory.hpp"
#include "hardware/LCD.hpp"
#include "hardware/Keypad.hpp"
#include "Scheduler.hpp" // todo: implement this bad boy

struct GBA {
    /// Cycle scheduler variables ///
    uint32_t cyclesPassed = 0;
    int16_t cyclesSinceHBlank = 0;

    /// Hardware ///
    ARM7TDMI arm7tdmi{};
    GBAMemory* systemMemory;
    LCD lcd{};
    Keypad keypad{};

    GBA();

    // todo: implement a cached interpreter (fetch a batch of instructions before interpretation)
    void interpretARM();
    void interpretTHUMB();

    // Control helpers
    struct {
        uint8_t jit : 1; // not implemented lol
    } runtimeOptions;
    bool parseArguments(uint64_t argc, char* argv[]);
    void run(char*);
};