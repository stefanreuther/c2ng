/**
  *  \file test/game/interface/globalcommandstest.cpp
  *  \brief Test for game::interface::GlobalCommands
  */

#include "game/interface/globalcommands.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/beamfunction.hpp"
#include "game/interface/planetfunction.hpp"
#include "game/playerlist.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        interpreter::Process proc;

        Environment()
            : tx(), fs(), session(tx, fs),
              proc(session.world(), "test", 1)
            { }
        Environment(afl::io::FileSystem& xfs)
            : tx(), fs(), session(tx, xfs),
              proc(session.world(), "test", 1)
            { }
    };

    void addRoot(Environment& env)
    {
        env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    }

    void addGame(Environment& env)
    {
        env.session.setGame(new game::Game());
    }

    void addEditableGame(Environment& env)
    {
        addGame(env);
        env.session.getGame()->currentTurn().setLocalDataPlayers(game::PlayerSet_t(1));
    }

    void addShipList(Environment& env)
    {
        env.session.setShipList(new game::spec::ShipList());
    }

    void addHistoryTurns(Environment& env)
    {
        game::Game& g = *env.session.getGame();
        g.currentTurn().setTurnNumber(20);
        for (int i = 10; i < 20; ++i) {
            afl::base::Ref<game::Turn> t = *new game::Turn();
            t->setTurnNumber(i);
            g.previousTurns().create(i)->handleLoadSucceeded(t);
        }
    }

    void addMarkedPlanets(Environment& env)
    {
        game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
        for (int i = 1; i <= 20; ++i) {
            game::map::Planet& pl = *univ.planets().create(i);
            pl.setPosition(game::map::Point(1000, 1000 + 10*i));
            pl.internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(), 20, env.tx, env.session.log());
            pl.setIsMarked(i <= 10);
        }
    }

    /*
     *  Simple TurnLoader for testing
     */

    class NullTurnLoader : public game::TurnLoader {
     public:
        NullTurnLoader(String_t& log, bool status)
            : m_log(log), m_status(status)
            { }
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            {
                m_log += "loadCurrentTurn\n";
                return game::makeConfirmationTask(m_status, then);
            }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            {
                m_log += "saveCurrentTurn\n";
                return game::makeConfirmationTask(m_status, then);
            }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> status, const game::Root& /*root*/)
            { status.fill(WeaklyPositive); }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> then)
            {
                m_log += "loadHistoryTurn\n";
                return game::makeConfirmationTask(m_status, then);
            }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, std::auto_ptr<game::Task_t> then)
            { return then; }
        virtual String_t getProperty(Property /*p*/)
            { return String_t(); }
     private:
        String_t& m_log;
        bool m_status;
    };

}

/** Test checkPlayerSetArg: null.
    A: call checkPlayerSetArg with a null argument.
    E: result must be 0. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:null", a)
{
    game::PlayerSet_t result;
    a.checkEqual("", game::interface::checkPlayerSetArg(result, 0), false);
}

/** Test checkPlayerSetArg: wrong type.
    A: call checkPlayerSetArg with a wrong type argument.
    E: must throw exception. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:type-error", a)
{
    afl::data::StringValue value("hi");
    game::PlayerSet_t result;
    AFL_CHECK_THROWS(a, game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: integer.
    A: call checkPlayerSetArg with integer argument.
    E: must return correct value. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:int:8", a)
{
    afl::data::IntegerValue value(8);
    game::PlayerSet_t result;
    a.checkEqual("status", game::interface::checkPlayerSetArg(result, &value), true);
    a.checkEqual("result", result, game::PlayerSet_t(8));
}

AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:int:0", a)
{
    afl::data::IntegerValue value(0);
    game::PlayerSet_t result;
    a.checkEqual("status", game::interface::checkPlayerSetArg(result, &value), true);
    a.checkEqual("result", result, game::PlayerSet_t(0));
}

/** Test checkPlayerSetArg: array.
    A: call checkPlayerSetArg with array argument.
    E: must return correct value. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:array", a)
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().setNew(0, new afl::data::IntegerValue(4));
    ad->content().setNew(2, new afl::data::IntegerValue(7));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    a.checkEqual("status", game::interface::checkPlayerSetArg(result, &av), true);
    a.checkEqual("result", result, game::PlayerSet_t() + 4 + 7);
}

/** Test checkPlayerSetArg: out of range integer.
    A: call checkPlayerSetArg with out-of-range integer.
    E: must throw exception. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:int:out-of-range", a)
{
    afl::data::IntegerValue value(-1);
    game::PlayerSet_t result;
    AFL_CHECK_THROWS(a, game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: out of range integer in array.
    A: call checkPlayerSetArg with an array containing out-of-range argument.
    E: must throw exception. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:array:out-of-range", a)
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().setNew(0, new afl::data::IntegerValue(44));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    AFL_CHECK_THROWS(a, game::interface::checkPlayerSetArg(result, &av), interpreter::Error);
}

/** Test checkPlayerSetArg: vector.
    A: call checkPlayerSetArg with afl::data::Vector argument.
    E: must return correct value. */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:vector", a)
{
    afl::base::Ref<afl::data::Vector> vd = afl::data::Vector::create();
    vd->setNew(0, new afl::data::IntegerValue(9));
    vd->setNew(2, new afl::data::IntegerValue(1));
    afl::data::VectorValue vv(vd);

    game::PlayerSet_t result;
    a.checkEqual("status", game::interface::checkPlayerSetArg(result, &vv), true);
    a.checkEqual("result", result, game::PlayerSet_t() + 9 + 1);
}

