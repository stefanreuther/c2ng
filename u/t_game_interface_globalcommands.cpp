/**
  *  \file u/t_game_interface_globalcommands.cpp
  *  \brief Test for game::interface::GlobalCommands
  */

#include "game/interface/globalcommands.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/playerlist.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "game/test/shiplist.hpp"
#include "game/interface/beamfunction.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/interface/planetfunction.hpp"

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
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgNull()
{
    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, 0), false);
}

/** Test checkPlayerSetArg: wrong type.
    A: call checkPlayerSetArg with a wrong type argument.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgWrong()
{
    afl::data::StringValue value("hi");
    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: integer.
    A: call checkPlayerSetArg with integer argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgInt()
{
    {
        afl::data::IntegerValue value(8);
        game::PlayerSet_t result;
        TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &value), true);
        TS_ASSERT_EQUALS(result, game::PlayerSet_t(8));
    }
    {
        afl::data::IntegerValue value(0);
        game::PlayerSet_t result;
        TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &value), true);
        TS_ASSERT_EQUALS(result, game::PlayerSet_t(0));
    }
}

/** Test checkPlayerSetArg: array.
    A: call checkPlayerSetArg with array argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgArray()
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().setNew(0, new afl::data::IntegerValue(4));
    ad->content().setNew(2, new afl::data::IntegerValue(7));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &av), true);
    TS_ASSERT_EQUALS(result, game::PlayerSet_t() + 4 + 7);
}

/** Test checkPlayerSetArg: out of range integer.
    A: call checkPlayerSetArg with out-of-range integer.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgIntRange()
{
    afl::data::IntegerValue value(-1);
    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &value), interpreter::Error);
}

/** Test checkPlayerSetArg: out of range integer in array.
    A: call checkPlayerSetArg with an array containing out-of-range argument.
    E: must throw exception. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgArrayRange()
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().setNew(0, new afl::data::IntegerValue(44));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &av), interpreter::Error);
}

/** Test checkPlayerSetArg: vector.
    A: call checkPlayerSetArg with afl::data::Vector argument.
    E: must return correct value. */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArgVector()
{
    afl::base::Ref<afl::data::Vector> vd = afl::data::Vector::create();
    vd->setNew(0, new afl::data::IntegerValue(9));
    vd->setNew(2, new afl::data::IntegerValue(1));
    afl::data::VectorValue vv(vd);

    game::PlayerSet_t result;
    TS_ASSERT_EQUALS(game::interface::checkPlayerSetArg(result, &vv), true);
    TS_ASSERT_EQUALS(result, game::PlayerSet_t() + 9 + 1);
}

/** Test checkPlayerSetArg: 2-D array.
    A: call checkPlayerSetArg with 2-D array.
    E: must fail */
void
TestGameInterfaceGlobalCommands::testCheckPlayerArg2DArray()
{
    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->addDimension(1);
    ad->content().setNew(0, new afl::data::IntegerValue(4));
    ad->content().setNew(2, new afl::data::IntegerValue(7));
    interpreter::ArrayValue av(ad);

    game::PlayerSet_t result;
    TS_ASSERT_THROWS(game::interface::checkPlayerSetArg(result, &av), interpreter::Error);
}

/** Test AddConfig command. */
void
TestGameInterfaceGlobalCommands::testAddConfig()
{
    // Normal case: 'AddConfig "key=value"' must set option
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("gamename = test 3");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddConfig(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.getRoot()->hostConfiguration()[game::config::HostConfiguration::GameName](), "test 3");
    }

    // Null: 'AddConfig EMPTY' must be ignored silently
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddConfig(env.session, env.proc, args));
    }

    // Syntax error: 'AddConfig "syntax-error"' must be rejected
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("gamename: test 3");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddConfig(env.session, env.proc, args), interpreter::Error);
    }

    // Error case: no root, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackString("gamename = test 3");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddConfig(env.session, env.proc, args), game::Exception);
    }

    // Error case: arity, command must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFAddConfig(env.session, env.proc, args), interpreter::Error);
    }
}

