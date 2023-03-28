/**
  *  \file u/t_game_interface_globalproperty.cpp
  *  \brief Test for game::interface::GlobalProperty
  */

#include <stdexcept>
#include "game/interface/globalproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "version.hpp"

using game::HostVersion;
using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** Test behaviour with fully-populated session.
    All optional objects are present. */
void
TestGameInterfaceGlobalProperty::testIt()
{
    // Key
    class Key : public game::RegistrationKey {
     public:
        virtual Status getStatus() const
            { return Registered; }                                               // igpRegSharewareXXX
        virtual String_t getLine(Line which) const
            {
                switch (which) {
                 case Line1: return "one";                                       // igpRegStr1
                 case Line2: return "two";                                       // igpRegStr2
                 case Line3: return "three";
                 case Line4: return "four";
                }
                return "?";
            }
        virtual bool setLine(Line /*which*/, String_t /*value*/)
            { return false; }
        virtual int getMaxTechLevel(game::TechLevel /*area*/) const
            { return 10; }
    };

    // TurnLoader
    class Loader : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { throw std::runtime_error("unexpected: getPlayerStatus"); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { throw std::runtime_error("unexpected: loadCurrentTurn"); }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { throw std::runtime_error("unexpected: saveCurrentTurn"); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { throw std::runtime_error("unexpected: getHistoryStatus"); }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { throw std::runtime_error("unexpected: loadHistoryTurn"); }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, std::auto_ptr<game::Task_t> /*then*/)
            { throw std::runtime_error("unexpected: saveConfiguration"); }
        virtual String_t getProperty(Property p)
            {
                switch (p) {
                 case LocalFileFormatProperty:
                    return "lfmt";                                               // igpFileFormatLocal
                 case RemoteFileFormatProperty:
                    return "rfmt";                                               // igpFileFormatRemote
                 case RootDirectoryProperty:
                    return "/home/root";                                         // igpRootDirectory
                }
                return "";
            }
    };

    // Directory
    class Dir : public afl::io::Directory {
     public:
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t /*name*/)
            { throw std::runtime_error("unexpected: getDirectoryEntryByName"); }
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries()
            { throw std::runtime_error("unexpected: getDirectoryEntries"); }
        virtual afl::base::Ptr<afl::io::Directory> getParentDirectory()
            { return 0; }
        virtual String_t getDirectoryName()
            { return "/home/gamedir"; }                                          // igpGameDirectory
        virtual String_t getTitle()
            { return "gamedir"; }
    };

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.rng().setSeed(77);                                                   // igpRandomSeed

    // VCRs
    afl::base::Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
    for (int i = 0; i < 5; ++i) {
        db->addBattle();                                                         // igpMyVCRs
    }

    // Game
    const int PLAYER_NR = 4;
    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(PLAYER_NR);
    g->currentTurn().setTurnNumber(42);                                          // igpTurnNumber
    g->currentTurn().setDatabaseTurnNumber(41);                                  // igpTurnIsNew
    g->currentTurn().setTimestamp(game::Timestamp(2022, 12, 24, 13, 20, 15));    // igpTurnDate/Time
    for (int i = 0; i < 7; ++i) {
        g->currentTurn().inbox().addMessage("msg...", 42);                       // igpMyInMsgs
    }
    for (int i = 0; i < 11; ++i) {
        g->currentTurn().outbox().addMessage(1, "msg...", game::PlayerSet_t(1)); // igpMyOutMsgs
    }
    g->currentTurn().setBattles(db);
    g->selections().setCurrentLayer(3, g->currentTurn().universe());             // igpSelectionLayer
    session.setGame(g);

    game::v3::GenExtra::create(g->currentTurn()).create(PLAYER_NR).setPassword("xyz"); // igpSystemHasPassword

    // Root
    HostVersion host(HostVersion::PHost, MKVERSION(4,1,2));                      // igpSystemHostXXX
    afl::base::Ptr<game::Root> r = new game::Root(*new Dir(), *new game::test::SpecificationLoader(), host,
                                                  std::auto_ptr<game::RegistrationKey>(new Key()),
                                                  std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                                  std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                                  game::Root::Actions_t());
    r->setTurnLoader(new Loader());
    session.setRoot(r);

    // Verify
    verifyNewString ("igpFileFormatLocal",   getGlobalProperty(game::interface::igpFileFormatLocal,   session), "lfmt");
    verifyNewString ("igpFileFormatRemote",  getGlobalProperty(game::interface::igpFileFormatRemote,  session), "rfmt");
    verifyNewString ("igpGameDirectory",     getGlobalProperty(game::interface::igpGameDirectory,     session), "/home/gamedir");
    verifyNewInteger("igpMyInMsgs",          getGlobalProperty(game::interface::igpMyInMsgs,          session), 7);
    verifyNewInteger("igpMyOutMsgs",         getGlobalProperty(game::interface::igpMyOutMsgs,         session), 11);
    verifyNewInteger("igpMyVCRs",            getGlobalProperty(game::interface::igpMyVCRs,            session), 5);
    verifyNewString ("igpRootDirectory",     getGlobalProperty(game::interface::igpRootDirectory,     session), "/home/root");
    verifyNewInteger("igpSelectionLayer",    getGlobalProperty(game::interface::igpSelectionLayer,    session), 3);
    verifyNewString ("igpSystemLanguage",    getGlobalProperty(game::interface::igpSystemLanguage,    session), "en");
    verifyNewString ("igpSystemProgram",     getGlobalProperty(game::interface::igpSystemProgram,     session), "PCC");
    verifyNewString ("igpSystemVersion",     getGlobalProperty(game::interface::igpSystemVersion,     session), PCC2_VERSION);
    verifyNewInteger("igpSystemVersionCode", getGlobalProperty(game::interface::igpSystemVersionCode, session), PCC2_VERSION_CODE);
    verifyNewBoolean("igpSystemHasPassword", getGlobalProperty(game::interface::igpSystemHasPassword, session), true);
    verifyNewString ("igpSystemHost",        getGlobalProperty(game::interface::igpSystemHost,        session), "PHost");
    verifyNewInteger("igpSystemHostCode",    getGlobalProperty(game::interface::igpSystemHostCode,    session), 2);
    verifyNewInteger("igpSystemHostVersion", getGlobalProperty(game::interface::igpSystemHostVersion, session), 401002);
    verifyNewInteger("igpRandomSeed",        getGlobalProperty(game::interface::igpRandomSeed,        session), 77);
    verifyNewBoolean("igpRegSharewareFlag",  getGlobalProperty(game::interface::igpRegSharewareFlag,  session), false);
    verifyNewString ("igpRegSharewareText",  getGlobalProperty(game::interface::igpRegSharewareText,  session), "Registered");
    verifyNewString ("igpRegStr1",           getGlobalProperty(game::interface::igpRegStr1,           session), "one");
    verifyNewString ("igpRegStr2",           getGlobalProperty(game::interface::igpRegStr2,           session), "two");
    verifyNewInteger("igpTurnNumber",        getGlobalProperty(game::interface::igpTurnNumber,        session), 42);
    verifyNewString ("igpTurnDate",          getGlobalProperty(game::interface::igpTurnDate,          session), "12-24-2022");
    verifyNewBoolean("igpTurnIsNew",         getGlobalProperty(game::interface::igpTurnIsNew,         session), true);
    verifyNewString ("igpTurnTime",          getGlobalProperty(game::interface::igpTurnTime,          session), "13:20:15");
}