/** Test checkPlayerSetArg: 2-D array.
    A: call checkPlayerSetArg with 2-D array.
    E: must fail */
AFL_TEST("game.interface.GlobalCommands:checkPlayerSetArg:2d-array", a)
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->addDimension(1);
    ad->content().setNew(0, new afl::data::IntegerValue(4));
    ad->content().setNew(2, new afl::data::IntegerValue(7));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    AFL_CHECK_THROWS(a, game::interface::checkPlayerSetArg(result, &av), interpreter::Error);
}

/*
 *  AddConfig
 */

// Normal case: 'AddConfig "key=value"' must set option
AFL_TEST("game.interface.GlobalCommands:IFAddConfig:normal", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("gamename = test 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAddConfig(env.session, env.proc, args));

    a.checkEqual("HostConfiguration", env.session.getRoot()->hostConfiguration()[game::config::HostConfiguration::GameName](), "test 3");
}

// Null: 'AddConfig EMPTY' must be ignored silently
AFL_TEST("game.interface.GlobalCommands:IFAddConfig:null", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAddConfig(env.session, env.proc, args));
}

// Syntax error: 'AddConfig "syntax-error"' must be rejected
AFL_TEST("game.interface.GlobalCommands:IFAddConfig:syntax-error", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("gamename: test 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddConfig(env.session, env.proc, args), interpreter::Error);
}

// Error case: no root, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddConfig:no-root", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("gamename = test 3");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddConfig(env.session, env.proc, args), game::Exception);
}

// Error case: arity, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddConfig:arity-error", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFAddConfig(env.session, env.proc, args), interpreter::Error);
}

/*
 *  AddFCode
 */

// Normal case: 'AddFCode "definition"' must add fcode
AFL_TEST("game.interface.GlobalCommands:IFAddFCode:normal", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    seg.pushBackString("abc,p,Info");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a("01. call"), game::interface::IFAddFCode(env.session, env.proc, args));

    const game::spec::FriendlyCode* fc = env.session.getShipList()->friendlyCodes().at(0);
    a.checkNonNull("11. fcode", fc);
    a.checkEqual("12. code", fc->getCode(), "abc");
    a.checkEqual("13, flags", fc->getFlags(), game::spec::FriendlyCode::FlagSet_t(game::spec::FriendlyCode::PlanetCode));

    game::PlayerList players;
    a.checkEqual("21. desc", fc->getDescription(players, env.tx), "Info");
}

// Null: 'AddFCode EMPTY' must be silently ignored
AFL_TEST("game.interface.GlobalCommands:IFAddFCode:null", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAddFCode(env.session, env.proc, args));
}

// Syntax error: Must be rejected
// Exceptions are generated at different places.
// As of 20230408, first comma is checked by AddFCode command and generates interpreter::Error.
// Second comma is checked by FriendlyCode and generates std::range_error.
AFL_TEST("game.interface.GlobalCommands:IFAddFCode:error:syntax:1", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    seg.pushBackString("abc");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddFCode(env.session, env.proc, args), std::exception);
}

AFL_TEST("game.interface.GlobalCommands:IFAddFCode:error:syntax:2", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    seg.pushBackString("abc,p");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddFCode(env.session, env.proc, args), std::exception);
}

// Error case: no ship list, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddFCode:error:no-shiplist", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("abc,p,Info");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddFCode(env.session, env.proc, args), game::Exception);
}

// Error case: arity, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddFCode:error:arity", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFAddFCode(env.session, env.proc, args), interpreter::Error);
}


/*
 *  AddPref
 */

// Normal case: 'AddPref "key=value"' must set option
AFL_TEST("game.interface.GlobalCommands:IFAddPref:normal", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("backup.turn = /dir");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAddPref(env.session, env.proc, args));

    a.checkEqual("UserConfiguration updated", env.session.getRoot()->userConfiguration()[game::config::UserConfiguration::Backup_Turn](), "/dir");
}

// Null: 'AddPref EMPTY' must be ignored silently
AFL_TEST("game.interface.GlobalCommands:IFAddPref:null", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAddPref(env.session, env.proc, args));
}