/** Test AddFCode command. */
void
TestGameInterfaceGlobalCommands::testAddFCode()
{
    // Normal case: 'AddFCode "definition"' must add fcode
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        seg.pushBackString("abc,p,Info");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddFCode(env.session, env.proc, args));

        const game::spec::FriendlyCode* fc = env.session.getShipList()->friendlyCodes().at(0);
        TS_ASSERT(fc != 0);
        TS_ASSERT_EQUALS(fc->getCode(), "abc");
        TS_ASSERT_EQUALS(fc->getFlags(), game::spec::FriendlyCode::FlagSet_t(game::spec::FriendlyCode::PlanetCode));

        game::PlayerList players;
        TS_ASSERT_EQUALS(fc->getDescription(players, env.tx), "Info");
    }

    // Null: 'AddFCode EMPTY' must be silently ignored
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddFCode(env.session, env.proc, args));
    }

    // Syntax error: Must be rejected
    // Exceptions are generated at different places.
    // As of 20230408, first comma is checked by AddFCode command and generates interpreter::Error.
    // Second comma is checked by FriendlyCode and generates std::range_error.
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        seg.pushBackString("abc");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddFCode(env.session, env.proc, args), std::exception);
    }
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        seg.pushBackString("abc,p");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddFCode(env.session, env.proc, args), std::exception);
    }

    // Error case: no ship list, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackString("abc,p,Info");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddFCode(env.session, env.proc, args), game::Exception);
    }

    // Error case: arity, command must fail
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFAddFCode(env.session, env.proc, args), interpreter::Error);
    }
}

/** Test AddPref command. */
void
TestGameInterfaceGlobalCommands::testAddPref()
{
    // Normal case: 'AddPref "key=value"' must set option
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("backup.turn = /dir");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddPref(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.getRoot()->userConfiguration()[game::config::UserConfiguration::Backup_Turn](), "/dir");
    }

    // Null: 'AddPref EMPTY' must be ignored silently
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAddPref(env.session, env.proc, args));
    }

    // Syntax error: 'AddPref "syntax-error"' must be rejected
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("backup.turn/dir");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddPref(env.session, env.proc, args), interpreter::Error);
    }

    // Error case: no root, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackString("backup.turn = /dir");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFAddPref(env.session, env.proc, args), game::Exception);
    }

    // Error case: arity, command must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFAddPref(env.session, env.proc, args), interpreter::Error);
    }
}

/** Test AuthPlayer command. */
void
TestGameInterfaceGlobalCommands::testAuthPlayer()
{
    const int PLAYER_NR = 7;
    game::AuthCache::Item match;
    match.playerNr = PLAYER_NR;

    // Normal case: 'AuthPlayer PLAYER, PASSWORD' must produce AuthCache entry
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(PLAYER_NR);
        seg.pushBackString("geheim");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAuthPlayer(env.session, env.proc, args));

        game::AuthCache::Items_t result = env.session.authCache().find(match);
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0]->password.orElse(""), "geheim");
    }

    // Null: any argument null must cause command to be ignored, AuthCache remains empty
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(7);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAuthPlayer(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.authCache().find(match).size(), 0U);
    }
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("geheim");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFAuthPlayer(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.authCache().find(match).size(), 0U);
    }

    // Range error: 'AuthPlayer 999, PASS' must be rejected
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(999);
        seg.pushBackString("geheim");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
    }

    // Type error: 'AuthPlayer "X", PASS' must be rejected
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackString("X");
        seg.pushBackString("geheim");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFAuthPlayer(env.session, env.proc, args), interpreter::Error);
    }
}

