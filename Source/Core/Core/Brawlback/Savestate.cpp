#include "Savestate.h"

#include <Common/MemoryUtil.h>
#include <Core/HW/EXI/EXI.h>
#include <Core/HW/Memmap.h>
#include <thread>
#include "Common/Thread.h"
#include "Common/Timer.h"
#include "VideoCommon/OnScreenDisplay.h"
#include "Common/Logging/Log.h"

#include "MemRegions.h"

#define LOW_BOUND_MEM 0x80000000

// lots of code here is heavily derived from Slippi's Savestates.cpp

BrawlbackSavestate::BrawlbackSavestate()
{
  // init member list with proper addresses
  initBackupLocs();
  // iterate through address ranges and allocate mem for our savestates
  for (auto it = backupLocs.begin(); it != backupLocs.end(); ++it)
  {
    auto size = it->endAddress - it->startAddress;
    it->data = static_cast<u8*>(Common::AllocateAlignedMemory(size, 64));
  }
}

BrawlbackSavestate::~BrawlbackSavestate()
{
  for (auto it = backupLocs.begin(); it != backupLocs.end(); ++it)
  {
    Common::FreeAlignedMemory(it->data);
  }
}

void BrawlbackSavestate::getDolphinState(PointerWrap& p)
{
  // p.DoArray(Memory::m_pRAM, Memory::RAM_SIZE);
  // p.DoMarker("Memory");
  // VideoInterface::DoState(p);
  // p.DoMarker("VideoInterface");
  // SerialInterface::DoState(p);
  // p.DoMarker("SerialInterface");
  // ProcessorInterface::DoState(p);
  // p.DoMarker("ProcessorInterface");
  // DSP::DoState(p);
  // p.DoMarker("DSP");
  // DVDInterface::DoState(p);
  // p.DoMarker("DVDInterface");
  // GPFifo::DoState(p);
  // p.DoMarker("GPFifo");
  ExpansionInterface::DoState(p);
  p.DoMarker("ExpansionInterface");
  // AudioInterface::DoState(p);
  // p.DoMarker("AudioInterface");
}

void BrawlbackSavestate::initBackupLocs()
{
    SlippiInitBackupLocations(this->backupLocs, memRegions->memRegions, memRegions->excludeSections);
}

typedef std::vector<SlippiUtility::Savestate::ssBackupLoc>::iterator backupLocIterator;

void captureMemRegions(backupLocIterator start, backupLocIterator end) {
    for (auto it = start; it != end; ++it) {
        auto size = it->endAddress - it->startAddress;
        Memory::CopyFromEmu(it->data, it->startAddress, size);  // game -> emu
    }
}

void BrawlbackSavestate::Capture()
{
    captureMemRegions(backupLocs.begin(), backupLocs.end());
    u64 totalsize = 0;
    for (auto& loc : backupLocs)
    {
        totalsize += loc.endAddress - loc.startAddress;
    }
    double dsize = ((double)totalsize / 1000000.0);
    INFO_LOG_FMT(BRAWLBACK, "backupLocs total size: {:.13} mb\n", dsize);
}

void BrawlbackSavestate::Load(std::vector<PreserveBlock> blocks)
{
    // Back up regions of game that should stay the same between savestates

    for (auto it = blocks.begin(); it != blocks.end(); ++it)
    {
        if (!preservationMap.count(*it))  // if this PreserveBlock is NOT in our preservationMap
        {
          // TODO: Clear preservation map when game ends
          preservationMap[*it] =
              std::vector<u8>(it->length);  // init new entry at this PreserveBlock key
        }

        Memory::CopyFromEmu(&preservationMap[*it][0], it->address, it->length);
    }

    // Restore memory blocks
    for (auto it = backupLocs.begin(); it != backupLocs.end(); ++it)
    {
        auto size = it->endAddress - it->startAddress;
        // if (it->endAddress < LOW_BOUND_MEM)
        //{
        //    Memory::CopyToEmu(it->startAddress, it->data, it->endAddress);  // emu -> game
        //}
        // else
        //{
        Memory::CopyToEmu(it->startAddress, it->data, size);  // emu -> game
                                                              //}
    }

    //// Restore audio
    // u8 *ptr = &dolphinSsBackup[0];
    // PointerWrap p(&ptr, PointerWrap::MODE_READ);
    // getDolphinState(p);

    // Restore preservation blocks
    for (auto it = blocks.begin(); it != blocks.end(); ++it)
    {
        Memory::CopyToEmu(it->address, &preservationMap[*it][0], it->length);
    }
  
}