// Syntax error: 'AddPref "syntax-error"' must be rejected
AFL_TEST("game.interface.GlobalCommands:IFAddPref:error:syntax", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("backup.turn/dir");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddPref(env.session, env.proc, args), interpreter::Error);
}

// Error case: no root, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddPref:error:no-root", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("backup.turn = /dir");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFAddPref(env.session, env.proc, args), game::Exception);
}

// Error case: arity, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAddPref:error:arity", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFAddPref(env.session, env.proc, args), interpreter::Error);
}

/*
 *  AuthPlayer
 */

// Normal case: 'AuthPlayer PLAYER, PASSWORD' must produce AuthCache entry
AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:normal", a)
{
    Environment env;
    const int PLAYER_NR = 7;
    game::AuthCache::Item match;
    match.playerNr = PLAYER_NR;

    afl::data::Segment seg;
    seg.pushBackInteger(PLAYER_NR);
    seg.pushBackString("geheim");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAuthPlayer(env.session, env.proc, args));

    game::AuthCache::Items_t result = env.session.authCache().find(match);
    a.checkEqual("size", result.size(), 1U);
    a.checkEqual("password", result[0]->password.orElse(""), "geheim");
}

// Null: any argument null must cause command to be ignored, AuthCache remains empty
AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:null-first", a)
{
    Environment env;
    const int PLAYER_NR = 7;
    game::AuthCache::Item match;
    match.playerNr = PLAYER_NR;

    afl::data::Segment seg;
    seg.pushBackInteger(PLAYER_NR);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAuthPlayer(env.session, env.proc, args));

    a.checkEqual("authCache", env.session.authCache().find(match).size(), 0U);
}

AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:null-second", a)
{
    Environment env;
    const int PLAYER_NR = 7;
    game::AuthCache::Item match;
    match.playerNr = PLAYER_NR;

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("geheim");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFAuthPlayer(env.session, env.proc, args));

    a.checkEqual("authCache", env.session.authCache().find(match).size(), 0U);
}

// Range error: 'AuthPlayer 999, PASS' must be rejected
AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:error:range", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(999);
    seg.pushBackString("geheim");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
}

// Type error: 'AuthPlayer "X", PASS' must be rejected
AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:error:type", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("X");
    seg.pushBackString("geheim");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFAuthPlayer:error:arity", a)
{
    Environment env;

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
}


/*
 *  IFCCHistoryShowTurn
 */

// Success case: "CC$HistoryShowTurn TURN" must select history turn
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:success:turn", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    seg.pushBackInteger(15);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

    a.checkEqual("viewpointTurn", env.session.getGame()->viewpointTurn().getTurnNumber(), 15);
}

// Success case: "CC$HistoryShowTurn 0" must select current turn (which is 20)
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:success:current", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

    a.checkEqual("viewpointTurn", env.session.getGame()->viewpointTurn().getTurnNumber(), 20);
}

// Failure case: command fails if unknown turn is selected
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:error:bad-turn", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    seg.pushBackInteger(5);        // not present
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
}

// Failure case: command fails if turn is present but not loaded
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:error:not-loaded", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->previousTurns().create(17);

    afl::data::Segment seg;
    seg.pushBackInteger(17);       // present but not loaded
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
}

// Null, command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:null", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

    a.checkEqual("viewpointTurn", env.session.getGame()->viewpointTurn().getTurnNumber(), 20);
}

// Type error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:error:type", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:error:arity", a)
{
    Environment env;
    addGame(env);
    addHistoryTurns(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
}

// No game, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCCHistoryShowTurn:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(15);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), game::Exception);
}


/*
 *  Test IFCCSelectionExec
 */

// Standard case: assign to layer 3 ('SelectionExec C := Current')
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:success:assign-to-named", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    a.check("01. selection", !env.session.getGame()->selections().get(game::map::Selections::Planet, 2)->get(5));

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackString("c");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a("11. exec"), game::interface::IFCCSelectionExec(env.session, env.proc, args));

    a.check("21. selection", env.session.getGame()->selections().get(game::map::Selections::Planet, 2)->get(5));
}

// Standard case: assign to current layer ('SelectionExec Current := Planets - Current')
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:success:assign-to-current", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    a.check("01. selection",  env.session.getGame()->currentTurn().universe().planets().get(5)->isMarked());
    a.check("02. selection", !env.session.getGame()->currentTurn().universe().planets().get(15)->isMarked());

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    seg.pushBackString("pc!&");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a("11. exec"), game::interface::IFCCSelectionExec(env.session, env.proc, args));

    a.check("21. selection", !env.session.getGame()->selections().get(game::map::Selections::Planet, 0)->get(5));
    a.check("22. selection",  env.session.getGame()->selections().get(game::map::Selections::Planet, 0)->get(15));
    a.check("23. selection", !env.session.getGame()->currentTurn().universe().planets().get(5)->isMarked());
    a.check("24. selection",  env.session.getGame()->currentTurn().universe().planets().get(15)->isMarked());
}