/** Test IFCCHistoryShowTurn(). */
void
TestGameInterfaceGlobalCommands::testCCHistoryShowTurn()
{
    // Success case: "CC$HistoryShowTurn TURN" must select history turn
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        seg.pushBackInteger(15);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.getGame()->getViewpointTurn()->getTurnNumber(), 15);
    }

    // Success case: "CC$HistoryShowTurn 0" must select current turn (which is 20)
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.getGame()->getViewpointTurn()->getTurnNumber(), 20);
    }

    // Failure case: command fails if unknown turn is selected
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        seg.pushBackInteger(5);        // not present
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Failure case: command fails if turn is present but not loaded
    {
        Environment env;
        addGame(env);
        env.session.getGame()->previousTurns().create(17);

        afl::data::Segment seg;
        seg.pushBackInteger(17);       // present but not loaded
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Null, command must be ignored
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args));

        TS_ASSERT_EQUALS(env.session.getGame()->getViewpointTurn()->getTurnNumber(), 20);
    }

    // Type error, command must fail
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail
    {
        Environment env;
        addGame(env);
        addHistoryTurns(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(15);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCHistoryShowTurn(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFCCSelectionExec(). */
void
TestGameInterfaceGlobalCommands::testCCSelectionExec()
{
    // Standard case: assign to layer 3 ('SelectionExec C := Current')
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        TS_ASSERT(!env.session.getGame()->selections().get(game::map::Selections::Planet, 2)->get(5));

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackString("c");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCSelectionExec(env.session, env.proc, args));

        TS_ASSERT(env.session.getGame()->selections().get(game::map::Selections::Planet, 2)->get(5));
    }

    // Standard case: assign to current layer ('SelectionExec Current := Planets - Current')
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        TS_ASSERT( env.session.getGame()->currentTurn().universe().planets().get(5)->isMarked());
        TS_ASSERT(!env.session.getGame()->currentTurn().universe().planets().get(15)->isMarked());

        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackString("pc!&");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCSelectionExec(env.session, env.proc, args));

        TS_ASSERT(!env.session.getGame()->selections().get(game::map::Selections::Planet, 0)->get(5));
        TS_ASSERT( env.session.getGame()->selections().get(game::map::Selections::Planet, 0)->get(15));
        TS_ASSERT(!env.session.getGame()->currentTurn().universe().planets().get(5)->isMarked());
        TS_ASSERT( env.session.getGame()->currentTurn().universe().planets().get(15)->isMarked());
    }

    // Null: 'CC$SelectionExec EMPTY, EMPTY' is ignored (will not happen in compiled code)
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCSelectionExec(env.session, env.proc, args));
    }

    // Null: 'CC$SelectionExec 0, EMPTY' is ignored (will not happen in compiled code)
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCCSelectionExec(env.session, env.proc, args));
    }

    // Type error, command must fail (will not happen in compiled code)
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        afl::data::Segment seg;
        seg.pushBackString("X");
        seg.pushBackString("pc!&");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCCSelectionExec(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail (will not happen in compiled code)
    {
        Environment env;
        addGame(env);
        addMarkedPlanets(env);

        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCCSelectionExec(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackString("X");
        seg.pushBackString("pc!&");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCCSelectionExec(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFCreateConfigOption(). */
void
TestGameInterfaceGlobalCommands::testCreateConfigOption()
{
    // String: 'CreateConfigOption NAME, "str"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("str");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), "");
    }

    // String: 'CreateConfigOption NAME, "string"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("string");        // differs
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), "");
    }

    // Integer: 'CreateConfigOption NAME, "int"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("int");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), 0);
        TS_ASSERT_EQUALS(opt->toString(), "0");
    }

    // Integer: 'CreateConfigOption NAME, "integer"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("integer");       // differs
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), 0);
        TS_ASSERT_EQUALS(opt->toString(), "0");
    }

    // Boolean: 'CreateConfigOption NAME, "bool"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("bool");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), 0);
        TS_ASSERT_EQUALS(opt->toString(), "No");
    }

    // Boolean: 'CreateConfigOption NAME, "boolean"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("boolean");       // differs
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));

        game::config::IntegerOption* opt = dynamic_cast<game::config::IntegerOption*>(env.session.getRoot()->hostConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), 0);
        TS_ASSERT_EQUALS(opt->toString(), "No");
    }

    // Error: 'CreateConfigOption NAME, "INVALID"' must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("joke");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCreateConfigOption(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFCreateConfigOption(env.session, env.proc, args), interpreter::Error);
    }

    // Null, command must be ignored
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreateConfigOption(env.session, env.proc, args));
    }
}

