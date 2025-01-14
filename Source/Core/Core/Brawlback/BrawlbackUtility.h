#pragma once

#include <unordered_map>
#include <array>
#include <fstream>
#include <optional>
#include <random>
#include <algorithm>

#include "Common/FileUtil.h"
#include "Common/CommonTypes.h"
#include "Common/Timer.h"
#include "Common/Logging/Log.h"
#include "Common/Logging/LogManager.h"

#include "SlippiUtility.h"
#include "Brawltypes.h"
#include "Savestate.h"
#include "brawlback-common/ExiStructures.h"
#include "brawlback-common/BrawlbackConstants.h"

// ---
// mem dumping related
#include "Core/HW/AddressSpace.h"
#include "Common/FileUtil.h"
#include "Common/IOFile.h"
// ---

namespace Brawlback {

    struct UserInfo
    {
      std::string uid = "";
      std::string playKey = "";
      std::string displayName = "";
      std::string connectCode = "";
      std::string latestVersion = "";
      std::string fileContents = "";

      int port;
    };

    // TODO: put this in the submodule and pack it
    struct GameReport {
        double damage[MAX_NUM_PLAYERS];
        s32 stocks[MAX_NUM_PLAYERS];
        s32 frame_duration;
    };

    enum EXICommand : u8
    {
      CMD_UNKNOWN = 0,

      CMD_ONLINE_INPUTS = 1, // sending inputs from game to emulator
      CMD_CAPTURE_SAVESTATE = 2,
      CMD_LOAD_SAVESTATE = 3,

      CMD_FIND_OPPONENT = 5,
      CMD_START_MATCH = 13,
      CMD_SETUP_PLAYERS = 14,
      CMD_FRAMEDATA = 15, // game is requesting inputs for some frame
      CMD_TIMESYNC = 16,
      CMD_ROLLBACK = 17,
      CMD_FRAMEADVANCE = 18,

      CMD_SAVESTATE_REGION = 22,

      // REPLAYS
      CMD_REPLAY_START_REPLAYS_STRUCT = 19,
      CMD_REPLAY_REPLAYS_STRUCT = 20,
      CMD_REPLAYS_REPLAYS_END = 21,
      CMD_GET_NEXT_FRAME = 22,
      CMD_BAD_INDEX = 23,
      CMD_GET_NUM_REPLAYS = 24,
      CMD_SET_CUR_INDEX = 25,
      CMD_GET_START_REPLAY = 26,

      CMD_MATCH_END = 4,
      CMD_SET_MATCH_SELECTIONS = 6,

      CMD_TIMER_START = 7,
      CMD_TIMER_END = 8,
      CMD_UPDATE = 9,
      
      CMD_GET_ONLINE_STATUS = 10,
      CMD_CLEANUP_CONNECTION = 11,
      CMD_GET_NEW_SEED = 12,
    };

    enum NetPacketCommand : u8 
    {
        CMD_FRAME_DATA = 1,
        CMD_GAME_SETTINGS = 2,
        CMD_FRAME_DATA_ACK = 3,
    };

    struct FrameOffsetData {
        int idx;
        std::vector<s32> buf;
    };
    namespace Match
    {   
        bool isPlayerFrameDataEqual(const PlayerFrameData& p1, const PlayerFrameData& p2);
    }

    // https://mklimenko.github.io/english/2018/08/22/robust-endian-swap/
    template <typename T>
    T swap_endian(T val)
    {
      union U
      {
        T val;
        std::array<std::uint8_t, sizeof(T)> raw;
      } src, dst;

      src.val = val;
      std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
      val = dst.val;
      return val;
    }