// Null: 'CC$SelectionExec EMPTY, EMPTY' is ignored (will not happen in compiled code)
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:null", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCCSelectionExec(env.session, env.proc, args));
}

// Null: 'CC$SelectionExec 0, EMPTY' is ignored (will not happen in compiled code)
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:int+null", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCCSelectionExec(env.session, env.proc, args));
}

// Type error, command must fail (will not happen in compiled code)
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:error:type", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    afl::data::Segment seg;
    seg.pushBackString("X");
    seg.pushBackString("pc!&");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCCSelectionExec(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail (will not happen in compiled code)
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:error:arity", a)
{
    Environment env;
    addGame(env);
    addMarkedPlanets(env);

    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCCSelectionExec(env.session, env.proc, args), interpreter::Error);
}

// No game, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCCSelectionExec:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("X");
    seg.pushBackString("pc!&");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCCSelectionExec(env.session, env.proc, args), game::Exception);
}

/*
 *  IFCreateConfigOption
 */

// String: 'CreateConfigOption NAME, "str"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:str", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("str");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), "");
}

// String: 'CreateConfigOption NAME, "string"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:string", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("string");        // differs
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), "");
}

// Integer: 'CreateConfigOption NAME, "int"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:int", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("int");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), 0);
    a.checkEqual("toString", opt->toString(), "0");
}

// Integer: 'CreateConfigOption NAME, "integer"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:integer", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("integer");       // differs
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), 0);
    a.checkEqual("toString", opt->toString(), "0");
}

// Boolean: 'CreateConfigOption NAME, "bool"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:bool", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("bool");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), 0);
    a.checkEqual("toString", opt->toString(), "No");
}

// Boolean: 'CreateConfigOption NAME, "boolean"'
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:success:boolean", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("boolean");       // differs
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));

    game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), 0);
    a.checkEqual("toString", opt->toString(), "No");
}

// Error: 'CreateConfigOption NAME, "INVALID"' must fail
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:error:type-name", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("joke");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:error:arity", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args), interpreter::Error);
}

// Null, command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFCreateConfigOption:null", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreateConfigOption(env.session, env.proc, args));
}

/*
 *  IFCreatePrefOption
 *
 *  Subset of testCreateConfigOption() because it uses the same backend
 */

    // String: 'CreatePrefOption NAME, "str"'
AFL_TEST("game.interface.GlobalCommands:IFCreatePrefOption:success:str", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackString("testopt");
    seg.pushBackString("str");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreatePrefOption(env.session, env.proc, args));

    game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->userConfiguration().getOptionByName("TestOpt"));
    a.checkNonNull("option", opt);
    a.checkEqual("value", (*opt)(), "");
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFCreatePrefOption:error:arity", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFCreatePrefOption(env.session, env.proc, args), interpreter::Error);
}

// Null, command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFCreatePrefOption:null", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFCreatePrefOption(env.session, env.proc, args));
}

/*
 *  IFExport
 */

// Success case: 'Export Beam, "ID,COST.MC", "/result.txt", "csv"' must produce file
AFL_TEST("game.interface.GlobalCommands:IFExport:success", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::BeamFunction(env.session));
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result.txt");
    seg.pushBackString("csv");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFExport(env.session, env.proc, args));

    afl::base::Ref<afl::io::Stream> in = fs.openFile("/result.txt", afl::io::FileSystem::OpenRead);
    afl::io::TextFile text(*in);
    String_t line;
    a.check("11. readLine", text.readLine(line));
    a.checkEqual("12. line", line, "\"ID\",\"COST.MC\"");
    a.check("13. readLine", text.readLine(line));
    a.checkEqual("14. line", line, "1,1");
    a.check("15. readLine", text.readLine(line));
    a.checkEqual("16. line", line, "2,2");
    a.check("17. readLine", text.readLine(line));
    a.checkEqual("18. line", line, "3,5");
}

// Optional character set: 'Export Beam, "ID,COST.MC", "/result.txt", "csv", "latin1"' must produce file
AFL_TEST("game.interface.GlobalCommands:IFExport:success:option", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::BeamFunction(env.session));
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result1.txt");
    seg.pushBackString("csv");
    seg.pushBackString("latin1");
    interpreter::Arguments args(seg, 0, 5);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFExport(env.session, env.proc, args));

    afl::base::Ref<afl::io::Stream> in = fs.openFile("/result1.txt", afl::io::FileSystem::OpenRead);
    afl::io::TextFile text(*in);
    String_t line;
    a.check("11. readLine", text.readLine(line));
    a.checkEqual("12. line", line, "\"ID\",\"COST.MC\"");
}