/** Test IFCreatePrefOption(). */
void
TestGameInterfaceGlobalCommands::testCreatePrefOption()
{
    // Subset of testCreateConfigOption() because it uses the same backend
    // String: 'CreatePrefOption NAME, "str"'
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackString("testopt");
        seg.pushBackString("str");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreatePrefOption(env.session, env.proc, args));

        game::config::StringOption* opt = dynamic_cast<game::config::StringOption*>(env.session.getRoot()->userConfiguration().getOptionByName("TestOpt"));
        TS_ASSERT(opt != 0);
        TS_ASSERT_EQUALS((*opt)(), "");
    }

    // Arity error, command must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFCreatePrefOption(env.session, env.proc, args), interpreter::Error);
    }

    // Null, command must be ignored
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFCreatePrefOption(env.session, env.proc, args));
    }
}

/** Test IFExport(). */
void
TestGameInterfaceGlobalCommands::testExport()
{
    afl::io::InternalFileSystem fs;
    Environment env(fs);
    addRoot(env);
    addShipList(env);
    game::test::initStandardBeams(*env.session.getShipList());

    // Success case: 'Export Beam, "ID,COST.MC", "/result.txt", "csv"' must produce file
    {
        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::BeamFunction(env.session));
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result.txt");
        seg.pushBackString("csv");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFExport(env.session, env.proc, args));

        afl::base::Ref<afl::io::Stream> in = fs.openFile("/result.txt", afl::io::FileSystem::OpenRead);
        afl::io::TextFile text(*in);
        String_t line;
        TS_ASSERT(text.readLine(line));
        TS_ASSERT_EQUALS(line, "\"ID\",\"COST.MC\"");
        TS_ASSERT(text.readLine(line));
        TS_ASSERT_EQUALS(line, "1,1");
        TS_ASSERT(text.readLine(line));
        TS_ASSERT_EQUALS(line, "2,2");
        TS_ASSERT(text.readLine(line));
        TS_ASSERT_EQUALS(line, "3,5");
    }

    // Optional character set: 'Export Beam, "ID,COST.MC", "/result.txt", "csv", "latin1"' must produce file
    {
        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::BeamFunction(env.session));
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result1.txt");
        seg.pushBackString("csv");
        seg.pushBackString("latin1");
        interpreter::Arguments args(seg, 0, 5);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFExport(env.session, env.proc, args));

        afl::base::Ref<afl::io::Stream> in = fs.openFile("/result1.txt", afl::io::FileSystem::OpenRead);
        afl::io::TextFile text(*in);
        String_t line;
        TS_ASSERT(text.readLine(line));
        TS_ASSERT_EQUALS(line, "\"ID\",\"COST.MC\"");
    }

    // Empty array: 'Export EMPTYARRAY, ...' must fail and not produce a file
    {
        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::PlanetFunction(env.session));
        seg.pushBackString("ID,NAME");
        seg.pushBackString("/result2.txt");
        seg.pushBackString("csv");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
        TS_ASSERT_THROWS(fs.openFile("/result2.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }

    // Null array, 'Export EMPTY, ....', command must be ignored
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result3.txt");
        seg.pushBackString("csv");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFExport(env.session, env.proc, args));
        TS_ASSERT_THROWS(fs.openFile("/result3.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }

    // Null other, 'Export ARRAY, EMPTY, ...', command must be ignored
    {
        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::BeamFunction(env.session));
        seg.pushBackNew(0);
        seg.pushBackString("/result4.txt");
        seg.pushBackString("csv");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFExport(env.session, env.proc, args));
        TS_ASSERT_THROWS(fs.openFile("/result4.txt", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    }

    // Error: not an array: 'Export INTEGER, ....', command must fail
    {
        afl::data::Segment seg;
        seg.pushBackInteger(10);
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result5.txt");
        seg.pushBackString("csv");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
    }

    // Error: arity error, command must fail
    {
        afl::data::Segment seg;
        seg.pushBackInteger(10);
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result6.txt");
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFExport(env.session, env.proc, args), interpreter::Error);
    }

    // Error: bad type, 'Export Beam, "ID,COST.MC", "/result.txt", BAD-TYPE', command must fail
    {
        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::BeamFunction(env.session));
        seg.pushBackString("ID,COST.MC");
        seg.pushBackString("/result7.txt");
        seg.pushBackString("noway");
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFExport(env.session, env.proc, args), std::runtime_error);
    }
}

