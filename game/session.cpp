/**
  *  \file game/session.cpp
  */

#include "game/session.hpp"
#include "game/root.hpp"
#include "game/game.hpp"
#include "game/turnloader.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/hull.hpp"
#include "interpreter/values.hpp"
#include "game/interface/planetfunction.hpp"
#include "game/interface/globalcontext.hpp"
#include "afl/sys/time.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/processlist.hpp"
#include "afl/string/format.hpp"
#include "afl/io/textfile.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/process.hpp"
#include "game/interface/shipfunction.hpp"
#include "game/interface/simplefunction.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/globalfunctions.hpp"
#include "game/interface/ionstormfunction.hpp"
#include "game/interface/enginefunction.hpp"
#include "game/interface/beamfunction.hpp"
#include "game/interface/hullfunction.hpp"
#include "game/interface/torpedofunction.hpp"
#include "game/interface/playerfunction.hpp"
#include "game/interface/minefieldfunction.hpp"
#include "game/interface/simpleprocedure.hpp"
#include "game/interface/globalcommands.hpp"

game::Session::Session(afl::string::Translator& tx, afl::io::FileSystem& fs)
    : m_log(),
      m_translator(tx),
      m_root(),
      m_shipList(),
      m_game(),
      m_uiPropertyStack(),
      m_world(m_log, fs),
      m_rng(afl::sys::Time::getTickCounter()),
      conn_hostConfigToMap(),
      conn_userConfigToMap()
{
    initWorld();
}

game::Session::~Session()
{ }

afl::sys::Log&
game::Session::log()
{
    return m_log;
}

void
game::Session::logError(const interpreter::Error& e)
{
    m_world.logError(m_log.Error, e);
}

afl::string::Translator&
game::Session::translator()
{
    return m_translator;
}

afl::base::Ptr<game::Root>
game::Session::getRoot()
{
    return m_root;
}

void
game::Session::setRoot(afl::base::Ptr<Root> root)
{
    m_root = root;
    connectSignals();
}

afl::base::Ptr<game::spec::ShipList>
game::Session::getShipList()
{
    return m_shipList;
}

void
game::Session::setShipList(afl::base::Ptr<game::spec::ShipList> shipList)
{
    m_shipList = shipList;
    connectSignals();
}

afl::base::Ptr<game::Game>
game::Session::getGame()
{
    return m_game;
}

void
game::Session::setGame(afl::base::Ptr<Game> game)
{
    m_game = game;
    connectSignals();
}

game::interface::UserInterfacePropertyStack&
game::Session::uiPropertyStack()
{
    return m_uiPropertyStack;
}

const game::interface::UserInterfacePropertyStack&
game::Session::uiPropertyStack() const
{
    return m_uiPropertyStack;
}

interpreter::World&
game::Session::world()
{
    return m_world;
}

game::InterpreterInterface&
game::Session::interface()
{
    return *this;
}

util::RandomNumberGenerator&
game::Session::rng()
{
    return m_rng;
}

// /** Execute a file, simple interface. */
bool
game::Session::executeFile(afl::io::Stream& file)
{
    // ex int/simple.cc:executeFile
    bool result;
    try {
        // FIXME: port this
        //         // Figure out contexts
        //         ptr_vector<IntContext> ctx;
        //         IntUserInterfaceBinding::get().enumContexts(ctx);

        // Create process
        interpreter::ProcessList& processList = world().processList();
        interpreter::Process& proc = processList.create(world(),
                                                        afl::string::Format(translator().translateString("File: %s").c_str(), file.getName()));
        //         exec.pushContextsFrom(ctx);
        //         exec.markContextTOS();

        // Generate compilation objects
        afl::io::TextFile tf(file);
        interpreter::FileCommandSource fcs(tf);
        interpreter::BCORef_t nbco = new interpreter::BytecodeObject();
        nbco->setFileName(file.getName());

        // Compile
        try {
            interpreter::StatementCompiler sc(fcs);
            interpreter::DefaultStatementCompilationContext scc(world());
            scc.withFlag(scc.LocalContext)
                .withFlag(scc.ExpressionsAreStatements)
                .withFlag(scc.LinearExecution);
            sc.compileList(*nbco, scc);
            sc.finishBCO(*nbco, scc);
        }
        catch (interpreter::Error& e) {
            fcs.addTraceTo(e, translator());
            throw;
        }

        // Run
        proc.pushFrame(nbco, false);
        uint32_t pgid = processList.allocateProcessGroup();
        processList.resumeProcess(proc, pgid);
        processList.startProcessGroup(pgid);
        processList.run();
        if (proc.getState() == interpreter::Process::Failed) {
            // Log exception
            logError(proc.getError());
        }
        processList.removeTerminatedProcesses();
        result = true;
    }
    catch (interpreter::Error& e) {
        // Script error
        logError(e);
        result = false;
    }

    // Update
    notifyListeners();
    return result;
}