// Empty array: 'Export EMPTYARRAY, ...' must fail and not produce a file
AFL_TEST("game.interface.GlobalCommands:IFExport:error:empty-array", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::PlanetFunction(env.session));
    seg.pushBackString("ID,NAME");
    seg.pushBackString("/result2.txt");
    seg.pushBackString("csv");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a("command fails"), game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
    AFL_CHECK_THROWS(a("no file created"), fs.openFile("/result2.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

// Null array, 'Export EMPTY, ....', command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFExport:null-array", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result3.txt");
    seg.pushBackString("csv");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a("command succeeds"), game::interface::IFExport(env.session, env.proc, args));
    AFL_CHECK_THROWS(a("no file created"), fs.openFile("/result3.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

// Null other, 'Export ARRAY, EMPTY, ...', command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFExport:null-spec", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::BeamFunction(env.session));
    seg.pushBackNew(0);
    seg.pushBackString("/result4.txt");
    seg.pushBackString("csv");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a("command succeeds"), game::interface::IFExport(env.session, env.proc, args));
    AFL_CHECK_THROWS(a("no file created"), fs.openFile("/result4.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

// Error: not an array: 'Export INTEGER, ....', command must fail
AFL_TEST("game.interface.GlobalCommands:IFExport:error:type", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackInteger(10);
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result5.txt");
    seg.pushBackString("csv");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
}

// Error: arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFExport:error:arity", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackInteger(10);
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result6.txt");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
}

// Error: bad type, 'Export Beam, "ID,COST.MC", "/result.txt", BAD-TYPE', command must fail
AFL_TEST("game.interface.GlobalCommands:IFExport:error:bad-format", a)
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::BeamFunction(env.session));
    seg.pushBackString("ID,COST.MC");
    seg.pushBackString("/result7.txt");
    seg.pushBackString("noway");
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFExport(env.session, env.proc, args), std::runtime_error);
}

/*
 *  IFNewCannedMarker
 */

// Normal case: 'NewCannedMarker X,Y,SLOT': marker must be created
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:success", a)
{
    Environment env;
    addRoot(env);
    addEditableGame(env);
    env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(3);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker found", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::MarkerDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("14. kind",   (*it)->getMarkerKind(), 2);
    a.checkEqual("15. color",  (*it)->getColor(), 7);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Extra args: 'NewCannedMarker X,Y,SLOT,TAG,EXPIRE': marker must be created
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:success:extra", a)
{
    Environment env;
    addRoot(env);
    addEditableGame(env);
    env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(3);
    seg.pushBackInteger(99);
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 5);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker found", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::MarkerDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("14. kind",   (*it)->getMarkerKind(), 2);
    a.checkEqual("15. color",  (*it)->getColor(), 7);
    a.checkEqual("16. expire", (*it)->getExpire(), 50);
    a.checkEqual("17. tag",    (*it)->getTag(), 99U);
}

// Null: 'NewCannedMarker X,Y,EMPTY,EMPTY,EMPTY': command must be ignored silently
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:null", a)
{
    Environment env;
    addRoot(env);
    addEditableGame(env);
    env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 5);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("no marker created", it == dc.end());
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:error:arity", a)
{
    Environment env;
    addRoot(env);
    addEditableGame(env);
    env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args), interpreter::Error);
}

// Range error: 'NewCannedMarker X,Y,999': command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:error:range", a)
{
    Environment env;
    addRoot(env);
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(999);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args), interpreter::Error);
}

// No game, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:error:no-game", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args), game::Exception);
}

// No root, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:error:no-root", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args), game::Exception);
}

// Error case, game not played
AFL_TEST("game.interface.GlobalCommands:IFNewCannedMarker:error:not-played", a)
{
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(3);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCannedMarker(env.session, env.proc, args), game::Exception);

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    a.check("still empty", dc.begin() == dc.end());
}

/*
 *  IFNewCircle
 */

// Normal case: 'NewCircle X,Y,R'
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:success", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCircle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker found", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::CircleDrawing);
    a.checkEqual("13. radius", (*it)->getCircleRadius(), 50);
    a.checkEqual("14. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Extra args: 'NewCircle X,Y,R,COLOR,TAG,EXPIRE'
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:success:extra", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(50);
    seg.pushBackInteger(12);
    seg.pushBackInteger(88);
    seg.pushBackInteger(30);
    interpreter::Arguments args(seg, 0, 6);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCircle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker found", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::CircleDrawing);
    a.checkEqual("13. radius", (*it)->getCircleRadius(), 50);
    a.checkEqual("14. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("15. color",  (*it)->getColor(), 12);
    a.checkEqual("16. expire", (*it)->getExpire(), 30);
    a.checkEqual("17. tag",    (*it)->getTag(), 88U);
}

// Null mandatory arg: 'NewCircle X,EMPTY,R', command must be ignored
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:null", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackNew(0);
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewCircle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. no marker created", it == dc.end());
}