/** Test behaviour with half-populated session.
    A game is loaded, but optional objects are not present. */
void
TestGameInterfaceGlobalProperty::testHalf()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.rng().setSeed(77);                                                   // igpRandomSeed

    // Game
    const int PLAYER_NR = 4;
    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(PLAYER_NR);
    g->currentTurn().setTurnNumber(42);                                          // igpTurnNumber
    g->currentTurn().setDatabaseTurnNumber(42);                                  // igpTurnIsNew
    g->selections().setCurrentLayer(3, g->currentTurn().universe());             // igpSelectionLayer
    session.setGame(g);

    // Root
    afl::base::Ptr<game::Root> r = game::test::makeRoot(HostVersion(HostVersion::Host, MKVERSION(3,22,48))).asPtr();
    session.setRoot(r);

    // Verify
    verifyNewNull   ("igpFileFormatLocal",   getGlobalProperty(game::interface::igpFileFormatLocal,   session));
    verifyNewNull   ("igpFileFormatRemote",  getGlobalProperty(game::interface::igpFileFormatRemote,  session));
    verifyNewNull   ("igpGameDirectory",     getGlobalProperty(game::interface::igpGameDirectory,     session));
    verifyNewInteger("igpMyInMsgs",          getGlobalProperty(game::interface::igpMyInMsgs,          session), 0);
    verifyNewInteger("igpMyOutMsgs",         getGlobalProperty(game::interface::igpMyOutMsgs,         session), 0);
    verifyNewInteger("igpMyVCRs",            getGlobalProperty(game::interface::igpMyVCRs,            session), 0);
    verifyNewNull   ("igpRootDirectory",     getGlobalProperty(game::interface::igpRootDirectory,     session));
    verifyNewInteger("igpSelectionLayer",    getGlobalProperty(game::interface::igpSelectionLayer,    session), 3);
    verifyNewString ("igpSystemLanguage",    getGlobalProperty(game::interface::igpSystemLanguage,    session), "en");
    verifyNewString ("igpSystemProgram",     getGlobalProperty(game::interface::igpSystemProgram,     session), "PCC");
    verifyNewString ("igpSystemVersion",     getGlobalProperty(game::interface::igpSystemVersion,     session), PCC2_VERSION);
    verifyNewInteger("igpSystemVersionCode", getGlobalProperty(game::interface::igpSystemVersionCode, session), PCC2_VERSION_CODE);
    verifyNewNull   ("igpSystemHasPassword", getGlobalProperty(game::interface::igpSystemHasPassword, session));
    verifyNewString ("igpSystemHost",        getGlobalProperty(game::interface::igpSystemHost,        session), "Host");
    verifyNewInteger("igpSystemHostCode",    getGlobalProperty(game::interface::igpSystemHostCode,    session), 0);
    verifyNewInteger("igpSystemHostVersion", getGlobalProperty(game::interface::igpSystemHostVersion, session), 322048);
    verifyNewInteger("igpRandomSeed",        getGlobalProperty(game::interface::igpRandomSeed,        session), 77);
    verifyNewBoolean("igpRegSharewareFlag",  getGlobalProperty(game::interface::igpRegSharewareFlag,  session), true);
    verifyNewString ("igpRegSharewareText",  getGlobalProperty(game::interface::igpRegSharewareText,  session), "Shareware");
    verifyNewString ("igpRegStr1",           getGlobalProperty(game::interface::igpRegStr1,           session), "<Test>");
    verifyNewString ("igpRegStr2",           getGlobalProperty(game::interface::igpRegStr2,           session), "<Test>");
    verifyNewInteger("igpTurnNumber",        getGlobalProperty(game::interface::igpTurnNumber,        session), 42);
    verifyNewNull   ("igpTurnDate",          getGlobalProperty(game::interface::igpTurnDate,          session));
    verifyNewBoolean("igpTurnIsNew",         getGlobalProperty(game::interface::igpTurnIsNew,         session), false);
    verifyNewNull   ("igpTurnTime",          getGlobalProperty(game::interface::igpTurnTime,          session));
}