/** Test IFNewCannedMarker(). */
void
TestGameInterfaceGlobalCommands::testNewCannedMarker()
{
    // Normal case: 'NewCannedMarker X,Y,SLOT': marker must be created
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
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCannedMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::MarkerDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getMarkerKind(), 2);
        TS_ASSERT_EQUALS((*it)->getColor(), 7);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Extra args: 'NewCannedMarker X,Y,SLOT,TAG,EXPIRE': marker must be created
    {
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(3);
        seg.pushBackInteger(99);
        seg.pushBackInteger(50);
        interpreter::Arguments args(seg, 0, 5);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCannedMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::MarkerDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getMarkerKind(), 2);
        TS_ASSERT_EQUALS((*it)->getColor(), 7);
        TS_ASSERT_EQUALS((*it)->getExpire(), 50);
        TS_ASSERT_EQUALS((*it)->getTag(), 99U);
    }

    // Null: 'NewCannedMarker X,Y,EMPTY,EMPTY,EMPTY': command must be ignored silently
    {
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 5);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCannedMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_EQUALS(it, dc.end());
    }

    // Arity error, command must fail
    {
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->userConfiguration().setOption("Chart.Marker3", "2,7,", game::config::ConfigurationOption::Game);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFNewCannedMarker(env.session, env.proc, args), interpreter::Error);
    }

    // Range error: 'NewCannedMarker X,Y,999': command must fail
    {
        Environment env;
        addRoot(env);
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(999);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewCannedMarker(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must fail
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewCannedMarker(env.session, env.proc, args), game::Exception);
    }

    // No root, command must fail
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewCannedMarker(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFNewCircle(). */
void
TestGameInterfaceGlobalCommands::testNewCircle()
{
    // Normal case: 'NewCircle X,Y,R'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(50);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCircle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::CircleDrawing);
        TS_ASSERT_EQUALS((*it)->getCircleRadius(), 50);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Extra args: 'NewCircle X,Y,R,COLOR,TAG,EXPIRE'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(50);
        seg.pushBackInteger(12);
        seg.pushBackInteger(88);
        seg.pushBackInteger(30);
        interpreter::Arguments args(seg, 0, 6);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCircle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::CircleDrawing);
        TS_ASSERT_EQUALS((*it)->getCircleRadius(), 50);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getColor(), 12);
        TS_ASSERT_EQUALS((*it)->getExpire(), 30);
        TS_ASSERT_EQUALS((*it)->getTag(), 88U);
    }

    // Null mandatory arg: 'NewCircle X,EMPTY,R', command must be ignored
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackNew(0);
        seg.pushBackInteger(50);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewCircle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_EQUALS(it, dc.end());
    }

    // Type error: 'NewCircle X,ERROR,R', command must fail
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackString("X");
        seg.pushBackInteger(50);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewCircle(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFNewCircle(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1000);
        seg.pushBackInteger(50);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewCircle(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFNewRectangle(). */
void
TestGameInterfaceGlobalCommands::testNewRectangle()
{
    // Normal case: 'NewRectangle X1,Y1,X2,Y2'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(2500);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewRectangle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::RectangleDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(2500, 1000));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Extra args: 'NewRectangle X1,Y1,X2,Y2,COLOR,TAG,EXPIRE'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(2500);
        seg.pushBackInteger(1000);
        seg.pushBackInteger(5);
        seg.pushBackInteger(77);
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 7);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewRectangle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::RectangleDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(2500, 1000));
        TS_ASSERT_EQUALS((*it)->getColor(), 5);
        TS_ASSERT_EQUALS((*it)->getExpire(), 100);
        TS_ASSERT_EQUALS((*it)->getTag(), 77U);
    }

    // Mandatory null arg: 'NewRectangle X1,EMPTY,X2,Y2', command must be ignored and no drawing being made
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackNew(0);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewRectangle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_EQUALS(it, dc.end());
    }

    // Excess size must be rejected
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(100);
        seg.pushBackInteger(200);
        seg.pushBackInteger(6000);
        seg.pushBackInteger(7000);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
    }

    // Wrapped map: coordinates must be normalized: 'NewRectangle X1,Y1,X2,Y2'
    {
        Environment env;
        addGame(env);
        env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

        afl::data::Segment seg;
        seg.pushBackInteger(1020);
        seg.pushBackInteger(2950);
        seg.pushBackInteger(2980);
        seg.pushBackInteger(1010);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewRectangle(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::RectangleDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1020, 2950));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(980, 3010));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Type error: 'NewLine X1,ERROR,X2,Y2', command must fail
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackString("X");
        seg.pushBackInteger(2050);
        seg.pushBackInteger(1500);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error, command must fail
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFNewRectangle(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must fail
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(1020);
        seg.pushBackInteger(2950);
        seg.pushBackInteger(2980);
        seg.pushBackInteger(1010);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFNewRectangle(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFNewRectangleRaw().
    Testing only the difference to IFNewRectangle(). */
void
TestGameInterfaceGlobalCommands::testNewRectangleRaw()
{
    // Wrapped map: coordinates must NOT be normalized: 'NewRectangleRaw X1,Y1,X2,Y2'
    {
        Environment env;
        addGame(env);
        env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

        afl::data::Segment seg;
        seg.pushBackInteger(1020);
        seg.pushBackInteger(2950);
        seg.pushBackInteger(2980);
        seg.pushBackInteger(1010);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewRectangleRaw(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::RectangleDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1020, 2950));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(2980, 1010));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }
}