// Type error: 'NewCircle X,ERROR,R', command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:error:type", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackString("X");
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCircle(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:error:arity", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFNewCircle(env.session, env.proc, args), interpreter::Error);
}

// No game, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1000);
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCircle(env.session, env.proc, args), game::Exception);
}

// Error case, game not played
AFL_TEST("game.interface.GlobalCommands:IFNewCircle:error:not-played", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(50);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewCircle(env.session, env.proc, args), game::Exception);

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    a.check("still empty", dc.begin() == dc.end());
}

/*
 *  IFNewRectangle
 */

// Normal case: 'NewRectangle X1,Y1,X2,Y2'
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:success", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(2500);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewRectangle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::RectangleDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(2500, 1000));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Extra args: 'NewRectangle X1,Y1,X2,Y2,COLOR,TAG,EXPIRE'
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:success:extra", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(2500);
    seg.pushBackInteger(1000);
    seg.pushBackInteger(5);
    seg.pushBackInteger(77);
    seg.pushBackInteger(100);
    interpreter::Arguments args(seg, 0, 7);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewRectangle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::RectangleDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(2500, 1000));
    a.checkEqual("15. color",  (*it)->getColor(), 5);
    a.checkEqual("16. expire", (*it)->getExpire(), 100);
    a.checkEqual("17. tag",    (*it)->getTag(), 77U);
}

// Mandatory null arg: 'NewRectangle X1,EMPTY,X2,Y2', command must be ignored and no drawing being made
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:null", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackNew(0);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewRectangle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. no marker created", it == dc.end());
}

// Excess size must be rejected
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:error:excess-size", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(100);
    seg.pushBackInteger(200);
    seg.pushBackInteger(6000);
    seg.pushBackInteger(7000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
}

// Wrapped map: coordinates must be normalized: 'NewRectangle X1,Y1,X2,Y2'
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:success:wrapped-map", a)
{
    Environment env;
    addEditableGame(env);
    env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

    afl::data::Segment seg;
    seg.pushBackInteger(1020);
    seg.pushBackInteger(2950);
    seg.pushBackInteger(2980);
    seg.pushBackInteger(1010);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewRectangle(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::RectangleDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(1020, 2950));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(980, 3010));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Type error: 'NewLine X1,ERROR,X2,Y2', command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:error:type", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackString("X");
    seg.pushBackInteger(2050);
    seg.pushBackInteger(1500);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
}

// Arity error, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:error:arity", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
}

// No game, command must fail
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(1020);
    seg.pushBackInteger(2950);
    seg.pushBackInteger(2980);
    seg.pushBackInteger(1010);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFNewRectangle(env.session, env.proc, args), game::Exception);
}

// Error case, game not played
AFL_TEST("game.interface.GlobalCommands:IFNewRectangle:error:not-played", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(2500);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFNewRectangle(env.session, env.proc, args), game::Exception);

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    a.check("still empty", dc.begin() == dc.end());
}

/*
 *  IFNewRectangleRaw
 *
 *  Testing only the difference to IFNewRectangle()
 */

// Wrapped map: coordinates must NOT be normalized: 'NewRectangleRaw X1,Y1,X2,Y2'
AFL_TEST("game.interface.GlobalCommands:IFNewRectangleRaw", a)
{
    Environment env;
    addEditableGame(env);
    env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

    afl::data::Segment seg;
    seg.pushBackInteger(1020);
    seg.pushBackInteger(2950);
    seg.pushBackInteger(2980);
    seg.pushBackInteger(1010);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewRectangleRaw(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::RectangleDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(1020, 2950));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(2980, 1010));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}


/*
 *  IFNewLine
 *
 *  Very similar to IFNewRectangle()
 */

// Base case: 'NewLine X1,Y1,X2,Y2' (same as for NewRectangle)
AFL_TEST("game.interface.GlobalCommands:IFNewLine:success", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(2500);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewLine(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::LineDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(2000, 1200));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(2500, 1000));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Wrapped map: coordinates must be normalized: 'NewRectangle X1,Y1,X2,Y2'
AFL_TEST("game.interface.GlobalCommands:IFNewLine:success:wrapped-map", a)
{
    Environment env;
    addEditableGame(env);
    env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

    afl::data::Segment seg;
    seg.pushBackInteger(1020);
    seg.pushBackInteger(2950);
    seg.pushBackInteger(2980);
    seg.pushBackInteger(1010);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewLine(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::LineDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(1020, 2950));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(980, 3010));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

/*
 *  Test IFNewLineRaw
 *
 *  Testing only the difference to IFNewLine()
 */