    inline void SwapPlayerFrameDataEndianness(PlayerFrameData& pfd) {
        pfd.frame = swap_endian(pfd.frame);
        pfd.syncData.anim = swap_endian(pfd.syncData.anim);
        pfd.syncData.locX = swap_endian(pfd.syncData.locX);
        pfd.syncData.locY = swap_endian(pfd.syncData.locY);
        pfd.syncData.percent = swap_endian(pfd.syncData.percent);
        pfd.randomSeed = swap_endian(pfd.randomSeed);
    }
    inline void SwapFrameDataEndianness(FrameData& fd) {
        for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
            SwapPlayerFrameDataEndianness(fd.playerFrameDatas[i]);
        }
        swap_endian(fd.randomSeed);
    }

    inline void PrintSyncData(const SyncData& data) {
        INFO_LOG_FMT(
            BRAWLBACK,
            "xPos = {}  yPos = {}  facingDir = {}  anim = {}  percent = {}  stocks = {}\n",
            data.locX, data.locY, (s32)data.facingDir, data.anim, data.percent, (u32)data.stocks);
    }

    // checks if the specified `button` is held down based on the buttonBits bitfield
    bool isButtonPressed(u16 buttonBits, PADButtonBits button);
    namespace Mem {
        void print_byte(uint8_t byte);
        void print_half(u16 half);
        void print_word(u32 word);

        template <typename T>
        std::vector<u8> structToByteVector(T* s) {
            auto ptr = reinterpret_cast<u8*>(s);
            return std::vector<u8>(ptr, ptr + sizeof(T));
        }

        void fillByteVectorWithBuffer(std::vector<u8>& vec, u8* buf, size_t size);
    }
    namespace Sync {
        std::string getSyncLogFilePath();
        std::string str_byte(uint8_t byte);
        std::string str_half(u16 half);
        void SyncLog(const std::string& msg);
        std::string stringifyFramedata(const FrameData& fd, int numPlayers);
        std::string stringifyFramedata(const PlayerFrameData& pfd);
        std::string stringifyPad(const BrawlbackPad& pad);
    }
    
    typedef std::deque<std::unique_ptr<PlayerFrameData>> PlayerFrameDataQueue;

    PlayerFrameData* findInPlayerFrameDataQueue(const PlayerFrameDataQueue& queue,
                                                           u32 frame);

    inline bool isInputsEqual(const BrawlbackPad& p1, const BrawlbackPad& p2) {
        // TODO: this code is duplicated on the .cpp make it dry or I don't know
        bool buttons = p1.buttons == p2.buttons;
        bool holdButtons = p1.holdButtons == p2.holdButtons;
        bool rapidFireButtons = p1.rapidFireButtons == p2.rapidFireButtons;
        bool releasedButtons = p1.releasedButtons == p2.releasedButtons;
        bool newPressedButtons = p1.newPressedButtons == p2.newPressedButtons;
        bool triggers = p1.LTrigger == p2.LTrigger && p1.RTrigger == p2.RTrigger;
        bool analogSticks = p1.stickX == p2.stickX && p1.stickY == p2.stickY;
        bool cSticks = p1.cStickX == p2.cStickX && p1.cStickY == p2.cStickY;
        return buttons && holdButtons && rapidFireButtons && releasedButtons && newPressedButtons && analogSticks && cSticks && triggers;

        //return memcmp(&p1, &p2, sizeof(BrawlbackPad)) == 0;
    }

    inline PlayerFrameData generateRandomInput(s32 frame, u8 pIdx) {
        PlayerFrameData ret;
        ret.frame = frame;
        ret.playerIdx = pIdx;
        std::default_random_engine generator = std::default_random_engine((s32)Common::Timer::NowUs());
        ret.pad.buttons = (u16)((generator() % 65535));
        //ret.pad.stickX = (u8)(127-generator() % (127*2));
        ret.pad.stickX = 0;
        ret.pad.stickY = (u8)(127-generator() % (127*2));
        ret.pad.cStickX = (u8)(127-generator() % (127*2));
        ret.pad.cStickY = (u8)(127-generator() % (127*2));
        ret.pad.LTrigger = (u8)(127-generator() % (127*2));
        ret.pad.RTrigger = (u8)(127-generator() % (127*2));
        return ret;
    }

    template <typename T>
    T Clamp(T input, T Max, T Min) {
        return input > Max ? Max : ( input < Min ? Min : input );
    }


    template <typename T>
    inline T MAX(T x, T y) { return (((x) > (y)) ? (x) : (y)); }
    template <typename T>
    inline T MIN(T x, T y) { return (((x) < (y)) ? (x) : (y)); }
    // 1 if in range (inclusive), 0 otherwise
    template <typename T>
    inline T RANGE(T i, T min, T max) { return ((i < min) || (i > max) ? 0 : 1); }

    namespace Dump {
        void DoMemDumpIteration(int& dump_num);
        void DumpMem(AddressSpace::Type memType, const std::string& dumpPath);
    }

}