/** Test IFNewLine().
    Very similar to IFNewRectangle(). */
void
TestGameInterfaceGlobalCommands::testNewLine()
{
    // Base case: 'NewLine X1,Y1,X2,Y2' (same as for NewRectangle)
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(2000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(2500);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewLine(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::LineDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(2000, 1200));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(2500, 1000));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Wrapped map: coordinates must be normalized: 'NewRectangle X1,Y1,X2,Y2'
    {
        Environment env;
        addGame(env);
        env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

        afl::data::Segment seg;
        seg.pushBackInteger(1020);
        seg.pushBackInteger(2950);
        seg.pushBackInteger(2980);
        seg.pushBackInteger(1010);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewLine(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::LineDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1020, 2950));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(980, 3010));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }
}

/** Test IFNewLineRaw().
    Testing only the difference to IFNewLine(). */
void
TestGameInterfaceGlobalCommands::testNewLineRaw()
{
    // Wrapped map: coordinates must NOT be normalized: 'NewLineRaw X1,Y1,X2,Y2'
    {
        Environment env;
        addGame(env);
        env.session.getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(1000, 1000), game::map::Point(3000, 3000));

        afl::data::Segment seg;
        seg.pushBackInteger(1020);
        seg.pushBackInteger(2950);
        seg.pushBackInteger(2980);
        seg.pushBackInteger(1010);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewLineRaw(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::LineDrawing);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1020, 2950));
        TS_ASSERT_EQUALS((*it)->getPos2(), game::map::Point(2980, 1010));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }
}