// Wrapped map: coordinates must NOT be normalized: 'NewLineRaw X1,Y1,X2,Y2'
AFL_TEST("game.interface.GlobalCommands:IFNewLineRaw", a)
{
    Environment env;
    addEditableGame(env);
    env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

    afl::data::Segment seg;
    seg.pushBackInteger(1020);
    seg.pushBackInteger(2950);
    seg.pushBackInteger(2980);
    seg.pushBackInteger(1010);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewLineRaw(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::LineDrawing);
    a.checkEqual("13. pos",    (*it)->getPos(), game::map::Point(1020, 2950));
    a.checkEqual("14. pos2",   (*it)->getPos2(), game::map::Point(2980, 1010));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

/*
 *  IFNewMarker
 */

// Normal case: 'NewMarker X,Y,TYPE'
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:success", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackInteger(6);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::MarkerDrawing);
    a.checkEqual("13. kind",   (*it)->getMarkerKind(), 6);
    a.checkEqual("14. pos",    (*it)->getPos(), game::map::Point(1200, 1300));
    a.checkEqual("15. color",  (*it)->getColor(), 9);
    a.checkEqual("16. expire", (*it)->getExpire(), -1);
    a.checkEqual("17. tag",    (*it)->getTag(), 0U);
}

// Extra args: 'NewMarker X,Y,TYPE,TEXT,TAG,EXPIRE'
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:success:extra", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackInteger(6);
    seg.pushBackInteger(1);
    seg.pushBackString("Note");
    seg.pushBackInteger(66);
    seg.pushBackInteger(80);
    interpreter::Arguments args(seg, 0, 7);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. marker created", it != dc.end());
    a.checkEqual("12. type",   (*it)->getType(), game::map::Drawing::MarkerDrawing);
    a.checkEqual("13. kind",   (*it)->getMarkerKind(), 6);
    a.checkEqual("14. pos",    (*it)->getPos(), game::map::Point(1200, 1300));
    a.checkEqual("15. color",  (*it)->getColor(), 1);
    a.checkEqual("16. note",   (*it)->getComment(), "Note");
    a.checkEqual("17. expire", (*it)->getExpire(), 80);
    a.checkEqual("18. tag",    (*it)->getTag(), 66U);
}

// Null mandatory arg: 'NewMarker X,Y,EMPTY', must not create a marker
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:null", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFNewMarker(env.session, env.proc, args));

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    game::map::DrawingContainer::Iterator_t it = dc.begin();
    a.check("11. no marker created", it == dc.end());
}

// Type error: 'NewMarker X,Y,"X"', command must be rejected
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:error:type", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewMarker(env.session, env.proc, args), interpreter::Error);
}

// Arity error: 'NewMarker X,Y', command must be rejected
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:error:arity", a)
{
    Environment env;
    addEditableGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFNewMarker(env.session, env.proc, args), interpreter::Error);
}

// No game, command must be rejected
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackInteger(6);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewMarker(env.session, env.proc, args), game::Exception);
}

// Error case, game not played
AFL_TEST("game.interface.GlobalCommands:IFNewMarker:error:not-played", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1200);
    seg.pushBackInteger(1300);
    seg.pushBackInteger(6);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFNewMarker(env.session, env.proc, args), game::Exception);

    game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
    a.check("still empty", dc.begin() == dc.end());
}

/*
 *  IFHistoryLoadTurn
 */

// Normal case: 'History.LoadTurn TURN' must load the turn
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:success:turn", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

    // Check that TurnLoader was called
    a.checkEqual("log", log, "loadHistoryTurn\n");

    // Check status of turn
    a.checkEqual("status", env.session.getGame()->previousTurns().get(23)->getStatus(), game::HistoryTurn::Loaded);
}

// Normal case: 'History.LoadTurn 0' must load current turn, i.e. no-op
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:success:load-current", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

    // Check that TurnLoader was not called
    a.checkEqual("log", log, "");
}

// Null case: 'History.LoadTurn EMPTY' is a no-op
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:null", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

    // Check that TurnLoader was not called
    a.checkEqual("log", log, "");
}

// Load error: TurnLoader reports error, must be reflected in load status
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error-while-loading", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, false));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

    // Check that TurnLoader was called
    a.checkEqual("log", log, "loadHistoryTurn\n");

    // Check status of turn
    // Since NullTurnLoader claims WeaklyPositive, a load error produces Unavailable, not Failed.
    a.checkEqual("status", env.session.getGame()->previousTurns().get(23)->getStatus(), game::HistoryTurn::Unavailable);
}

// Range error: cannot load future turns
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:future", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(26);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
}

// Range error: cannot load turns before the big bang
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:past", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(-1);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
}

// Type error: 'History.LoadTurn "X"' is rejected
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:type", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:arity", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
}

// Error case: no turn loader present
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:no-turnloader", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    addShipList(env);
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
}

