/**
  *  \file game/interface/privatefunctions.cpp
  *  \brief Class game::interface::PrivateFunctions
  */

#include "game/interface/privatefunctions.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/interface/vmfile.hpp"
#include "game/limits.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/spec/shiplist.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/genericvalue.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simpleprocedure.hpp"

using afl::base::Ptr;
using afl::string::Format;
using afl::sys::LogListener;
using game::Root;
using game::Session;
using game::StatusTask_t;
using game::config::UserConfiguration;
using game::map::Object;
using game::spec::ShipList;
using interpreter::Arguments;
using interpreter::BytecodeObject;
using interpreter::Error;
using interpreter::GenericValue;
using interpreter::Opcode;
using interpreter::Process;
using interpreter::SimpleProcedure;
using util::RequestSender;

namespace {
    const char LOG_NAME[] = "game.interface";

    typedef GenericValue<Ptr<Root> > RootValue_t;
    typedef GenericValue<RequestSender<game::browser::Session> > BrowserValue_t;
    typedef GenericValue<RequestSender<Session> > GameValue_t;

    /* Create code to call the given function. */
    void call(Session& session, BytecodeObject& bco, void (*func)(Session&, Process&, Arguments&), int16_t numArgs)
    {
        SimpleProcedure<Session&> closure(session, func);
        bco.addPushLiteral(&closure);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, numArgs);
    }

    /* Create a StatusTask_t that will continue or fail the process */
    std::auto_ptr<StatusTask_t> makeStatusTask(Session& session, Process& proc, const char* operationId)
    {
        class Task : public StatusTask_t {
         public:
            Task(Session& session, Process& proc, const char* operationId)
                : m_session(session), m_process(proc), m_operationId(operationId)
                { }
            virtual void call(bool success)
                {
                    // Make a copy of m_session.
                    // The continueProcess() will destroy the Task.
                    Session& session = m_session;
                    if (success) {
                        session.processList().continueProcess(m_process);
                    } else {
                        session.processList().continueProcessWithFailure(m_process, Format("%s failed", m_operationId));
                    }
                    session.runScripts();
                }
         private:
            Session& m_session;
            Process& m_process;
            String_t m_operationId;
        };

        return std::auto_ptr<StatusTask_t>(new Task(session, proc, operationId));
    }
}


/*
 *  Public
 */

void
game::interface::PrivateFunctions::addTakeRoot(Session& session, interpreter::BytecodeObject& bco, util::RequestSender<game::Session> gameSender, util::RequestSender<game::browser::Session> browserSender)
{
    GameValue_t gv(gameSender);
    bco.addPushLiteral(&gv);
    BrowserValue_t bv(browserSender);
    bco.addPushLiteral(&bv);
    call(session, bco, IFTakeRoot, 2);
}

void
game::interface::PrivateFunctions::addMakeGame(Session& session, interpreter::BytecodeObject& bco)
{
    call(session, bco, IFMakeGame, 0);
}

void
game::interface::PrivateFunctions::addMakeShipList(Session& session, interpreter::BytecodeObject& bco)
{
    call(session, bco, IFMakeShipList, 0);
}

void
game::interface::PrivateFunctions::addLoadShipList(Session& session, interpreter::BytecodeObject& bco)
{
    call(session, bco, IFLoadShipList, 0);
}

void
game::interface::PrivateFunctions::addLoadCurrentTurn(Session& session, interpreter::BytecodeObject& bco, int player)
{
    afl::data::IntegerValue iv(player);
    bco.addPushLiteral(&iv);
    call(session, bco, IFLoadCurrentTurn, 1);
}

void
game::interface::PrivateFunctions::addPostprocessCurrentTurn(Session& session, interpreter::BytecodeObject& bco, int player)
{
    afl::data::IntegerValue iv(player);
    bco.addPushLiteral(&iv);
    call(session, bco, IFPostprocessCurrentTurn, 1);
}

/*
 *  Private
 */

