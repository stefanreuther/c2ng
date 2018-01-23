/**
  *  \file game/session.cpp
  */

#include "game/session.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/time.hpp"
#include "game/game.hpp"
#include "game/interface/beamfunction.hpp"
#include "game/interface/cargofunctions.hpp"
#include "game/interface/drawingfunction.hpp"
#include "game/interface/enginefunction.hpp"
#include "game/interface/friendlycodefunction.hpp"
#include "game/interface/globalcommands.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/interface/globalfunctions.hpp"
#include "game/interface/hullfunction.hpp"
#include "game/interface/ionstormfunction.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/minefieldfunction.hpp"
#include "game/interface/missionfunction.hpp"
#include "game/interface/planetfunction.hpp"
#include "game/interface/playerfunction.hpp"
#include "game/interface/pluginfunction.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/shipfunction.hpp"
#include "game/interface/simplefunction.hpp"
#include "game/interface/simpleprocedure.hpp"
#include "game/interface/torpedofunction.hpp"
#include "game/interface/ufofunction.hpp"
#include "game/interface/vcrfunction.hpp"
#include "game/root.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/process.hpp"
#include "interpreter/processlist.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"
#include "game/interface/explosionfunction.hpp"
#include "game/interface/commandinterface.hpp"

namespace {
    /** Maximum number of user files. */
    const size_t MAX_SCRIPT_FILES = 101;

    // /** Compile expression, simple interface. Compiles the expression into a byte-code object.
    //     Returns the byte-code object, null on failure. This is used when an expression is used
    //     behind a regular UI function, where it doesn't really matter what kind of error, if any,
    //     we've encountered.

    //     \todo This would be a nice place to implement caching. */
    interpreter::BCOPtr_t compileExpression(const String_t& expr, const interpreter::CompilationContext& cc)
    {
        // ex int/simple.h:compileExpression
        // FIXME: can we find a better home for this function?
        try {
            interpreter::Tokenizer tok(expr);
            if (tok.getCurrentToken() == tok.tEnd) {
                return 0; /* empty expression */
            }

            std::auto_ptr<interpreter::expr::Node> expr(interpreter::expr::Parser(tok).parse());
            if (tok.getCurrentToken() != tok.tEnd) {
                return 0; /* expression incorrectly terminated */
            }

            interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
            bco->setIsProcedure(false);
            expr->compileValue(*bco, cc);
            bco->relocate();
            return bco.asPtr();
        }
        catch (interpreter::Error& e) {
            return 0;
        }
    }
}

game::Session::Session(afl::string::Translator& tx, afl::io::FileSystem& fs)
    : m_log(),
      m_translator(tx),
      m_root(),
      m_shipList(),
      m_game(),
      m_uiPropertyStack(),
      m_world(m_log, fs),
      m_rng(afl::sys::Time::getTickCounter()),
      m_plugins(tx, m_log),
      m_extra(),
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

util::plugin::Manager&
game::Session::plugins()
{
    return m_plugins;
}

game::ExtraContainer<game::Session>&
game::Session::extra()
{
    return m_extra;
}


// FIXME: retire
// // /** Execute a file, simple interface. */
// bool
// game::Session::executeFile(afl::io::Stream& file)
// {
//     // ex int/simple.cc:executeFile
//     bool result;
//     try {
//         // FIXME: port this
//         //         // Figure out contexts
//         //         ptr_vector<IntContext> ctx;
//         //         IntUserInterfaceBinding::get().enumContexts(ctx);

//         // Create process
//         interpreter::ProcessList& processList = world().processList();
//         interpreter::Process& proc = processList.create(world(),
//                                                         afl::string::Format(translator().translateString("File: %s").c_str(), file.getName()));
//         //         exec.pushContextsFrom(ctx);
//         //         exec.markContextTOS();

//         // Run
//         proc.pushFrame(world().compileFile(file), false);
//         uint32_t pgid = processList.allocateProcessGroup();
//         processList.resumeProcess(proc, pgid);
//         processList.startProcessGroup(pgid);
//         processList.run();
//         if (proc.getState() == interpreter::Process::Failed) {
//             // Log exception
//             logError(proc.getError());
//         }
//         processList.removeTerminatedProcesses();
//         result = true;
//     }
//     catch (interpreter::Error& e) {
//         // Script error
//         logError(e);
//         result = false;
//     }

//     // Update
//     notifyListeners();
//     return result;
// }

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