// Error case: no root present
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:no-root", a)
{
    Environment env;
    addGame(env);
    addShipList(env);
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
}

// Error case: no game present
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:no-game", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addShipList(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
}

// Error case: no ship list present
AFL_TEST("game.interface.GlobalCommands:IFHistoryLoadTurn:error:no-shiplist", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));
    env.session.getGame()->currentTurn().setTurnNumber(25);

    afl::data::Segment seg;
    seg.pushBackInteger(23);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
}

/*
 *  IFSaveGame
 */

// Normal case: 'SaveGame'
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:success", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSaveGame(env.session, env.proc, args));

    // Check that TurnLoader was called
    a.checkEqual("log", log, "saveCurrentTurn\n");

    // Process is alive
    // We did not regularily start it, hence don't check for a specific state, but it must not be Failed.
    a.checkDifferent("process status", env.proc.getState(), interpreter::Process::Failed);
}

// Variation: mark it final: 'SaveGame "f"'
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:success:final", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

    afl::data::Segment seg;
    seg.pushBackString("f");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSaveGame(env.session, env.proc, args));

    // Check that TurnLoader was called
    a.checkEqual("log", log, "saveCurrentTurn\n");
}

// Error: bad option: 'SaveGame "xyzzy"'
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:error:bad-option", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

    afl::data::Segment seg;
    seg.pushBackString("xyzzy");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);

    // Check that TurnLoader was not called
    a.checkEqual("log", log, "");
}

// Error: save failure
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:error:save-error", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, false));

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSaveGame(env.session, env.proc, args));

    // Check that TurnLoader was called
    a.checkEqual("log", log, "saveCurrentTurn\n");

    // Process must be marked failed
    a.checkEqual("process status", env.proc.getState(), interpreter::Process::Failed);
}

// Error: no turnloader
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:error:no-turnloader", a)
{
    Environment env;
    addRoot(env);
    addGame(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
}

// Error: no game
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:error:no-game", a)
{
    String_t log;
    Environment env;
    addRoot(env);
    env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
}

// Error: no root
AFL_TEST("game.interface.GlobalCommands:IFSaveGame:error:no-root", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
}

/*
 *  IFSendMessage
 */


// Normal case: 'SendMessage 7, "hi", "there"'
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:success", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    env.session.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(3));

    afl::data::Segment seg;
    seg.pushBackInteger(7);
    seg.pushBackString("hi");
    seg.pushBackString("there");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSendMessage(env.session, env.proc, args));

    game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
    a.checkEqual("getNumMessages", out.getNumMessages(), 1U);
    a.checkEqual("getMessageRawText", out.getMessageRawText(0), "hi\nthere");
    a.checkEqual("getMessageReceivers", out.getMessageReceivers(0), game::PlayerSet_t(7));
}

// Normal case: 'SendMessage Array(2,3,4), "knock knock"
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:success:array", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    env.session.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(3));

    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().pushBackInteger(2);
    ad->content().pushBackInteger(3);
    ad->content().pushBackInteger(4);

    afl::data::Segment seg;
    seg.pushBackNew(new interpreter::ArrayValue(ad));
    seg.pushBackString("knock knock");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSendMessage(env.session, env.proc, args));

    game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
    a.checkEqual("getNumMessages", out.getNumMessages(), 1U);
    a.checkEqual("getMessageRawText", out.getMessageRawText(0), "knock knock");
    a.checkEqual("getMessageReceivers", out.getMessageReceivers(0), game::PlayerSet_t() + 2 + 3 + 4);
}

// Null receiver
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:null-receiver", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    env.session.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(3));

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("hi");
    seg.pushBackString("there");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSendMessage(env.session, env.proc, args));

    game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
    a.checkEqual("getNumMessages", out.getNumMessages(), 0U);
}

// Null text
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:null-text", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    env.session.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(3));

    afl::data::Segment seg;
    seg.pushBackInteger(7);
    seg.pushBackString("hi");
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_SUCCEEDS(a, game::interface::IFSendMessage(env.session, env.proc, args));

    game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
    a.checkEqual("getNumMessages", out.getNumMessages(), 0U);
}

// No game
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(7);
    seg.pushBackString("hi");
    seg.pushBackString("there");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFSendMessage(env.session, env.proc, args), game::Exception);
}

// Arity error
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:error:arity", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    env.session.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(3));

    afl::data::Segment seg;
    seg.pushBackInteger(7);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFSendMessage(env.session, env.proc, args), interpreter::Error);
}

// Viewpoint player not editable
AFL_TEST("game.interface.GlobalCommands:IFSendMessage:error:not-played", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);

    afl::data::Segment seg;
    seg.pushBackInteger(7);
    seg.pushBackString("hi");
    seg.pushBackString("there");
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFSendMessage(env.session, env.proc, args), game::Exception);
}
