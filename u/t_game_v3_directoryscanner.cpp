/**
  *  \file u/t_game_v3_directoryscanner.cpp
  *  \brief Test for game::v3::DirectoryScanner
  */

#include "game/v3/directoryscanner.hpp"

#include "t_game_v3.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"
#include "game/test/files.hpp"

using afl::base::Ref;
using afl::charset::CodepageCharset;
using afl::checksums::ByteSum;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;
using afl::io::InternalStream;
using afl::io::Stream;
using afl::string::NullTranslator;
using afl::sys::Log;
using game::HostVersion;
using game::PlayerSet_t;
using game::Timestamp;
using game::v3::DirectoryScanner;
using game::v3::TurnFile;

namespace {
    Ref<InternalDirectory> makeSpecificationDirectory()
    {
        const char*const MSGPARSE =
            "config,SRace PBP Message\n"
            "  kind   = c\n"
            "  check  = Priority Points\n"
            "  check  = Build Queue\n"
            "  check  = Srace 3\n"
            "  parse  = Host Version $\n"
            "  assign = HostVersion\n"
            "  value  = SRace\n"
            "  assign = HostType\n"
            "config,THost PBP Message\n"
            "  kind   = c\n"
            "  check  = Priority Points\n"
            "  check  = Build Queue\n"
            "  parse  = Host Version $\n"
            "  assign = HostVersion\n"
            "  value  = Host\n"
            "  assign = HostType\n"
            "config,PHost Version Message\n"
            "  kind   = h\n"
            "  check  = HUL=\n"
            "  check  = PXY=\n"
            "  parse  = =1,PHost $\n"
            "  assign = HostVersion\n"
            "  value  = PHost\n"
            "  assign = HostType\n";

        Ref<InternalDirectory> specDir = InternalDirectory::create("spec");
        specDir->addStream("hostver.ini", *new ConstMemoryStream(afl::string::toBytes(MSGPARSE)));
        return specDir;
    }


    struct Environment {
        Ref<InternalDirectory> specDir;
        Ref<InternalDirectory> workDir;
        NullTranslator tx;
        Log log;
        CodepageCharset charset;
        DirectoryScanner scanner;

        Environment()
            : specDir(makeSpecificationDirectory()),
              workDir(InternalDirectory::create("work")),
              tx(), log(),
              charset(afl::charset::g_codepage437),
              scanner(*specDir, tx, log)
            { }
    };

    void writeLong(Stream& out, uint32_t value)
    {
        game::v3::structures::UInt32_t v;
        v = value;
        out.fullWrite(afl::base::fromObject(v));
    }

    void writeWord(Stream& out, uint16_t value)
    {
        game::v3::structures::UInt16_t v;
        v = value;
        out.fullWrite(afl::base::fromObject(v));
    }

    void addResult(Environment& env, String_t fileName, int16_t playerId, int16_t turnNr, Timestamp ts)
    {
        /*
         *     +0     8 longs  section addresses
         *    +32       word   empty ship section
         *    +34       word   empty target section
         *    +36       word   empty pdata section
         *    +38       word   empty bdata section
         *    +40       word   empty msg section
         *    +42  4000 bytes  shipxy section
         *  +4042   144 bytes  gen section
         *  +4186       word   vcr section      -> 4188 bytes
         */
        Ref<InternalStream> rst = *new InternalStream();

        // Header
        writeLong(*rst, 33);
        writeLong(*rst, 35);
        writeLong(*rst, 37);
        writeLong(*rst, 39);
        writeLong(*rst, 41);
        writeLong(*rst, 43);
        writeLong(*rst, 4043);
        writeLong(*rst, 4187);

        // First 5 sections
        for (int i = 0; i < 5; ++i) {
            writeWord(*rst, 0);
        }

        // shipxy section
        for (int i = 0; i < 1000; ++i) {
            writeLong(*rst, 0);
        }

        // gen section
        game::v3::structures::ResultGen gen;
        afl::base::fromObject(gen).fill(0);
        ts.storeRawData(gen.timestamp);
        gen.playerId = playerId;
        gen.turnNumber = turnNr;
        gen.timestampChecksum = static_cast<int16_t>(ByteSum().add(gen.timestamp, 0));
        rst->fullWrite(afl::base::fromObject(gen));

        // vcr section
        writeWord(*rst, 0);

        // finish
        rst->setPos(0);
        env.workDir->addStream(fileName, rst);
    }