/** Test behaviour with empty session.
    No game loaded, so most properties are not present. */
void
TestGameInterfaceGlobalProperty::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.rng().setSeed(42);

    verifyNewNull   ("igpFileFormatLocal",   getGlobalProperty(game::interface::igpFileFormatLocal,   session));
    verifyNewNull   ("igpFileFormatRemote",  getGlobalProperty(game::interface::igpFileFormatRemote,  session));
    verifyNewNull   ("igpGameDirectory",     getGlobalProperty(game::interface::igpGameDirectory,     session));
    verifyNewNull   ("igpMyInMsgs",          getGlobalProperty(game::interface::igpMyInMsgs,          session));
    verifyNewNull   ("igpMyOutMsgs",         getGlobalProperty(game::interface::igpMyOutMsgs,         session));
    verifyNewNull   ("igpMyVCRs",            getGlobalProperty(game::interface::igpMyVCRs,            session));
    verifyNewNull   ("igpRootDirectory",     getGlobalProperty(game::interface::igpRootDirectory,     session));
    verifyNewNull   ("igpSelectionLayer",    getGlobalProperty(game::interface::igpSelectionLayer,    session));
    verifyNewString ("igpSystemLanguage",    getGlobalProperty(game::interface::igpSystemLanguage,    session), "en");
    verifyNewString ("igpSystemProgram",     getGlobalProperty(game::interface::igpSystemProgram,     session), "PCC");
    verifyNewString ("igpSystemVersion",     getGlobalProperty(game::interface::igpSystemVersion,     session), PCC2_VERSION);
    verifyNewInteger("igpSystemVersionCode", getGlobalProperty(game::interface::igpSystemVersionCode, session), PCC2_VERSION_CODE);
    verifyNewNull   ("igpSystemHasPassword", getGlobalProperty(game::interface::igpSystemHasPassword, session));
    verifyNewNull   ("igpSystemHost",        getGlobalProperty(game::interface::igpSystemHost,        session));
    verifyNewNull   ("igpSystemHostCode",    getGlobalProperty(game::interface::igpSystemHostCode,    session));
    verifyNewNull   ("igpSystemHostVersion", getGlobalProperty(game::interface::igpSystemHostVersion, session));
    verifyNewInteger("igpRandomSeed",        getGlobalProperty(game::interface::igpRandomSeed,        session), 42);
    verifyNewNull   ("igpRegSharewareFlag",  getGlobalProperty(game::interface::igpRegSharewareFlag,  session));
    verifyNewNull   ("igpRegSharewareText",  getGlobalProperty(game::interface::igpRegSharewareText,  session));
    verifyNewNull   ("igpRegStr1",           getGlobalProperty(game::interface::igpRegStr1,           session));
    verifyNewNull   ("igpRegStr2",           getGlobalProperty(game::interface::igpRegStr2,           session));
    verifyNewNull   ("igpTurnNumber",        getGlobalProperty(game::interface::igpTurnNumber,        session));
    verifyNewNull   ("igpTurnDate",          getGlobalProperty(game::interface::igpTurnDate,          session));
    verifyNewNull   ("igpTurnIsNew",         getGlobalProperty(game::interface::igpTurnIsNew,         session));
    verifyNewNull   ("igpTurnTime",          getGlobalProperty(game::interface::igpTurnTime,          session));
}

