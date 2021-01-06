#pragma once
#include <cstring>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include "../hardware/GBAMemory.hpp"

// debug console print, reeeally slow, like 1 fps slow
// file logging is faster but has limitations
//#define PRINT_INSTR

struct ARM7TDMI {
    // cycles per instruction
    int32_t cycleTicks;
    uint8_t s;
    uint8_t n;

    // lookup tables, array size is the different number of instructions
    void (ARM7TDMI::*armTable[4096])(uint32_t);
    void (ARM7TDMI::*thumbTable[256])(uint16_t);

    // could make this a generic pointer for code reuse with another system (the DS has an ARM7TDMI)
    GBAMemory* systemMemory;

    enum exceptions { Reset, UndefinedInstruction, SoftwareInterrupt, PrefetchAbort, DataAbort,
                      AddressExceeds26Bit, NormalInterrupt, FastInterrupt };

    enum modes { User = 16, FIQ, IRQ, Supervisor, Abort = 23, Undefined = 27, System = 31 };

    /// Registers ///
    // CPSR & SPSR = program status registers
    // registers are banked
    uint32_t reg[8]; // R0-7
    uint32_t r8[2]; // sys/user-fiq
    uint32_t r9[2]; // sys/user-fiq
    uint32_t r10[2]; // sys/user-fiq
    uint32_t r11[2]; // sys/user-fiq
    uint32_t r12[2]; // sys/user-fiq
    uint32_t r13[6]; // sys/user, fiq, svc, abt, irq, und - SP
    uint32_t r14[6]; // sys/user, fiq, svc, abt, irq, und - LR
    uint32_t pc; // R15
    // CPSR bitfield implementation
    uint8_t mode; // 5 : see enum modes
    bool    state, // 1 : 0 = ARM, 1 = THUMB
            fiqDisable, // 1 : 0 = enable, 1 = disable
            irqDisable; // 1 : 0 = enable, 1 = disable
    uint32_t reserved; // 19 : never used?
    bool stickyOverflow, // 1 : 1 = sticky overflow, ARMv5TE and up only
         overflowFlag, // V, 1 : 0 = no overflow, 1 = signed overflow (if the result register is a negative 2's compliment, set bit) 
         carryFlag, // C, 1 : 0 = borrow/no carry, 1 = carry/no borrow, (AKA unsigned overflow flag), for arithmetic ops, 1 if overflow in result of 32-bit register
         zeroFlag, // Z, 1 : 0 = not zero, 1 = zero
         signFlag; // N, 1 : 0 = not signed, 1 = signed (31 bit is filled)
    uint32_t spsr[6]; // N/A, fiq, svc, abt, irq, und

    /// Helper functions ///
    // todo: implement exception stuff at 0x3007F00
    void handleException(uint8_t, uint32_t, uint8_t);
    void fillARM();
    void fillTHUMB();
    uint16_t fetchARMIndex(uint32_t);
    uint8_t fetchTHUMBIndex(uint16_t);

    // memory helper functions
    void setSNCycles(uint8_t); // todo: implement waitstate cycles, currently using a scalable workaround
    void storeValue(uint16_t, uint32_t);
    void storeValue(uint32_t, uint32_t);
    // memory getters, rotates are for misaligned ldr+swp
    uint16_t readHalfWord(uint32_t);
    uint16_t readHalfWordRotate(uint32_t);
    uint32_t readWord(uint32_t);
    uint32_t readWordRotate(uint32_t);

    // more helpers
    uint32_t getModeArrayIndex(uint8_t, uint8_t);
    void setModeArrayIndex(uint8_t, uint8_t, uint32_t);
    uint32_t getArrayIndex(uint8_t);
    void setArrayIndex(uint8_t, uint32_t);
    uint32_t getCPSR();
    void setCPSR(uint32_t);
    bool checkCond(uint32_t);
    // ALU Helpers: shift for registers, ALUshift affects carry flags
    uint32_t shift(uint32_t, uint8_t, uint8_t);
    uint32_t ALUshift(uint32_t, uint8_t, uint8_t,bool);
    uint32_t sub(uint32_t,uint32_t,bool);
    uint32_t subCarry(uint32_t,uint32_t,bool);
    uint32_t add(uint32_t,uint32_t,bool);
    uint32_t addCarry(uint32_t,uint32_t,bool);
    void setZeroAndSign(uint32_t);

    // For unimplemented/undefined instructions
    void ARMundefinedInstruction(uint32_t);
    void ARMemptyInstruction(uint32_t);
    void THUMBemptyInstruction(uint16_t);