    void addTurn(Environment& env, String_t fileName, int16_t playerId, int16_t /*turnNr*/, Timestamp ts)
    {
        TurnFile trn(env.charset, playerId, ts);
        trn.setFeatures(TurnFile::FeatureSet_t());
        trn.addCommand(game::v3::tcm_PlanetBuildBase, 444);
        trn.update();

        Ref<InternalStream> file = *new InternalStream();
        trn.write(*file);
        file->setPos(0);
        env.workDir->addStream(fileName, file);
    }

    void addGen(Environment& env, String_t fileName, int16_t playerId, int16_t turnNr, Timestamp ts)
    {
        game::v3::structures::Gen gen;
        afl::base::fromObject(gen).fill(0);
        ts.storeRawData(gen.timestamp);
        gen.playerId = playerId;
        gen.turnNumber = turnNr;
        gen.timestampChecksum = static_cast<int16_t>(ByteSum().add(gen.timestamp, 0));

        Ref<InternalStream> file = *new InternalStream();
        file->fullWrite(afl::base::fromObject(gen));
        file->setPos(0);
        env.workDir->addStream(fileName, file);
    }

    void addMessage(Environment& env, String_t fileName, String_t msgText)
    {
        /*
         *  +0     word    number of messages
         *  +2     long    address of message #1
         *  +6     word    length of message #1
         *  +8   n bytes   message
         */
        Ref<InternalStream> file = *new InternalStream();
        writeWord(*file, 1);
        writeLong(*file, 9);
        writeWord(*file, int16_t(msgText.size()));
        for (size_t i = 0; i < msgText.size(); ++i) {
            uint8_t tmp[1] = { uint8_t(msgText[i] == '\n' ? 26 : msgText[i]+13) };
            file->fullWrite(tmp);
        }

        file->setPos(0);
        env.workDir->addStream(fileName, file);
    }
}

/** Test empty directory (base case).
    Verify that output is produced correctly. */
void
TestGameV3DirectoryScanner::testEmpty()
{
    // Environment
    Environment env;

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Everything empty
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(1), DirectoryScanner::PlayerFlags_t());
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t());
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t());
    TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getKind(), HostVersion::Unknown);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);

    // Out-of-bounds
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(20), DirectoryScanner::PlayerFlags_t());
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(2000), DirectoryScanner::PlayerFlags_t());
}

/** Test single RST file.
    File is reported correctly. */
void
TestGameV3DirectoryScanner::testResult()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(1), DirectoryScanner::PlayerFlags_t());
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(5), DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult));
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult));
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));

    // Host version not found out (no messages)
    TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getKind(), HostVersion::Unknown);

    // Default player is known
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test multiple RST files.
    Files are reported correctly. */
void
TestGameV3DirectoryScanner::testMultiResult()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player7.rst", 7, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player9.rst", 9, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Must have found multiple results, but no default player
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t() + 5 + 7 + 9);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test multiple RST files with different timestamps.
    Files are reported correctly, old results are marked conflicting. */
void
TestGameV3DirectoryScanner::testNewResult()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player7.rst", 7, 31, Timestamp(2001, 12, 10, 1, 1, 1));
    addResult(env, "player9.rst", 9, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Must have found multiple results, a conflict, and no default player
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult + DirectoryScanner::HaveConflict);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t() + 5 + 7 + 9);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveConflict)), PlayerSet_t() + 5 + 9);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test broken RST files (wrong owner).
    Broken files are ignored. */
void
TestGameV3DirectoryScanner::testBrokenResult()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player7.rst", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));  // will be ignored due to mismatch

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Must have found one result
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test broken RST files (truncated).
    Broken files are ignored. */
