template<typename T>
inline T& GBAMemory::memoryArray(uint32_t i) {
    switch(i >> 24) {
        case 0x00:
            if(i > 0x3FFF)
                return *reinterpret_cast<T*>(&ignore);
            return *reinterpret_cast<T*>(&bios[i]);
        case 0x02:
            return *reinterpret_cast<T*>(&wramOnBoard[(i-0x2000000) % 0x40000]);
        case 0x03:
            return *reinterpret_cast<T*>(&wramOnChip[(i-0x3000000) % 0x8000]);
        case 0x04:
            return *reinterpret_cast<T*>(&IORegisters[i-0x4000000]);
        case 0x05:
            return *reinterpret_cast<T*>(&pram[(i-0x5000000) % 0x400]);
        case 0x06:
            i = (i-0x6000000) % 0x20000;
            if(i > 0x17FFF)
                i -= 0x8000;
            return *reinterpret_cast<T*>(&vram[i]);
        case 0x07:
            return *reinterpret_cast<T*>(&oam[(i-0x7000000) % 0x400]);
        case 0x08:
        case 0x09:
            return *reinterpret_cast<T*>(&gamePak[i-0x8000000]);
        case 0x0A:
        case 0x0B:
            return *reinterpret_cast<T*>(&gamePak[i-0xA000000]);
        case 0x0C:
        case 0x0D:
            return *reinterpret_cast<T*>(&gamePak[i-0xC000000]);
        case 0x0E:
        case 0x0F:
            return *reinterpret_cast<T*>(&gPakSram[((i-0xE000000) % 0x10000) % 0x8000]);
        default:
            return *reinterpret_cast<T*>(&ignore); // ignore writes to misc addresses, reads are handled
    }
}
// return bitmask to write with, also handles special write behavior for certain regions (like IF regs)
template<typename T>
inline uint32_t GBAMemory::writeable(uint32_t address, T value) {
    
    uint8_t addressSection = address >> 24;
    
    if(addressSection == 0x00)
        return 0x0;
    
    // Some IO regs have special behavior, this is how I handle them
    // Gotta keep in mind that this approach might not work if the MMIO fields overlap
    // Also, word stores/reads are never unaligned
    if(addressSection == 0x04) {
        constexpr uint8_t offset = sizeof(T) - 1;
        uint16_t ioAddress = address & 0xFFF;

        // Affine BG reference point regs
        if(ioAddress < 0x40 && ioAddress > 0x27 - offset) {
            memoryArray<T>(address) = value;
            if(ioAddress < 0x2C && ioAddress > 0x27 - offset)
                internalRef[0].x = memoryArray<int32_t>(0x4000028);

            if(ioAddress < 0x30 && ioAddress > 0x2B - offset)
                internalRef[0].y = memoryArray<int32_t>(0x400002C);

            if(ioAddress < 0x3C && ioAddress > 0x37 - offset) 
                internalRef[1].x = memoryArray<int32_t>(0x4000038);

            if(ioAddress < 0x40 && ioAddress > 0x3B - offset)
                internalRef[1].y = memoryArray<int32_t>(0x400003C);

            return 0x0;
        }

        // DMA regs
        else if(ioAddress < 0xE0 && ioAddress > 0xA9 - offset) {
            uint8_t channel;

            if(address < 0x40000BC)
                channel = 0;
            else if(address < 0x40000C8)
                channel = 1;
            else if(address < 0x40000D4)
                channel = 2;
            else
                channel = 3;

            // if CNT_H will be modified
            uint32_t dmaCntHAddr = 0x40000BA + 12 * channel;
            uint16_t oldCntH = memoryArray<uint16_t>(dmaCntHAddr);
            const bool oldEnable = oldCntH & 0x8000;

            memoryArray<T>(address) = value; // write the new value

            uint16_t newCntH = memoryArray<uint16_t>(dmaCntHAddr);

            if(newCntH & 0x8000 && !oldEnable) {
                // reload SAD, DAD, and CNT_L
                internalSrc[channel] = memoryArray<uint32_t>(0x40000B0 + 12 * channel) & sourceAddressMasks[channel];
                internalDst[channel] = memoryArray<uint32_t>(0x40000B4 + 12 * channel) & destAddressMasks[channel];

                // only trigger dmas if if the enable bit is 1 and old enable bit was 0
                if((newCntH & 0x3000) == 0x0000)
                    dmaTransfer(channel,newCntH);
            }

            return 0x0;
        }

        // Timer control regs
        else if(ioAddress < 0x10F && ioAddress > 0x101 - offset) {
            // a rescheduable timer event (ticking) is added to the scheduler when a timer is enabled (when disabled), and removed when disabled or cascaded
            // if the timer cascading bit is modified while a timer is enabled, timer stops (or starts) immediately
            uint8_t timerId;
            if(ioAddress < 0x102)
                timerId = 0;
            else if(ioAddress < 0x106)
                timerId = 1;
            else if(ioAddress < 0x10A)
                timerId = 2;
            else
                timerId = 3;

            const uint32_t controlAddress = 0x4000102 + 4*timerId;

            bool oldTimerEnable = memoryArray<uint8_t>(controlAddress) & 0x80;
            bool oldTimerCascade = memoryArray<uint8_t>(controlAddress) & 0x4;

            memoryArray<T>(address) = value;

            bool newTimerEnable = memoryArray<uint8_t>(controlAddress) & 0x80;
            bool newTimerCascade = memoryArray<uint8_t>(controlAddress) & 0x4;
            const uint8_t prescalerSelection = memoryArray<uint8_t>(controlAddress) & 0x3;
            if(newTimerEnable && !oldTimerEnable) {
                internalTimer[timerId] = memoryArray<uint16_t>(controlAddress-2);
                uint8_t shiftAmount = prescalerSelection > 0 ? 1 << prescalerSelection*2 + 4 : 1;
                interrupts->scheduleTimerStep(timerId,shiftAmount);
            } else if(!newTimerEnable && oldTimerEnable || !oldTimerCascade && newTimerCascade)
                    interrupts->removeTimerSteps(timerId);

            return 0x0;
        }
        
        // Interrupt regs
        else if(ioAddress < 0x20A && ioAddress > 0x1FF - offset) {
            // if writing to IE or IME
            // todo: check if I also need to check for interrupts when respective hardware register IRQ bits are written to
            if(address >= 0x4000200 - offset && address < 0x4000202 || address >= 0x4000208 - offset && address <= 0x4000208) { // respective bit range for IO regs
                interrupts->scheduleInterruptCheck();
            }

            // if value overlaps with IF
            if(address >= 0x4000202 - offset && address <= 0x4000203) {
                uint16_t oldIF = memoryArray<uint16_t>(0x4000202);
                memoryArray<T>(address) = value;
                memoryArray<uint16_t>(0x4000202) = oldIF & ~memoryArray<uint16_t>(0x4000202);
                return 0x0;
            }
        }

        // HALTCNT reg
        else if(ioAddress < 0x302 && ioAddress > 0x300 - offset) {
            // writing 0 here switches the GBA into power saving mode until the next interrupt (no instructions executed)
            // schedule an event that skips to the next event and reschedules itself after the current front until (IE & IF) != 0
            memoryArray<T>(address) = value;
            if((memoryArray<uint8_t>(0x4000301) & 0x80) == 0)
                interrupts->scheduleHaltCheck();
            return 0x0;
        }
        
        return *reinterpret_cast<const uint32_t*>(&writeMask[ioAddress]);
    }

    return 0xFFFFFFFF;
}
inline uint32_t GBAMemory::readValue(uint32_t address) {
    uint32_t value = memoryArray<uint32_t>(address);

    // handle special MMIO reads
    if(address >> 24 == 0x04) {
        uint16_t ioAddress = address & 0xFFF;
        
        // Timer Reload regs
        if(ioAddress < 0x10D && ioAddress > 0xFD) {
            uint8_t timerId;
            if(ioAddress < 0x102)
                timerId = 0;
            else if(ioAddress < 0x106)
                timerId = 1;
            else if(ioAddress < 0x10A)
                timerId = 2;
            else
                timerId = 3;

            uint16_t tempTimer;
            const uint32_t reloadAddress = 0x4000100 + 4*timerId;
            tempTimer = memoryArray<uint16_t>(reloadAddress);
            memoryArray<uint16_t>(reloadAddress) = internalTimer[timerId];

            value = memoryArray<uint32_t>(address);
            
            memoryArray<uint16_t>(reloadAddress) = tempTimer;
        }
    }

    return value;
}
inline void GBAMemory::storeValue(uint8_t value, uint32_t address) {
    switch(address >> 24) {
        case 0x05:
            storeValue(static_cast<uint16_t>(static_cast<uint16_t>(value) * 0x101), address);
            break;
        case 0x06:
            if(IORegisters[0] < 3) { // bitmap mode writes
                if(address < 0x6014000)
                    storeValue(static_cast<uint16_t>(static_cast<uint16_t>(value) * 0x101), address);
            } else {
                if(address < 0x6010000)
                    storeValue(static_cast<uint16_t>(static_cast<uint16_t>(value) * 0x101), address);
            }
            return;
        case 0x07:
            return;
        default:
        {
            uint8_t mask = writeable<uint8_t>(address,value);
            uint8_t* mem = &memoryArray<uint8_t>(address);
            mem[0] = (value & mask) | (mem[0] & ~mask);
        }
    }
}
inline void GBAMemory::storeValue(uint16_t value, uint32_t address) {
    address = address & ~1;
    uint16_t mask = writeable<uint16_t>(address,value);
    uint16_t* mem = &memoryArray<uint16_t>(address);
    mem[0] = (value & mask) | (mem[0] & ~mask);
}
inline void GBAMemory::storeValue(uint32_t value, uint32_t address) {
    address = address & ~3;
    uint32_t mask = writeable<uint32_t>(address,value);
    uint32_t* mem = &memoryArray<uint32_t>(address);
    mem[0] = (value & mask) | (mem[0] & ~mask);
}
inline uint8_t GBAMemory::readByte(uint32_t address) {
    uint8_t memType = getUnusedMemType(address);
    if(memType)
        return readUnusedMem(cpuState->state,memType);
    return readValue(address);
}
inline uint16_t GBAMemory::readHalfWord(uint32_t address) {
    uint8_t memType = getUnusedMemType(address);
    if(memType)
        return readUnusedMem(cpuState->state,memType);
    address = address & ~1;
    return readValue(address);
}
inline uint32_t GBAMemory::readHalfWordRotate(uint32_t address) {
    uint8_t rorAmount = (address & 1) << 3;
    return ror(readHalfWord(address),rorAmount);
}
inline uint32_t GBAMemory::readWord(uint32_t address) {
    uint8_t memType = getUnusedMemType(address);
    if(memType)
        return readUnusedMem(cpuState->state,memType);
    address = address & ~3;
    return readValue(address);
}
inline uint32_t GBAMemory::readWordRotate(uint32_t address) {
    uint8_t rorAmount = (address & 3) << 3;
    return ror(readWord(address),rorAmount);
}