/** Test IFNewMarker(). */
void
TestGameInterfaceGlobalCommands::testNewMarker()
{
    // Normal case: 'NewMarker X,Y,TYPE'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::MarkerDrawing);
        TS_ASSERT_EQUALS((*it)->getMarkerKind(), 6);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1200, 1300));
        TS_ASSERT_EQUALS((*it)->getColor(), 9);
        TS_ASSERT_EQUALS((*it)->getExpire(), -1);
        TS_ASSERT_EQUALS((*it)->getTag(), 0U);
    }

    // Extra args: 'NewMarker X,Y,TYPE,TEXT,TAG,EXPIRE'
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        seg.pushBackInteger(6);
        seg.pushBackInteger(1);
        seg.pushBackString("Note");
        seg.pushBackInteger(66);
        seg.pushBackInteger(80);
        interpreter::Arguments args(seg, 0, 7);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_DIFFERS(it, dc.end());
        TS_ASSERT_EQUALS((*it)->getType(), game::map::Drawing::MarkerDrawing);
        TS_ASSERT_EQUALS((*it)->getMarkerKind(), 6);
        TS_ASSERT_EQUALS((*it)->getPos(), game::map::Point(1200, 1300));
        TS_ASSERT_EQUALS((*it)->getColor(), 1);
        TS_ASSERT_EQUALS((*it)->getComment(), "Note");
        TS_ASSERT_EQUALS((*it)->getExpire(), 80);
        TS_ASSERT_EQUALS((*it)->getTag(), 66U);
    }

    // Null mandatory arg: 'NewMarker X,Y,EMPTY', must not create a marker
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFNewMarker(env.session, env.proc, args));

        game::map::DrawingContainer& dc = env.session.getGame()->currentTurn().universe().drawings();
        game::map::DrawingContainer::Iterator_t it = dc.begin();
        TS_ASSERT_EQUALS(it, dc.end());
    }

    // Type error: 'NewMarker X,Y,"X"', command must be rejected
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewMarker(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error: 'NewMarker X,Y', command must be rejected
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFNewMarker(env.session, env.proc, args), interpreter::Error);
    }

    // No game, command must be rejected
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(1200);
        seg.pushBackInteger(1300);
        seg.pushBackInteger(6);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFNewMarker(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFHistoryLoadTurn(). */
void
TestGameInterfaceGlobalCommands::testHistoryLoadTurn()
{
    // Normal case: 'History.LoadTurn TURN' must load the turn
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
        TS_ASSERT_THROWS_NOTHING(game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

        // Check that TurnLoader was called
        TS_ASSERT_EQUALS(log, "loadHistoryTurn\n");

        // Check status of turn
        TS_ASSERT_EQUALS(env.session.getGame()->previousTurns().get(23)->getStatus(), game::HistoryTurn::Loaded);
    }

    // Normal case: 'History.LoadTurn 0' must load current turn, i.e. no-op
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
        TS_ASSERT_THROWS_NOTHING(game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

        // Check that TurnLoader was not called
        TS_ASSERT_EQUALS(log, "");
    }

    // Null case: 'History.LoadTurn EMPTY' is a no-op
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
        TS_ASSERT_THROWS_NOTHING(game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

        // Check that TurnLoader was not called
        TS_ASSERT_EQUALS(log, "");
    }

    // Load error: TurnLoader reports error, must be reflected in load status
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
        TS_ASSERT_THROWS_NOTHING(game::interface::IFHistoryLoadTurn(env.session, env.proc, args));

        // Check that TurnLoader was called
        TS_ASSERT_EQUALS(log, "loadHistoryTurn\n");

        // Check status of turn
        // Since NullTurnLoader claims WeaklyPositive, a load error produces Unavailable, not Failed.
        TS_ASSERT_EQUALS(env.session.getGame()->previousTurns().get(23)->getStatus(), game::HistoryTurn::Unavailable);
    }

    // Range error: cannot load future turns
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Range error: cannot load turns before the big bang
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Type error: 'History.LoadTurn "X"' is rejected
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Arity error
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Error case: no turn loader present
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), interpreter::Error);
    }

    // Error case: no root present
    {
        Environment env;
        addGame(env);
        addShipList(env);
        env.session.getGame()->currentTurn().setTurnNumber(25);

        afl::data::Segment seg;
        seg.pushBackInteger(23);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
    }

    // Error case: no game present
    {
        String_t log;
        Environment env;
        addRoot(env);
        addShipList(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

        afl::data::Segment seg;
        seg.pushBackInteger(23);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
    }

    // Error case: no ship list present
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
        TS_ASSERT_THROWS(game::interface::IFHistoryLoadTurn(env.session, env.proc, args), game::Exception);
    }
}