void
TestGameV3DirectoryScanner::testTruncatedResult()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    env.workDir->addStream("player7.rst", *new ConstMemoryStream(afl::string::toBytes("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"))); // will be ignored due to format error

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Must have found one result
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test single GEN file.
    File is reported correctly. */
void
TestGameV3DirectoryScanner::testGen()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(1), DirectoryScanner::PlayerFlags_t());
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(4), DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked));
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked));
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t(4));

    // Host version not found out (no messages)
    TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getKind(), HostVersion::Unknown);

    // Default player is known
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 4);
}

/** Test multiple GEN files.
    Files are reported correctly. */
void
TestGameV3DirectoryScanner::testMultiGen()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5 + 6);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test conflicting GEN files.
    Files are reported correctly, old files are marked conflicting. */
void
TestGameV3DirectoryScanner::testConflictGen()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 31, Timestamp(2001, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked + DirectoryScanner::HaveConflict);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5 + 6);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveConflict)), PlayerSet_t() + 4 + 5);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test broken GEN files (wrong owner).
    Broken files are ignored. */
void
TestGameV3DirectoryScanner::testBadGen()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 7, 30, Timestamp(2000, 12, 10, 1, 1, 1)); // will be ignored

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5);
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test GEN files and new result.
    New result is reported as such. */
void
TestGameV3DirectoryScanner::testGenAndNewResult()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player5.rst", 5, 31, Timestamp(2001, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked + DirectoryScanner::HaveNewResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5 + 6);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveNewResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test GEN files and same-turn result.
    Result is reported as such. */
void
TestGameV3DirectoryScanner::testGenAndSameResult()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5 + 6);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test GEN files and old result.
    Old result is reported as "Other". */
void
TestGameV3DirectoryScanner::testGenAndOldResult()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player5.rst", 5, 29, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveUnpacked + DirectoryScanner::HaveOtherResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveUnpacked)), PlayerSet_t() + 4 + 5 + 6);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveOtherResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 0);
}

/** Test GEN and RST files in "only result" mode.
    Only the RST is reported. */
void
TestGameV3DirectoryScanner::testGenOnlyResult()
{
    // Environment
    Environment env;
    addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen5.dat", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addGen(env, "gen6.dat", 6, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset, true);

    // Check flags
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test RST and matching TRN file.
    Files are reported correctly. */
void
TestGameV3DirectoryScanner::testResultAndTurn()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addTurn(env, "player5.trn", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(5), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult + DirectoryScanner::HaveTurn);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult + DirectoryScanner::HaveTurn);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveTurn)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test RST and mismatching TRN file.
    TRN is ignored. */
void
TestGameV3DirectoryScanner::testResultAndMismatchingTurn()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addTurn(env, "player5.trn", 5, 30, Timestamp(2001, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(5), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test RST and broken TRN file (wrong owner).
    TRN is ignored. */
void
TestGameV3DirectoryScanner::testResultAndWrongOwnerTurn()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    addTurn(env, "player5.trn", 7, 30, Timestamp(2000, 12, 10, 1, 1, 1));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(5), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test RST and broken TRN file (bad format).
    TRN is ignored. */
void
TestGameV3DirectoryScanner::testResultAndBadTurn()
{
    // Environment
    Environment env;
    addResult(env, "player5.rst", 5, 30, Timestamp(2000, 12, 10, 1, 1, 1));
    env.workDir->addStream("player5.trn", *new ConstMemoryStream(afl::string::toBytes("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"))); // will be ignored due to format error

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(5), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(5));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 5);
}

/** Test host version parsing.
    For simplicity, generate MDATAx.DAT files with known-good messages. */