void
game::Session::notifyListeners()
{
    if (Root* r = m_root.get()) {
        r->notifyListeners();
    }
    if (Game* g = m_game.get()) {
        g->notifyListeners();
    }
    m_world.notifyListeners();
}

bool
game::Session::evaluate(Scope /*scope*/, int /*id*/, String_t /*expr*/)
{
    // FIXME: implement me
    return false;
}

String_t
game::Session::getComment(Scope scope, int id)
{
    switch (scope) {
     case Ship:
        return interpreter::toString(m_world.shipProperties().get(id, interpreter::World::sp_Comment), false);

     case Planet:
     case Base:
        return interpreter::toString(m_world.planetProperties().get(id, interpreter::World::pp_Comment), false);
    }
    return String_t();
}

bool
game::Session::hasTask(Scope /*scope*/, int /*id*/)
{
    // FIXME: implement me
    return false;
}

bool
game::Session::getHullShortName(int nr, String_t& out)
{
    if (const game::spec::ShipList* list = m_shipList.get()) {
        if (const game::spec::Hull* hull = list->hulls().get(nr)) {
            out = hull->getShortName(list->componentNamer());
            return true;
        }
    }
    return false;
}

bool
game::Session::getPlayerAdjective(int nr, String_t& out)
{
    if (const Root* root = m_root.get()) {
        if (const Player* player = root->playerList().get(nr)) {
            out = player->getName(Player::AdjectiveName);
            return true;
        }
    }
    return false;
}