    /// ARM Instructions ///
    void ARMbranch(uint32_t);
    void ARMbranchExchange(uint32_t);
    void ARMsoftwareInterrupt(uint32_t);

    void ARMdataProcessing(uint32_t);
    void ARMmultiplyAndMultiplyAccumulate(uint32_t);

    void ARMpsrTransfer(uint32_t);

    void ARMsingleDataTransfer(uint32_t);
    void ARMhdsDataSTRH(uint32_t);
    void ARMhdsDataLDRH(uint32_t);
    void ARMhdsDataLDRSB(uint32_t);
    void ARMhdsDataLDRSH(uint32_t);
    void ARMblockDataTransfer(uint32_t);
    void ARMswap(uint32_t);

    /// THUMB Instructions ///
    void THUMBmoveShiftedRegister(uint16_t);
    void THUMBaddSubtract(uint16_t);
    void THUMBmoveCompareAddSubtract(uint16_t);
    void THUMBaluOperations(uint16_t);
    void THUMBhiRegOpsBranchEx(uint16_t);

    void THUMBloadPCRelative(uint16_t);
    void THUMBloadStoreRegOffset(uint16_t);
    void THUMBloadStoreSignExtendedByteHalfword(uint16_t);
    void THUMBloadStoreImmOffset(uint16_t);
    void THUMBloadStoreHalfword(uint16_t);
    void THUMBloadStoreSPRelative(uint16_t);

    void THUMBgetRelativeAddress(uint16_t);
    void THUMBaddOffsetToSP(uint16_t);

    void THUMBpushPopRegisters(uint16_t);
    void THUMBmultipleLoadStore(uint16_t);

    void THUMBconditionalBranch(uint16_t);
    void THUMBunconditionalBranch(uint16_t);
    void THUMBlongBranchWithLink(uint16_t);
    void THUMBsoftwareInterrupt(uint16_t);
};

// Bits 27-20 + 7-4
inline uint16_t ARM7TDMI::fetchARMIndex(uint32_t instruction) {
    return ((instruction >> 16) & 0xFF0) | ((instruction >> 4) & 0xF);
}
// Bits 15-8
inline uint8_t ARM7TDMI::fetchTHUMBIndex(uint16_t instruction) {
    return instruction >> 8;
}

inline void ARM7TDMI::setSNCycles(uint8_t accessWidth) {
    switch(pc >> 24) {
        case 0x02:
            if(accessWidth == 32)
                s = 6, n = 6;
            else
                s = 3, n = 3;
            break;
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
            if(accessWidth == 32)
                s = 8, n = 8;
            else
                s = 5, n = 5;
            break;
        case 0x0E:
            s = 5, n = 5;
            break;
        default:
            s = 1, n = 1;
    }
}
inline void ARM7TDMI::storeValue(uint16_t value, uint32_t address) {
    (*systemMemory)[address] = value;
    (*systemMemory)[address + 1] = (value & 0xFF00) >> 8;
}
inline void ARM7TDMI::storeValue(uint32_t value, uint32_t address) {
    (*systemMemory)[address] = value;
    (*systemMemory)[address + 1] = (value & 0xFF00) >> 8;
    (*systemMemory)[address + 2] = (value & 0xFF0000) >> 16;
    (*systemMemory)[address + 3] = (value & 0xFF000000) >> 24;
}
inline uint16_t ARM7TDMI::readHalfWord(uint32_t address) {
    return (*systemMemory)[address] |
           (*systemMemory)[address + 1] << 8;
}
inline uint16_t ARM7TDMI::readHalfWordRotate(uint32_t address) {
    uint16_t halfWord = readHalfWord(address);
    uint8_t rorAmount = (address & 1) << 3;
    return shift(halfWord,rorAmount,3);
}
inline uint32_t ARM7TDMI::readWord(uint32_t address) {
    return (*systemMemory)[address] |
           (*systemMemory)[address + 1] << 8 |
           (*systemMemory)[address + 2] << 16 |
           (*systemMemory)[address + 3] << 24;
}
inline uint32_t ARM7TDMI::readWordRotate(uint32_t address) {
    uint32_t word = readWord(address);
    uint8_t rorAmount = (address & 3) << 3;
    return shift(word,rorAmount,3);
}

