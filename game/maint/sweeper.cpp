/**
  *  \file game/maint/sweeper.cpp
  *  \brief Class game::maint::Sweeper
  *
  *  PCC2 Comment:
  *
  *  The file lists are based upon CCSWEEP 1.06 shipped with PCC 1.x.
  *  Comments show when a file was added to the list, plus an explanation
  *  of what the file is for if known.
  *
  *  Version History of CCSWEEP:
  *  - CCSWEEP 1.06 (20020128)
  *  - CCSWEEP 1.05 (20010326)
  *  - CCSWEEP 1.04(2) (20001001), names itself 1.04, knows a file configX.dat(?)
  *  - CCSWEEP 1.04 (19990924)
  *  - CCSWEEP 1.03 (19990922)
  *  - CCSWEEP 1.01 (19990531)
  *  - CCSWEEP 1.0 (19980412)
  *  - SWEEP 1.0 (19971220)
  */

#include <cassert>
#include "game/maint/sweeper.hpp"
#include "game/v3/structures.hpp"
#include "afl/string/format.hpp"
#include "afl/io/stream.hpp"
#include "afl/base/countof.hpp"

// Constructor.
game::maint::Sweeper::Sweeper()
    : m_eraseDatabaseFlag(false),
      m_didScan(false),
      m_remainingPlayers(),
      m_selectedPlayers()
{
    // ex GSweepControl::GSweepControl
}

// Scan game directory.
void
game::maint::Sweeper::scan(afl::io::Directory& dir)
{
    // ex GSweepControl::scan
    m_remainingPlayers.clear();
    for (int i = 1; i <= game::v3::structures::NUM_PLAYERS; ++i) {
        if (dir.openFileNT(afl::string::Format("gen%d.dat", i), afl::io::FileSystem::OpenRead).get() != 0) {
            m_remainingPlayers += i;
        }
    }
    m_didScan = true;
}

// Execute operation.
void
game::maint::Sweeper::execute(afl::io::Directory& dir)
{
    // ex GSweepControl::processFiles

    // \change in c2ng, execute() implies updateIndex().
    // In PCC2, processFiles() and processIndex() were separate.

    // Scan for players
    if (!m_didScan) {
        scan(dir);
    }

    // Delete configured player files
    for (int i = 1; i <= game::v3::structures::NUM_PLAYERS; ++i) {
        if (m_selectedPlayers.contains(i)) {
            processPlayerFiles(dir, i);
            m_remainingPlayers -= i;
        }
    }

    // Delete standard temporary files left over by various clients
    dir.eraseNT("temp.pln");
    dir.eraseNT("temp.bmp");
    dir.eraseNT("temp.dat");
    dir.eraseNT("path.dat");
    dir.eraseNT("templock.dat");
    dir.eraseNT("vpa.bak");
    dir.eraseNT("vpa.tmp");
    dir.eraseNT("vpaaddon.bak");
    dir.eraseNT("vpaaddon.tmp");
    dir.eraseNT("vpacm.bak");
    dir.eraseNT("vpacm.tmp");

    // If no players remain, delete some more files
    if (m_remainingPlayers.empty()) {
        dir.eraseNT("control.dat");      // CCSweep 1.0
        dir.eraseNT("init.tmp");         // CCSweep 1.0
        if (m_eraseDatabaseFlag) {
            dir.eraseNT("stat.cc");      // CCSweep 1.0 (PCC)
            dir.eraseNT("score.cc");     // PCC2
            dir.eraseNT("config.cc");    // CCSweep 1.0 (PCC <1.0.17)
            dir.eraseNT("config2.cc");   // PCC >1.0.17
            dir.eraseNT("hconfig.hst");  // CCSweep 1.03+
            dir.eraseNT("rn.dat");       // CCSweep 1.03+ (Winplan race name config)
            dir.eraseNT("pconfig.src");  // CCSweep 1.04+
            dir.eraseNT("hconfig.ini");  // created by VPA
            dir.eraseNT("map.ini");      // created by VPA
            dir.eraseNT("races.ini");    // created by VPA
        }
    }

    updateIndex(dir);
}

// Configuration: erase database flag.
void
game::maint::Sweeper::setEraseDatabase(bool flag)
{
    // ex GSweepControl::setEraseDatabase
    m_eraseDatabaseFlag = flag;
}

// Configuration: set selected players.
void
game::maint::Sweeper::setPlayers(PlayerSet_t set)
{
    // ex GSweepControl::setPlayers
    m_selectedPlayers = set;
}

// Get selected players.
game::PlayerSet_t
game::maint::Sweeper::getPlayers() const
{
    // ex GSweepControl::getPlayers
    return m_selectedPlayers;
}