/** Test IFSaveGame(). */
void
TestGameInterfaceGlobalCommands::testSaveGame()
{
    // Normal case: 'SaveGame'
    {
        String_t log;
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSaveGame(env.session, env.proc, args));

        // Check that TurnLoader was called
        TS_ASSERT_EQUALS(log, "saveCurrentTurn\n");

        // Process is alive
        // We did not regularily start it, hence don't check for a specific state, but it must not be Failed.
        TS_ASSERT_DIFFERS(env.proc.getState(), interpreter::Process::Failed);
    }

    // Variation: mark it final: 'SaveGame "f"'
    {
        String_t log;
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

        afl::data::Segment seg;
        seg.pushBackString("f");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSaveGame(env.session, env.proc, args));

        // Check that TurnLoader was called
        TS_ASSERT_EQUALS(log, "saveCurrentTurn\n");
    }

    // Error: bad option: 'SaveGame "xyzzy"'
    {
        String_t log;
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

        afl::data::Segment seg;
        seg.pushBackString("xyzzy");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);

        // Check that TurnLoader was not called
        TS_ASSERT_EQUALS(log, "");
    }

    // Error: save failure
    {
        String_t log;
        Environment env;
        addRoot(env);
        addGame(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, false));

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSaveGame(env.session, env.proc, args));

        // Check that TurnLoader was called
        TS_ASSERT_EQUALS(log, "saveCurrentTurn\n");

        // Process must be marked failed
        TS_ASSERT_EQUALS(env.proc.getState(), interpreter::Process::Failed);
    }

    // Error: no turnloader
    {
        Environment env;
        addRoot(env);
        addGame(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
    }

    // Error: no game
    {
        String_t log;
        Environment env;
        addRoot(env);
        env.session.getRoot()->setTurnLoader(new NullTurnLoader(log, true));

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
    }

    // Error: no root
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFSaveGame(env.session, env.proc, args), interpreter::Error);
    }
}

/** Test IFSendMessage(). */
void
TestGameInterfaceGlobalCommands::testSendMessage()
{
    // Normal case: 'SendMessage 7, "hi", "there"'
    {
        Environment env;
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        afl::data::Segment seg;
        seg.pushBackInteger(7);
        seg.pushBackString("hi");
        seg.pushBackString("there");
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSendMessage(env.session, env.proc, args));

        game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
        TS_ASSERT_EQUALS(out.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(out.getMessageRawText(0), "hi\nthere");
        TS_ASSERT_EQUALS(out.getMessageReceivers(0), game::PlayerSet_t(7));
    }

    // Normal case: 'SendMessage Array(2,3,4), "knock knock"
    {
        Environment env;
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
        ad->addDimension(3);
        ad->content().pushBackInteger(2);
        ad->content().pushBackInteger(3);
        ad->content().pushBackInteger(4);

        afl::data::Segment seg;
        seg.pushBackNew(new interpreter::ArrayValue(ad));
        seg.pushBackString("knock knock");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSendMessage(env.session, env.proc, args));

        game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
        TS_ASSERT_EQUALS(out.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(out.getMessageRawText(0), "knock knock");
        TS_ASSERT_EQUALS(out.getMessageReceivers(0), game::PlayerSet_t() + 2 + 3 + 4);
    }

    // Null sender
    {
        Environment env;
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("hi");
        seg.pushBackString("there");
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSendMessage(env.session, env.proc, args));

        game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
        TS_ASSERT_EQUALS(out.getNumMessages(), 0U);
    }

    // Null text
    {
        Environment env;
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        afl::data::Segment seg;
        seg.pushBackInteger(7);
        seg.pushBackString("hi");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS_NOTHING(game::interface::IFSendMessage(env.session, env.proc, args));

        game::msg::Outbox& out = env.session.getGame()->currentTurn().outbox();
        TS_ASSERT_EQUALS(out.getNumMessages(), 0U);
    }

    // No game
    {
        Environment env;

        afl::data::Segment seg;
        seg.pushBackInteger(7);
        seg.pushBackString("hi");
        seg.pushBackString("there");
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFSendMessage(env.session, env.proc, args), game::Exception);
    }

    // Arity error
    {
        Environment env;
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        afl::data::Segment seg;
        seg.pushBackInteger(7);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFSendMessage(env.session, env.proc, args), interpreter::Error);
    }
}