void
game::interface::PrivateFunctions::IFTakeRoot(Session& /*session*/, interpreter::Process& proc, interpreter::Arguments& args)
{
    /*
     *  Logic:
     *    - suspend process
     *    - post task into browser session to load the Root (Task1)
     *    - callback in browser that retrieves the Root and posts it into the game (Task2)
     *    - call Session::setRoot() and resume process (Task3)
     */

    // Parse parameters
    args.checkArgumentCount(2);
    GameValue_t* pgv = dynamic_cast<GameValue_t*>(args.getNext());
    BrowserValue_t* pbv = dynamic_cast<BrowserValue_t*>(args.getNext());
    if (pgv == 0 || pbv == 0) {
        return;
    }

    // Extract values; GenericValue constifies its content.
    RequestSender<Session>& gameSender = const_cast<RequestSender<Session>&>(pgv->get());
    RequestSender<game::browser::Session>& browserSender = const_cast<RequestSender<game::browser::Session>&>(pbv->get());

    // Task in game session: set root and resume process
    // The process is identified by process Id.
    // In theory, it's possible that someone kills the process behind our back (e.g. while the browser task is interacting with the user).
    class Task3 : public util::Request<Session> {
     public:
        Task3(Ptr<Root> root, uint32_t processId)
            : m_root(root), m_processId(processId)
            { }
        void handle(Session& session)
            {
                session.setRoot(m_root);
                if (interpreter::Process* p = session.processList().findProcessById(m_processId)) {
                    session.processList().continueProcess(*p);
                    session.runScripts();
                }
            }
     private:
        Ptr<Root> m_root;
        uint32_t m_processId;
    };

    // Task in browser session: take root and forward to game (and mark Task1 done)
    class Task2 : public Task_t {
     public:
        Task2(RequestSender<Session> gameSender, game::browser::Session& browserSession, uint32_t processId)
            : m_gameSender(gameSender), m_browserSession(browserSession), m_processId(processId)
            { }
        void call()
            {
                m_gameSender.postNewRequest(new Task3(m_browserSession.browser().getSelectedRoot(), m_processId));
                m_browserSession.finishTask();
            }
     private:
        RequestSender<Session> m_gameSender;
        game::browser::Session& m_browserSession;
        uint32_t m_processId;
    };

    // Task in browser session: load root and schedule callback Task2
    class Task1 : public util::Request<game::browser::Session> {
     public:
        Task1(RequestSender<Session> gameSender, uint32_t processId)
            : m_gameSender(gameSender), m_processId(processId)
            { }
        void handle(game::browser::Session& session)
            { session.addTask(session.browser().loadChildRoot(std::auto_ptr<Task_t>(new Task2(m_gameSender, session, m_processId)))); }
     private:
        RequestSender<Session> m_gameSender;
        uint32_t m_processId;
    };

    proc.suspend(std::auto_ptr<Task_t>());
    browserSender.postNewRequest(new Task1(gameSender, proc.getProcessId()));
}

void
game::interface::PrivateFunctions::IFMakeGame(Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    session.setGame(new Game());
}

void
game::interface::PrivateFunctions::IFMakeShipList(Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    session.setShipList(new ShipList());
}

void
game::interface::PrivateFunctions::IFLoadShipList(Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);

    Root& r = game::actions::mustHaveRoot(session);
    ShipList& sl = game::actions::mustHaveShipList(session);
    proc.suspend(r.specificationLoader().loadShipList(sl, r, makeStatusTask(session, proc, "LoadShipList")));
}

void
game::interface::PrivateFunctions::IFLoadCurrentTurn(Session& session, interpreter::Process& proc, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    int playerNr;
    if (!interpreter::checkIntegerArg(playerNr, args.getNext(), 1, game::MAX_PLAYERS)) {
        return;
    }

    Root& r = game::actions::mustHaveRoot(session);
    Game& g = game::actions::mustHaveGame(session);
    Ptr<TurnLoader> tl = r.getTurnLoader();
    if (tl.get() == 0) {
        throw Error("No TurnLoader");
    }

    proc.suspend(tl->loadCurrentTurn(g, playerNr, r, session, makeStatusTask(session, proc, "LoadCurrentTurn")));
}

void
game::interface::PrivateFunctions::IFPostprocessCurrentTurn(Session& session, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    int playerNr;
    if (!interpreter::checkIntegerArg(playerNr, args.getNext(), 1, game::MAX_PLAYERS)) {
        return;
    }

    session.log().write(LogListener::Info, LOG_NAME, session.translator()("Compiling starchart..."));

    Root& r = game::actions::mustHaveRoot(session);
    Game& g = game::actions::mustHaveGame(session);
    g.setViewpointPlayer(playerNr);

    if (r.userConfiguration()[UserConfiguration::Team_AutoSync]()) {
        g.synchronizeTeamsFromAlliances();
    }
    if (r.userConfiguration()[UserConfiguration::Team_SyncTransfer]()) {
        g.teamSettings().synchronizeDataTransferConfigurationFromTeams();
    }

    Object::Playability playability;
    PlayerSet_t commandPlayers;
    PlayerSet_t localDataPlayers;
    if (r.getPossibleActions().contains(Root::aLoadEditable) && !r.userConfiguration()[UserConfiguration::Game_ReadOnly]()) {
        if (r.userConfiguration()[UserConfiguration::Game_Finished]()) {
            // Finished game
            playability = Object::ReadOnly;
        } else {
            // Active game
            playability = Object::Playable;
            commandPlayers += playerNr;
        }
        localDataPlayers += playerNr;
    } else {
        // View only
        playability = Object::ReadOnly;
    }

    g.currentTurn().setCommandPlayers(commandPlayers);
    g.currentTurn().setLocalDataPlayers(localDataPlayers);
    session.postprocessTurn(g.currentTurn(), PlayerSet_t(playerNr), PlayerSet_t(playerNr), playability);
    g.currentTurn().alliances().postprocess();

    game::sim::initSimulatorSession(session);

    // Load VM
    try {
        loadVM(session, playerNr);
    }
    catch (std::exception& e) {
        session.log().write(LogListener::Warn, LOG_NAME, session.translator()("Unable to load scripts and auto-tasks"), e);
    }
    terminateUnusableAutoTasks(session);
}