/** Test setGlobalProperty(). */
void
TestGameInterfaceGlobalProperty::testSet()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.rng().setSeed(42);
    session.setGame(new game::Game());

    // Seed
    {
        afl::data::IntegerValue iv(69);
        setGlobalProperty(game::interface::igpRandomSeed, session, &iv);
        TS_ASSERT_EQUALS(session.rng().getSeed(), 69U);
    }

    // Layer
    {
        afl::data::IntegerValue iv(6);
        setGlobalProperty(game::interface::igpSelectionLayer, session, &iv);
        TS_ASSERT_EQUALS(session.getGame()->selections().getCurrentLayer(), 6U);
    }

    // Layer: assigning null does not change
    {
        setGlobalProperty(game::interface::igpSelectionLayer, session, 0);
        TS_ASSERT_EQUALS(session.getGame()->selections().getCurrentLayer(), 6U);
    }

    // Error: not assignable
    {
        afl::data::IntegerValue iv(6);
        TS_ASSERT_THROWS(setGlobalProperty(game::interface::igpTurnNumber, session, &iv), interpreter::Error);
    }

    // Error: type error
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(setGlobalProperty(game::interface::igpRandomSeed, session, &sv), interpreter::Error);
    }

    // Error: range error
    {
        afl::data::IntegerValue iv(99);
        TS_ASSERT_THROWS(setGlobalProperty(game::interface::igpSelectionLayer, session, &iv), interpreter::Error);
    }
}

/** Test setGlobalProperty() with empty session. */
void
TestGameInterfaceGlobalProperty::testSetEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Seed - ok, directly in session
    {
        afl::data::IntegerValue iv(69);
        setGlobalProperty(game::interface::igpRandomSeed, session, &iv);
        TS_ASSERT_EQUALS(session.rng().getSeed(), 69U);
    }

    // Layer - not assignable
    {
        afl::data::IntegerValue iv(6);
        TS_ASSERT_THROWS(setGlobalProperty(game::interface::igpSelectionLayer, session, &iv), interpreter::Error);
    }
}

/** Test host version properties. */
void
TestGameInterfaceGlobalProperty::testHostVersions()
{
    struct TestCase {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestCase(HostVersion host)
            : tx(), fs(), session(tx, fs)
            { session.setRoot(game::test::makeRoot(host).asPtr()); }
        void verify(const char* name, int code)
            {
                afl::test::Assert a(name);
                verifyNewString (a("igpSystemHost"),     getGlobalProperty(game::interface::igpSystemHost,     session), name);
                verifyNewInteger(a("igpSystemHostCode"), getGlobalProperty(game::interface::igpSystemHostCode, session), code);
            }
    };

    TestCase(HostVersion(HostVersion::Host,   MKVERSION(3,22,48))).verify("Host",   0);
    TestCase(HostVersion(HostVersion::SRace,  MKVERSION(3,22,48))).verify("SRace",  1);
    TestCase(HostVersion(HostVersion::PHost,  MKVERSION(4,1,5)))  .verify("PHost",  2);
    TestCase(HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)))  .verify("NuHost", 3);
}