void
game::Session::initWorld()
{
    // ex initInterpreterGameInterface()
    // // Add global variables
    m_world.setNewGlobalValue("BEAM", new game::interface::BeamFunction(*this));
    // defineGlobalValue("CADD",           new IntSimpleIndexableValue(0,              IFCAdd,              0));
    // defineGlobalValue("CC$NOTIFYCONFIRMED", new IntCCNotifyConfirmed());            
    // defineGlobalValue("CCOMPARE",       new IntSimpleIndexableValue(0,              IFCCompare,          0));
    // defineGlobalValue("CDIV",           new IntSimpleIndexableValue(0,              IFCDiv,              0));
    // defineGlobalValue("CEXTRACT",       new IntSimpleIndexableValue(0,              IFCExtract,          0));
    // defineGlobalValue("CFG",            new IntSimpleIndexableValue(0,              IFCfgGet,            0));
    // defineGlobalValue("CMUL",           new IntSimpleIndexableValue(0,              IFCMul,              0));
    // defineGlobalValue("CREMOVE",        new IntSimpleIndexableValue(0,              IFCRemove,           0));
    // defineGlobalValue("CSUB",           new IntSimpleIndexableValue(0,              IFCSub,              0));
    // defineGlobalValue("DISTANCE",       new IntSimpleIndexableValue(0,              IFDistanceGet,       0));
    m_world.setNewGlobalValue("ENGINE", new game::interface::EngineFunction(*this));
    m_world.setNewGlobalValue("FORMAT", new game::interface::SimpleFunction(*this, game::interface::IFFormat));
    m_world.setNewGlobalValue("HULL", new game::interface::HullFunction(*this));
    // defineGlobalValue("INMSG",          new IntSimpleIndexableValue(IFInmsgDim,     IFInmsgGet,          IFInmsgMake));
    // defineGlobalValue("ISSPECIALFCODE", new IntSimpleIndexableValue(0,              IFIsSpecialFCodeGet, 0));
    m_world.setNewGlobalValue("ITERATOR", new game::interface::SimpleFunction(*this, game::interface::IFIterator));
    m_world.setNewGlobalValue("LAUNCHER", new game::interface::TorpedoFunction(true, *this));
    // defineGlobalValue("MARKER" ,        new IntSimpleIndexableValue(0,              0,                   IFDrawingMake));
    m_world.setNewGlobalValue("MINEFIELD", new game::interface::MinefieldFunction(*this));
    m_world.setNewGlobalValue("PLANET", new game::interface::PlanetFunction(*this));
    // defineGlobalValue("PLANETAT",       new IntSimpleIndexableValue(0,              IFPlanetAtGet,       0));
    // defineGlobalValue("PLANETNAME",     new IntSimpleIndexableValue(IFPlanetDim,    IFPlanetNameGet,     0));
    m_world.setNewGlobalValue("PLAYER", new game::interface::PlayerFunction(*this));
    m_world.setNewGlobalValue("RANDOM", new game::interface::SimpleFunction(*this, game::interface::IFRandom));
    // defineGlobalValue("RANDOMFCODE",    new IntSimpleIndexableValue(0,              IFRandomFCode,       0));
    m_world.setNewGlobalValue("SHIP", new game::interface::ShipFunction(*this));
    // defineGlobalValue("SHIPNAME",       new IntSimpleIndexableValue(IFShipDim,      IFShipNameGet,       0));
    m_world.setNewGlobalValue("STORM", new game::interface::IonStormFunction(*this));
    // defineGlobalValue("SYSTEM.PLUGIN",  new IntSimpleIndexableValue(0,              IFSystemPlugin,      0));
    m_world.setNewGlobalValue("TORPEDO", new game::interface::TorpedoFunction(false, *this));
    m_world.setNewGlobalValue("TRANSLATE", new game::interface::SimpleFunction(*this, game::interface::IFTranslate));
    m_world.setNewGlobalValue("TRUEHULL",  new game::interface::SimpleFunction(*this, game::interface::IFTruehull));
    // defineGlobalValue("UFO",            new IntSimpleIndexableValue(IFUfoDim,       IFUfoGet,            IFUfoMake));
    // defineGlobalValue("VCR",            new IntSimpleIndexableValue(IFVcrDim,       IFVcrGet,            IFVcrMake));

    m_world.setNewGlobalValue("RADD",    new game::interface::SimpleFunction(*this, game::interface::IFRAdd));
    m_world.setNewGlobalValue("RLEN",    new game::interface::SimpleFunction(*this, game::interface::IFRLen));
    m_world.setNewGlobalValue("RLINK",   new game::interface::SimpleFunction(*this, game::interface::IFRLink));
    m_world.setNewGlobalValue("RMID",    new game::interface::SimpleFunction(*this, game::interface::IFRMid));
    m_world.setNewGlobalValue("RSTRING", new game::interface::SimpleFunction(*this, game::interface::IFRString));
    m_world.setNewGlobalValue("RSTYLE",  new game::interface::SimpleFunction(*this, game::interface::IFRStyle));
    m_world.setNewGlobalValue("RXML",    new game::interface::SimpleFunction(*this, game::interface::IFRXml));

    // defineGlobalProcedure("ADDCOMMAND",       IFAddCommand);
    // defineGlobalProcedure("ADDCONFIG",        IFAddConfig);
    // defineGlobalProcedure("ADDFCODE",         IFAddFCode);
    // defineGlobalProcedure("AUTHPLAYER",       IFAuthPlayer);
    // defineGlobalProcedure("CC$NOTIFY",        IFCCNotify);
    // defineGlobalProcedure("CC$SELECTIONEXEC", IFCCSelectionExec);
    // defineGlobalProcedure("NEWCIRCLE",        IFNewCircle);
    // defineGlobalProcedure("NEWLINE",          IFNewLine);
    // defineGlobalProcedure("NEWLINERAW",       IFNewLineRaw);
    // defineGlobalProcedure("NEWMARKER",        IFNewMarker);
    // defineGlobalProcedure("NEWRECTANGLE",     IFNewRectangle);
    // defineGlobalProcedure("NEWRECTANGLERAW",  IFNewRectangleRaw);
    // defineGlobalProcedure("SAVEGAME",         IFSaveGame);
    // defineGlobalProcedure("SELECTIONLOAD",    IFSelectionLoad);
    // defineGlobalProcedure("SELECTIONSAVE",    IFSelectionSave);
    m_world.setNewGlobalValue("HISTORY.SHOWTURN", new game::interface::SimpleProcedure(*this, game::interface::IFHistoryShowTurn));

    // Add global context (=properties)
    m_world.addNewGlobalContext(new game::interface::GlobalContext(*this));
}

void
game::Session::connectSignals()
{
    if (m_root.get() != 0 && m_game.get() != 0) {
        conn_hostConfigToMap = m_root->hostConfiguration().sig_change.add(this, &Session::updateMap);
        conn_userConfigToMap = m_root->userConfiguration().sig_change.add(this, &Session::updateMap);
        updateMap();
    } else {
        conn_hostConfigToMap.disconnect();
        conn_userConfigToMap.disconnect();
    }

    if (m_root.get() != 0) {
        m_world.setLoadDirectory(&m_root->gameDirectory());
    } else {
        m_world.setLoadDirectory(0);
    }
}

void
game::Session::updateMap()
{
    if (m_root.get() != 0 && m_game.get() != 0) {
        m_game->currentTurn().universe().config().initFromConfiguration(m_root->hostVersion(), m_root->hostConfiguration(), m_root->userConfiguration());
    }
}