inline uint32_t ARM7TDMI::getCPSR() {
    return mode |
           (state << 5) |
           (fiqDisable << 6) |
           (irqDisable << 7) |
           (reserved << 8) |
           (stickyOverflow << 27) |
           (overflowFlag << 28) |
           (carryFlag << 29) |
           (zeroFlag << 30) |
           (signFlag << 31);
}
inline void ARM7TDMI::setCPSR(uint32_t num) {
    mode = (num & 0x1F);
    state = (num & 0x20) >> 5;
    fiqDisable = (num & 0x40) >> 6;
    irqDisable = (num & 0x80) >> 7;
    reserved = (num & 0x7FFFF00) >> 8;
    stickyOverflow = (num & 0x8000000) >> 27;
    overflowFlag = (num & 0x10000000) >> 28;
    carryFlag = (num & 0x20000000) >> 29;
    zeroFlag = (num & 0x40000000) >> 30;
    signFlag = (num & 0x80000000) >> 31;
}

inline uint32_t ARM7TDMI::shift(uint32_t value, uint8_t shiftAmount, uint8_t shiftType) {
    switch(shiftType) {
        case 0b00: // lsl
            return value << shiftAmount; 
        case 0b01: // lsr
            return value >> shiftAmount;
        case 0b10: // asr
            if(value & 0x80000000)
                return (value >> shiftAmount) | (0xFFFFFFFF << shiftAmount);
            return value >> shiftAmount;
        case 0b11: // ror
            uint32_t dupeVal = value >> (shiftAmount % 32);
            return dupeVal | (value << ((32 - shiftAmount) % 32));
    }
}
inline uint32_t ARM7TDMI::ALUshift(uint32_t value, uint8_t shiftAmount, uint8_t shiftType, bool setFlags) {
    switch(shiftType) {
        case 0b00: // lsl
            if(shiftAmount == 0)
                return value;
            value <<= shiftAmount - 1;
            if(setFlags)
                carryFlag = 0x80000000 & value;
            return value << 1; 
        case 0b01: // lsr
            if(shiftAmount == 0) {
                if(setFlags)
                    carryFlag = value & 0x80000000;
                return 0;
            }
            value >>= shiftAmount - 1;
            if(setFlags)
                carryFlag = 1 & value;
            return value >> 1;
        case 0b10: // asr
            if(shiftAmount == 0)
                shiftAmount = 32;
            if(setFlags)
                carryFlag = 1 & (value >> (shiftAmount - 1));
            if(value & 0x80000000)
                return (value >> shiftAmount) | (0xFFFFFFFF << shiftAmount);
            return value >> shiftAmount;
        case 0b11: // ror
            {
            if(shiftAmount == 0)
                shiftAmount = 1;
            uint32_t dupeVal = value >> (shiftAmount % 32);
            value = dupeVal | (value << ((32 - shiftAmount) % 32));
            if(setFlags)
                carryFlag = value & 0x80000000;
            return value;
            }
    }
}
inline uint32_t ARM7TDMI::sub(uint32_t op1, uint32_t op2, bool setFlags) {
    uint32_t result = op1 - op2;
    if(setFlags) {
        carryFlag = op1 < op2;
        op1 >>= 31; op2 >>= 31;
        overflowFlag = (op1 ^ op2) ? (result >> 31) ^ op1 : 0; // if rn and op2 bits 31 are diff, check for overflow
    }
    return result;
}
inline uint32_t ARM7TDMI::subCarry(uint32_t op1, uint32_t op2, bool setFlags) {
    uint32_t result = op1 - op2 + carryFlag - 1;
    if(setFlags) {
        carryFlag = op1 < (op2 + carryFlag - 1);
        op1 >>= 31; op2 >>= 31;
        overflowFlag = (op1 ^ op2) ? (result >> 31) ^ op1 : 0;
    }
    return result;
}
inline uint32_t ARM7TDMI::add(uint32_t op1, uint32_t op2, bool setFlags) {
    uint32_t result = op1 + op2;
    if(s) {
        carryFlag = (*reinterpret_cast<uint64_t*>(&op1) + op2) > 0xFFFFFFFF;
        op1 >>= 31; op2 >>= 31;
        overflowFlag = (op1 ^ op2) ? 0 : (result >> 31) ^ op1;
    }
    return result;
}
inline uint32_t ARM7TDMI::addCarry(uint32_t op1, uint32_t op2, bool setFlags){
    uint32_t result = op1 + op2 + carryFlag;
    if(setFlags) {
        carryFlag = (*reinterpret_cast<uint64_t*>(&op1) + op2 + carryFlag) > 0xFFFFFFFF;
        op1 >>= 31; op2 >>= 31;
        overflowFlag = (op1 ^ op2) ? 0 : (result >> 31) ^ op1; // todo: check if overflow calc for carry opcodes are correct
    }
    return result;
}
inline void ARM7TDMI::setZeroAndSign(uint32_t arg) {
    (arg == 0) ?  zeroFlag = 1 : zeroFlag = 0;
    (arg & 0x80000000) ? signFlag = 1 : signFlag = 0;
}