// Get remaining players.
game::PlayerSet_t
game::maint::Sweeper::getRemainingPlayers() const
{
    // c2ng change: does no longer imply scan()
    // ex GSweepControl::getRemainingPlayers
    return m_remainingPlayers;
}

/** Process one player's files.
    \param dir Directory
    \param player Player */
void
game::maint::Sweeper::processPlayerFiles(afl::io::Directory& dir, int player)
{
    // ex GSweepControl::processPlayerFiles
    static const char* const TURN_FILES[] = {
        "bdata%d.dat",          // CCSweep 1.0
        "bdata%d.dis",          // CCSweep 1.0
        "gen%d.dat",            // CCSweep 1.0
        "mdata%d.dat",          // CCSweep 1.0
        "pdata%d.dat",          // CCSweep 1.0
        "pdata%d.dis",          // CCSweep 1.0
        "ship%d.dat",           // CCSweep 1.0
        "ship%d.dis",           // CCSweep 1.0
        "shipxy%d.dat",         // CCSweep 1.0
        "target%d.dat",         // CCSweep 1.0
        "target%d.ext",         // CCSweep 1.01+ (VPUnpack etc.)
        "vcr%d.dat",            // CCSweep 1.0
        "obj%d.cc",             // CCSweep 1.0+ (PCC <1.0)
        "contrl%d.dat",         // CCSweep 1.0 (Winplan)
        "mess35%d.dat",         // CCSweep 1.0 (Winplan)
        "mess%d.dat",           // CCSweep 1.0
        "mt%d.txt",             // CCSweep 1.0+ (Winplan maketurn log)
        "kore%d.dat",           // CCSweep 1.01+
        "skore%d.dat",          // CCSweep 1.01+
        "cp%d.cc",              // CCSweep 1.04(2)+ (PCC <1.0.17)
        "fat%d.trn",            // CCSweep 1.04(2)+ (Winplan Maketurn temp file)
        "temp%d.trn",           // CCSweep 1.04(2)+ (Winplan Maketurn temp file)
        "vcr%d.tmp",            // created by VPA and probably others
    };
    static const char* const DB_FILES[] = {
        "chart%d.cc",           // CCSweep 1.0 (PCC)
        "mess%d.cc",            // CCSweep 1.0 (PCC <1.1.5)
        "fleet%d.cc",           // CCSweep 1.01+ (PCC)
        "team%d.cc",            // CCSweep 1.03+ (PCC)
        "auto%d.dat",           // CCSweep 1.0 (Winplan)
        "notes%d.dat",          // CCSweep 1.0 (Winplan)
        "pref%d.dat",           // CCSweep 1.0 (Winplan)
        "task%d.dat",           // CCSweep 1.0 (Winplan)
        "config%d.dat",         // CCSweep 1.0 (Winplan?)
        "vm%d.cc",              // CCSweep 1.05+ (PCC)
        "cmd%d.txt",            // CCSweep 1.06+ (PCC, VPA)
        "vpa%d.db",             // CCSweep 1.06+ (VPA)
        "msg%d.ini",            // PCC, VPA
        "vpaclr%d.ini",         // VPA
        "vpadat%d.ini",         // VPA
        "vpamsg%d.dat",         // VPA
        "vpanot%d.dat",         // VPA
        "vpascr%d.ini",         // VPA
        "vpasnb%d.dat",         // VPA
        "vpasnm%d.dat",         // VPA
        "vpasta%d.dat",         // VPA
    };

    for (size_t i = 0; i < countof(TURN_FILES); ++i) {
        dir.eraseNT(afl::string::Format(TURN_FILES[i], player));
    }

    if (m_eraseDatabaseFlag) {
        for (size_t i = 0; i < countof(DB_FILES); ++i) {
            dir.eraseNT(afl::string::Format(DB_FILES[i], player));
        }
    }
}

/** Update index file.
    This rewrites the init.tmp index file required by planets.exe and Winplan.
    Unlike CCSweep 1.x, we rebuild the file from scratch.

    \pre m_didScan */
void
game::maint::Sweeper::updateIndex(afl::io::Directory& dir)
{
    // ex GSweepControl::processIndex
    using game::v3::structures::NUM_PLAYERS;

    // Only write the file if it wouldn't be empty. If it would be empty,
    // processFiles will have deleted it.
    if (!m_remainingPlayers.empty()) {
        // Build new index. Since it's simple, we build it directly instead of
        // using the normal marshalling functions.
        uint8_t newIndex[2*NUM_PLAYERS];
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            newIndex[2*(i-1)]   = (m_remainingPlayers.contains(i));
            newIndex[2*(i-1)+1] = 0;
        }

        // Write it
        afl::base::Ref<afl::io::Stream> file = dir.openFile("init.tmp", afl::io::FileSystem::Create);
        file->fullWrite(newIndex);
    }
}
