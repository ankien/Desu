#include "Memory.hpp"

uint8_t& Memory::operator[](uint32_t i) {
    // unmapped memory accesses will crash! this isn't supposed to happen in the first place but the GBA handles it fine
    switch(i >> 24) {
        case 0x00:
            return bios[i];
        case 0x02:
            return wramOnBoard[i-0x2000000];
        case 0x03:
            return wramOnChip[i-0x3000000];
        case 0x04:
            return IORegisters[i-0x4000000];
        case 0x05:
            return pram[i-0x5000000];
        case 0x06:
            return vram[i-0x6000000];
        case 0x07:
            return oam[i-0x7000000];
        case 0x08:
            return gamePak[i-0x8000000];
        case 0x0E:
            return gPakSram[i-0xE000000];
        default: // unmapped memory access, will crash! todo: handle unpredictable behavior
            break;
    }
}

// todo: make this constructor accept args to
// turn it into a general memory class for more than one system
Memory::Memory() {
    bios = new uint8_t[0x4000];
    wramOnBoard = new uint8_t[0x40000];
    wramOnChip = new uint8_t[0x8000];
    IORegisters = new uint8_t[0x3FF];
    pram = new uint8_t[0x400];
    vram = new uint8_t[0x18000];
    oam = new uint8_t[0x400];
    gamePak = new uint8_t[0x3000000];
    gPakSram = new uint8_t[0x10000];
}

// todo: create a saveRom function for different storage types (None, EEPROM-512/8, SRAM-32, Flash-64/128)
bool Memory::loadRom(std::string rom) {
    std::ifstream fileStream(rom.c_str(), std::ifstream::in | std::ifstream::binary);
    uint32_t romSizeInBytes = std::filesystem::file_size(rom);
    if(romSizeInBytes > 0x1FFFFFF)
        return 0;
    fileStream.read(reinterpret_cast<char*>(gamePak),romSizeInBytes);
    return 1;
}