afl::data::Value*
game::Session::evaluate(Scope scope, int id, String_t expr)
{
    // Compiler
    interpreter::BCOPtr_t bco = compileExpression(expr, interpreter::CompilationContext(m_world));
    if (bco.get() == 0) {
        return 0;
    }

    // Create process
    interpreter::Process proc(m_world, "Session.evaluate", 0);

    // Create context
    switch (scope) {
     case Ship:
        if (m_game.get() != 0 && m_root.get() != 0 && m_shipList.get() != 0 && m_game->currentTurn().universe().ships().get(id) != 0) {
            proc.pushNewContext(new game::interface::ShipContext(id, *this, *m_root, *m_game, *m_shipList));
        }
        break;
     case Base:
     case Planet:
        if (m_game.get() != 0 && m_root.get() != 0 && m_game->currentTurn().universe().planets().get(id) != 0) {
            proc.pushNewContext(new game::interface::PlanetContext(id, *this, *m_root, *m_game));
        }
        break;
    }

    // Populate process group
    proc.pushFrame(*bco, true);
    if (proc.runTemporary()) {
        return afl::data::Value::cloneOf(proc.getResult());
    } else {
        return 0;
    }
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
    m_world.setNewGlobalValue("BEAM",          new game::interface::BeamFunction(*this));
    m_world.setNewGlobalValue("CADD",          new game::interface::SimpleFunction(*this, game::interface::IFCAdd));
    // defineGlobalValue("CC$NOTIFYCONFIRMED", new IntCCNotifyConfirmed());
    m_world.setNewGlobalValue("CCOMPARE",      new game::interface::SimpleFunction(*this, game::interface::IFCCompare));
    m_world.setNewGlobalValue("CDIV",          new game::interface::SimpleFunction(*this, game::interface::IFCDiv));
    m_world.setNewGlobalValue("CEXTRACT",      new game::interface::SimpleFunction(*this, game::interface::IFCExtract));
    m_world.setNewGlobalValue("CFG",           new game::interface::SimpleFunction(*this, game::interface::IFCfg));
    m_world.setNewGlobalValue("CMUL",          new game::interface::SimpleFunction(*this, game::interface::IFCMul));
    m_world.setNewGlobalValue("CREMOVE",       new game::interface::SimpleFunction(*this, game::interface::IFCRemove));
    m_world.setNewGlobalValue("CSUB",          new game::interface::SimpleFunction(*this, game::interface::IFCSub));
    m_world.setNewGlobalValue("DISTANCE",      new game::interface::SimpleFunction(*this, game::interface::IFDistance));
    m_world.setNewGlobalValue("ENGINE",        new game::interface::EngineFunction(*this));
    m_world.setNewGlobalValue("EXPLOSION",     new game::interface::ExplosionFunction(*this));
    m_world.setNewGlobalValue("FORMAT",        new game::interface::SimpleFunction(*this, game::interface::IFFormat));
    m_world.setNewGlobalValue("FCODE",         new game::interface::FriendlyCodeFunction(*this));
    m_world.setNewGlobalValue("GETCOMMAND",    new game::interface::SimpleFunction(*this, game::interface::IFGetCommand));
    m_world.setNewGlobalValue("HULL",          new game::interface::HullFunction(*this));
    // defineGlobalValue("INMSG",              new IntSimpleIndexableValue(IFInmsgDim,     IFInmsgGet,          IFInmsgMake));
    m_world.setNewGlobalValue("ITERATOR",      new game::interface::SimpleFunction(*this, game::interface::IFIterator));
    m_world.setNewGlobalValue("LAUNCHER",      new game::interface::TorpedoFunction(true, *this));
    m_world.setNewGlobalValue("MARKER" ,       new game::interface::DrawingFunction(*this));
    m_world.setNewGlobalValue("MINEFIELD",     new game::interface::MinefieldFunction(*this));
    m_world.setNewGlobalValue("MISSION",       new game::interface::MissionFunction(*this));
    m_world.setNewGlobalValue("PLANET",        new game::interface::PlanetFunction(*this));
    m_world.setNewGlobalValue("PLANETAT",      new game::interface::SimpleFunction(*this, game::interface::IFPlanetAt));
    m_world.setNewGlobalValue("PLAYER",        new game::interface::PlayerFunction(*this));
    m_world.setNewGlobalValue("PREF",          new game::interface::SimpleFunction(*this, game::interface::IFPref));
    m_world.setNewGlobalValue("RANDOM",        new game::interface::SimpleFunction(*this, game::interface::IFRandom));
    m_world.setNewGlobalValue("RANDOMFCODE",   new game::interface::SimpleFunction(*this, game::interface::IFRandomFCode));
    m_world.setNewGlobalValue("SHIP",          new game::interface::ShipFunction(*this));
    m_world.setNewGlobalValue("STORM",         new game::interface::IonStormFunction(*this));
    m_world.setNewGlobalValue("SYSTEM.PLUGIN", new game::interface::PluginFunction(*this));
    m_world.setNewGlobalValue("TORPEDO",       new game::interface::TorpedoFunction(false, *this));
    m_world.setNewGlobalValue("TRANSLATE",     new game::interface::SimpleFunction(*this, game::interface::IFTranslate));
    m_world.setNewGlobalValue("TRUEHULL",      new game::interface::SimpleFunction(*this, game::interface::IFTruehull));
    m_world.setNewGlobalValue("UFO",           new game::interface::UfoFunction(*this));
    m_world.setNewGlobalValue("VCR",           new game::interface::VcrFunction(*this));

    m_world.setNewGlobalValue("RADD",          new game::interface::SimpleFunction(*this, game::interface::IFRAdd));
    m_world.setNewGlobalValue("RALIGN",        new game::interface::SimpleFunction(*this, game::interface::IFRAlign));
    m_world.setNewGlobalValue("RLEN",          new game::interface::SimpleFunction(*this, game::interface::IFRLen));
    m_world.setNewGlobalValue("RLINK",         new game::interface::SimpleFunction(*this, game::interface::IFRLink));
    m_world.setNewGlobalValue("RMID",          new game::interface::SimpleFunction(*this, game::interface::IFRMid));
    m_world.setNewGlobalValue("RSTRING",       new game::interface::SimpleFunction(*this, game::interface::IFRString));
    m_world.setNewGlobalValue("RSTYLE",        new game::interface::SimpleFunction(*this, game::interface::IFRStyle));
    m_world.setNewGlobalValue("RXML",          new game::interface::SimpleFunction(*this, game::interface::IFRXml));

    m_world.setNewGlobalValue("ADDCOMMAND",       new game::interface::SimpleProcedure(*this, game::interface::IFAddCommand));
    m_world.setNewGlobalValue("ADDCONFIG",        new game::interface::SimpleProcedure(*this, game::interface::IFAddConfig));
    m_world.setNewGlobalValue("ADDFCODE",         new game::interface::SimpleProcedure(*this, game::interface::IFAddFCode));
    m_world.setNewGlobalValue("ADDPREF",          new game::interface::SimpleProcedure(*this, game::interface::IFAddPref));
    // m_world.setNewGlobalValue("AUTHPLAYER",       new game::interface::SimpleProcedure(*this, game::interface::IFAuthPlayer));
    // m_world.setNewGlobalValue("CC$NOTIFY",        new game::interface::SimpleProcedure(*this, game::interface::IFCCNotify));
    m_world.setNewGlobalValue("CC$SELECTIONEXEC", new game::interface::SimpleProcedure(*this, game::interface::IFCCSelectionExec));
    m_world.setNewGlobalValue("CREATECONFIGOPTION", new game::interface::SimpleProcedure(*this, game::interface::IFCreateConfigOption));
    m_world.setNewGlobalValue("CREATEPREFOPTION", new game::interface::SimpleProcedure(*this, game::interface::IFCreatePrefOption));
    m_world.setNewGlobalValue("DELETECOMMAND",    new game::interface::SimpleProcedure(*this, game::interface::IFDeleteCommand));
    m_world.setNewGlobalValue("NEWCIRCLE",        new game::interface::SimpleProcedure(*this, game::interface::IFNewCircle));
    m_world.setNewGlobalValue("NEWLINE",          new game::interface::SimpleProcedure(*this, game::interface::IFNewLine));
    m_world.setNewGlobalValue("NEWLINERAW",       new game::interface::SimpleProcedure(*this, game::interface::IFNewLineRaw));
    m_world.setNewGlobalValue("NEWMARKER",        new game::interface::SimpleProcedure(*this, game::interface::IFNewMarker));
    m_world.setNewGlobalValue("NEWRECTANGLE",     new game::interface::SimpleProcedure(*this, game::interface::IFNewRectangle));
    m_world.setNewGlobalValue("NEWRECTANGLERAW",  new game::interface::SimpleProcedure(*this, game::interface::IFNewRectangleRaw));
    // m_world.setNewGlobalValue("SAVEGAME",         new game::interface::SimpleProcedure(*this, game::interface::IFSaveGame));
    // m_world.setNewGlobalValue("SELECTIONLOAD",    new game::interface::SimpleProcedure(*this, game::interface::IFSelectionLoad));
    // m_world.setNewGlobalValue("SELECTIONSAVE",    new game::interface::SimpleProcedure(*this, game::interface::IFSelectionSave));
    m_world.setNewGlobalValue("HISTORY.SHOWTURN", new game::interface::SimpleProcedure(*this, game::interface::IFHistoryShowTurn));

    // Add global context (=properties)
    m_world.addNewGlobalContext(new game::interface::GlobalContext(*this));

    // Configure files
    m_world.fileTable().setMaxFiles(MAX_SCRIPT_FILES);
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
        m_world.setLocalLoadDirectory(&m_root->gameDirectory());
    } else {
        m_world.setLocalLoadDirectory(0);
    }
}

void
game::Session::updateMap()
{
    if (m_root.get() != 0 && m_game.get() != 0) {
        m_game->currentTurn().universe().config().initFromConfiguration(m_root->hostVersion(), m_root->hostConfiguration(), m_root->userConfiguration());
    }
}
