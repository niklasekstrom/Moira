// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef TESTCPU_H
#define TESTCPU_H

#include "Moira.h"

using namespace moira;

class TestCPU : public Moira {

    future issueRead(u32 address, u8 size);
    void issueWrite(u32 address, u32 data, u8 size);

    void submitAccesses();
    u32 getAccessSlotFutureValue(int accessSlot) override;

    void sync(int cycles) override;
    future read8(u32 addr) override { return issueRead(addr, 1); };
    future read16(u32 addr) override { return issueRead(addr, 2); };
    u16 read16OnReset(u32 addr) override;
    u16 read16Dasm(u32 addr) override;
    void write8 (u32 addr, u8  val) override;
    void write16 (u32 addr, u16 val) override;
    u16 readIrqUserVector(u8 level) const override;
    void breakpointReached(u32 addr) override;
    void watchpointReached(u32 addr) override;
};

#endif
