// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "testrunner.h"

/* Signals the CPU clock to advance.
 *
 * Moira calls this function prior to each memory access and provides the
 * number of CPU cycles that have been elapsed since the previous call.
 * The demo application simply advances the CPU clock. A real-life would
 * emulate the surrounding hardware up the current CPU cycle to make sure
 * that the memory is up-to-date when the CPU accesses it.
 */
void
TestCPU::sync(int cycles)
{
    clock += cycles;
}

future
TestCPU::issueRead(u32 address, u8 size)
{
    int tail = accessSlotTail;
    accessSlotTail = (accessSlotTail + 1) & (ACCESS_SLOT_COUNT - 1);
    AccessSlot *slot = &accessSlots[tail];
    slot->address = address;
    slot->size = size;
    slot->read = 1;
    slot->state = AS_PENDING;

    submitAccesses();

    future fu = getNextFutureSlot();
    FutureSlot *fs = &futureSlots[fu];
    fs->kind = FK_ACCESS_SLOT;
    fs->accessSlot = tail;
    return fu;
}

void
TestCPU::issueWrite(u32 address, u32 data, u8 size)
{
    int tail = accessSlotTail;
    accessSlotTail = (accessSlotTail + 1) & (ACCESS_SLOT_COUNT - 1);
    AccessSlot *slot = &accessSlots[tail];
    slot->address = address;
    slot->data = data;
    slot->size = size;
    slot->read = 0;
    slot->state = AS_PENDING;

    submitAccesses();
}

void
TestCPU::submitAccesses()
{
    while (accessSlotHead != accessSlotTail) {
        int head = accessSlotHead;
        accessSlotHead = (accessSlotHead + 1) & (ACCESS_SLOT_COUNT - 1);
        AccessSlot *slot = &accessSlots[head];
        if (slot->read) {
            if (slot->size == 1)
                slot->data = get8(moiraMem, slot->address);
            else if (slot->size == 2)
                slot->data = get16(moiraMem, slot->address);
        } else {
            if (slot->size == 1)
                set8(moiraMem, slot->address, slot->data);
            else if (slot->size == 2)
                set16(moiraMem, slot->address, slot->data);
        }
        slot->state = AS_COMPLETED;
    }
}

u32
TestCPU::getAccessSlotFutureValue(int accessSlot)
{
    // All accesses are already completed when this method is invoked
    return accessSlots[accessSlot].data;
}

/* Reads a word from memory.
 *
 * This function is called by the disassembler to read a word from memory.
 * In contrast to read16, no side effects should be emulated.
 */
u16
TestCPU::read16Dasm(u32 addr)
{
    return (u16)getFutureValue(read16(addr));
}

/* Reads a word from memory.
 *
 * This function is called by the reset routine to read a word from memory.
 * It's up to you if you want to emulate side effects here.
 */
u16
TestCPU::read16OnReset(u32 addr)
{
    switch (addr) {
        case 0: return 0x0000;
        case 2: return 0x2000;
        case 4: return 0x0000;
        case 6: return 0x1000;
    }
    return (u16)getFutureValue(read16(addr));
}

/* Writes a byte into memory.
 *
 * This function is called whenever the 68000 CPU writes a byte into memory.
 * It should emulate the write access including all side effects.
 */
void
TestCPU::write8(u32 addr, u8  val)
{
    if (CHECK_MEM_WRITES)
        sandbox.replayPoke(POKE8, addr, getClock(), readFC(), val);
    issueWrite(addr, val, 1);
}

/* Writes a word into memory.
 *
 * This function is called whenever the 68000 CPU writes a word into memory.
 * It should emulate the write access including all side effects.
 */
void
TestCPU::write16 (u32 addr, u16 val)
{
    if (CHECK_MEM_WRITES)
        sandbox.replayPoke(POKE16, addr, moiracpu->getClock(), readFC(), val);
    issueWrite(addr, val, 2);
}

/* Returns the interrupt vector in IRQ_USER mode
 */
u16
TestCPU::readIrqUserVector(moira::u8 level) const { return 0; }

/* Breakpoint handler
 *
 * Moira calls this function to signal that a breakpoint has been reached.
 */
void
TestCPU::breakpointReached(moira::u32 addr) { }

/* Watchpoint handler
 *
 * Moira calls this function to signal that a watchpoint has been reached.
 */
void
TestCPU::watchpointReached(moira::u32 addr) { }
