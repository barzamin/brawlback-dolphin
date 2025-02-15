#pragma once

#include "SlippiUtility.h"

using namespace SlippiUtility::Savestate;

// thank you Slippi :)

class BrawlbackSavestate
{

public:


    BrawlbackSavestate();
    ~BrawlbackSavestate();


    void Capture();
    void Load(std::vector<PreserveBlock> blocks);

    //static bool shouldForceInit;

    std::vector<ssBackupLoc>* getBackupLocs() { return &backupLocs; }

    int frame = -1;
    int checksum = -1;
private:


    std::vector<ssBackupLoc> backupLocs = {};
    std::unordered_map<PreserveBlock, std::vector<u8>, preserve_hash_fn, preserve_eq_fn> preservationMap;
    std::vector<u8> dolphinSsBackup = {};

    void getDolphinState(PointerWrap& p);


    void initBackupLocs();

    //std::thread firstHalf;
    //std::thread secondHalf;


};