void
TestGameV3DirectoryScanner::testHostVersion()
{
    struct TestCase {
        const char* msg;
        HostVersion::Kind kind;
        int32_t version;
    };

    static const TestCase CASES[] = {
        // Real test cases
        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.22.020\n"
          "Compiled: Nov 26, 1997\n",
          HostVersion::Host,
          MKVERSION(3, 22, 20) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.20\n",
          HostVersion::Host,
          MKVERSION(3, 20, 0) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "Fed   :         0\n"
          "Colonial   :    0\n"
          "HOST Version 3.22.020.SRACE.012\n"
          "Srace 3.22.020.012\n"
          "Compiled: Jan 4, 1998\n",
          HostVersion::SRace,
          MKVERSION(3, 22, 20) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.22.025b \n"
          "Compiled: Nov 21, 1998\n",
          HostVersion::Host,
          MKVERSION(3, 22, 25) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.2-Beta Test 3g\n",
          HostVersion::Host,
          MKVERSION(3, 20, 0) },

        { "(-h000) PHOST v3.4l\n"
          "HUL=DFC40C02\n"
          "PXY=6D3FCA8E\n"
          "\n"
          "The above information is for use by\n"
          "external player utilities and can\n"
          "be safely ignored.\n",
          HostVersion::PHost,
          MKVERSION(3, 4, 12) },

        { "(-h000)<<< PHOST v4.0  >>>\n"
          "\n"
          "Die nachfolgenden Informationen\n"
          "sind fuer externe Programme und\n"
          "haben keine Bedeutung fuer Dich.\n"
          "\n"
          "HUL=2BA33201\n"
          "ENG=40394EDE\n"
          "BEA=2949E405\n"
          "TOR=CF636FBC\n"
          "TRU=C884F8C0\n"
          "PXY=F4EE5310\n"
          "CFG=8A5DFEDB\n"
          "NAM=E2914F5A\n",
          HostVersion::PHost,
          MKVERSION(4, 0, 0) },

        // Those are not real, but could be:
        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.20c\n",
          HostVersion::Host,
          MKVERSION(3, 20, 3) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 3.15b\n",
          HostVersion::Host,
          MKVERSION(3, 15, 2) },

        { "(-h000) PHOST v2.7 \n"
          "HUL=DFC40C02\n"
          "PXY=6D3FCA8E\n"
          "\n"
          "The above information is for use by\n"
          "external player utilities and can\n"
          "be safely ignored.\n",
          HostVersion::PHost,
          MKVERSION(2, 7, 0) },

        // The following are entirely fake:
        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version Unknown\n",
          HostVersion::Host,
          0 },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 5\n",
          HostVersion::Host,
          MKVERSION(5, 0, 0) },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version 5.\n",           // invalid, minor number expected after '.'
          HostVersion::Host,
          0 },

        { "(-c0000)<<< Priority Points >>>\n"
          "(For Ship Build Queue)\n"
          "HOST Version -1\n",
          HostVersion::Host,
          0 },
    };

    for (size_t i = 0; i < sizeof(CASES)/sizeof(CASES[0]); ++i) {
        // Environment
        Environment env;
        addGen(env, "gen4.dat", 4, 30, Timestamp(2000, 12, 10, 1, 1, 1));
        addMessage(env, "mdata4.dat", CASES[i].msg);

        // Scan!
        env.scanner.scan(*env.workDir, env.charset);

        // Verify
        TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getKind(), CASES[i].kind);
        TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getVersion(), CASES[i].version);
    }
}

/** Test host version parsing from result file. */
void
TestGameV3DirectoryScanner::testHostVersionResult()
{
    // Environment
    Environment env;
    env.workDir->addStream("player7.rst", *new ConstMemoryStream(game::test::getResultFile30()));

    // Scan!
    env.scanner.scan(*env.workDir, env.charset);

    // Check player flags
    TS_ASSERT_EQUALS(env.scanner.getPlayerFlags(7), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryFlags(), DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult);
    TS_ASSERT_EQUALS(env.scanner.getPlayersWhere(DirectoryScanner::PlayerFlags_t(DirectoryScanner::HaveResult)), PlayerSet_t(7));
    TS_ASSERT_EQUALS(env.scanner.getDefaultPlayer(), 7);

    // Check host version
    TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getKind(), HostVersion::PHost);
    TS_ASSERT_EQUALS(env.scanner.getDirectoryHostVersion().getVersion(), MKVERSION(4, 1, 8));
}