inline uint32_t GBAMemory::ror(uint32_t value, uint8_t shiftAmount) {
    shiftAmount &= 0x1F;
    return (value >> shiftAmount) | (value << (32 - shiftAmount));
}

template <uint16_t timing>
inline void GBAMemory::delayedDma() {
    for(uint8_t channel = 0; channel < 4; channel++) {
        uint16_t dmaCntH = memoryArray<uint16_t>(0x40000BA + 12*channel);
        if(dmaCntH & 0x8000 && (dmaCntH & 0x3000) == timing) {
            dmaTransfer(channel,dmaCntH);
        }
    }
}
inline void GBAMemory::dmaTransfer(uint8_t channel, uint16_t dmaCntH) {
    const uint8_t destAddressControl = (dmaCntH & 0x60) >> 5;
    int8_t destCtrlFactor = destAddressControl;
    int8_t srcCtrlFactor = (dmaCntH & 0x180) >> 7;
    int16_t startTiming = dmaCntH & 0x3000;
    const bool wordTransfer = dmaCntH & 0x400;
    uint8_t transferSize = wordTransfer ? 4 : 2;
    uint32_t length = memoryArray<uint16_t>(0x40000B8 + 12 * channel) & lengthMasks[channel];
    if(length == 0)
        channel == 3 ? length = 0x10000 : length = 0x4000;

    destCtrlFactor = getIncrementFactor(destCtrlFactor);
    srcCtrlFactor = getIncrementFactor(srcCtrlFactor);

    if(startTiming == 0x3000) {
        switch(channel) 	{
            case 1:
            case 2:
                length = 4;
                transferSize = 4;
                destCtrlFactor = 0;
                break;
            case 3:
                break;
                // todo: video capture
        }
    }

    int8_t destIncrement = transferSize * destCtrlFactor;
    int8_t srcIncrement = transferSize * srcCtrlFactor;

    if(wordTransfer) {
        internalSrc[channel] &= ~3;
        internalDst[channel] &= ~3;
        for(uint32_t i = 0; i < length; i++) {
            uint32_t value = memoryArray<uint32_t>(internalSrc[channel]);
            uint32_t mask = writeable<uint32_t>(internalDst[channel],value);
            uint32_t* mem = &memoryArray<uint32_t>(internalDst[channel]);
            mem[0] = (value & mask) | (mem[0] & ~mask);
            internalSrc[channel] += srcIncrement;
            internalDst[channel] += destIncrement;
        }
    } else {
        internalSrc[channel] &= ~1;
        internalDst[channel] &= ~1;
        for(uint32_t i = 0; i < length; i++) {
            uint16_t value = memoryArray<uint16_t>(internalSrc[channel]);
            uint16_t mask = writeable<uint16_t>(internalDst[channel],value);
            uint16_t* mem = &memoryArray<uint16_t>(internalDst[channel]);
            mem[0] = (value & mask) | (mem[0] & ~mask);
            internalSrc[channel] += srcIncrement;
            internalDst[channel] += destIncrement;
        }
    }

    // reset destination address
    if(destAddressControl == 3)
        internalDst[channel] = memoryArray<uint32_t>(0x40000B4 + 12*channel) & destAddressMasks[channel];

    // If the repeat bit is set at the end of a transfer; enable bit is set, else clear enable bit
    if(dmaCntH & 0x200 && startTiming != 0x0000) {
        memoryArray<uint8_t>(0x40000BB + 12*channel) |= 0x80;
    } else {
        memoryArray<uint8_t>(0x40000BB + 12*channel) &= 0x7F;
    }

    // check for DMA interrupts
    if(dmaCntH & 0x4000) {
        memoryArray<uint8_t>(0x4000202) |= 1 << channel;
        interrupts->scheduleInterruptCheck();
    }
}
inline int8_t GBAMemory::getIncrementFactor(uint8_t addressControl) {
    switch(addressControl) {
        case 0:
        case 3:
            return 1;
        case 1:
            return -1;
        case 2:
            return 0;
    }
}