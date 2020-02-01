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
#include "game/map/object.hpp"
#include "game/interface/markingfunctions.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/interface/referencelistcontext.hpp"
#include "interpreter/taskeditor.hpp"
#include "game/interface/notificationfunctions.hpp"
#include "game/interface/inboxfunction.hpp"

namespace {
    using afl::string::Format;

    /** Maximum number of user files.

        - PCC1: 20, defining a range of 1..20 for user, 0 for internal use.
        - PCC2: 101, defining a range of allowing 0..100, which are all accessible to the user
          (but slot 0 is never returned by FreeFile()) */
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
      m_editableAreas(),
      m_world(m_log, fs),
      m_rng(afl::sys::Time::getTickCounter()),
      m_plugins(tx, m_log),
      m_authCache(),
      m_extra(),
      m_notifications(m_world.processList()),
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

game::interface::NotificationStore&
game::Session::notifications()
{
    return m_notifications;
}

const game::interface::NotificationStore&
game::Session::notifications() const
{
    return m_notifications;
}

void
game::Session::setEditableAreas(AreaSet_t set)
{
    m_editableAreas = set;
}

game::Session::AreaSet_t
game::Session::getEditableAreas() const
{
    return m_editableAreas;
}

afl::base::Ptr<interpreter::TaskEditor>
game::Session::getAutoTaskEditor(Id_t id, interpreter::Process::ProcessKind kind, bool create)
{
    // ex getAutoTaskForObject
    using interpreter::Process;
    using interpreter::TaskEditor;

    // Need to have a game
    if (m_game.get() == 0) {
        return 0;
    }

    // Determine object
    game::map::Object* obj = 0;
    switch (kind) {
     case Process::pkShipTask:
        obj = m_game->currentTurn().universe().ships().get(id);
        break;
     case Process::pkPlanetTask:
     case Process::pkBaseTask:
        obj = m_game->currentTurn().universe().planets().get(id);
        break;
     case Process::pkDefault:
        // Invalid
        break;
    }
    if (obj == 0) {
        return 0;
    }

    // Find the process
    Process* proc = m_world.processList().getProcessByObject(obj, kind);
    if (proc == 0 && create) {
        // Create process
        String_t fmt = (kind == Process::pkShipTask
                        ? m_translator("Auto Task Ship %d")
                        : kind == Process::pkPlanetTask
                        ? m_translator("Auto Task Planet %d")
                        : m_translator("Auto Task Starbase %d"));
        proc = &m_world.processList().create(m_world, afl::string::Format(fmt, id));

        // Place in appropriate context
        // (Note that this fails if the Session is not fully-populated, e.g. has no ship list.)
        interpreter::Context* ctx = 0;
        if (kind == Process::pkShipTask) {
            ctx = game::interface::ShipContext::create(id, *this);
        } else {
            ctx = game::interface::PlanetContext::create(id, *this);
        }
        if (ctx != 0) {
            proc->pushNewContext(ctx);
        }
        proc->markContextTOS();

        // Mark as auto-task
        proc->setProcessKind(kind);
    }
    if (proc == 0) {
        return 0;
    }

    // Try to create (re-use) editor
    try {
        TaskEditor* ed = dynamic_cast<TaskEditor*>(proc->getFreezer());
        if (ed != 0) {
            return ed;
        } else {
            return new TaskEditor(*proc);
        }
    }
    catch (interpreter::Error& err) {
        logError(err);
        return 0;
    }
}


void
game::Session::releaseAutoTaskEditor(afl::base::Ptr<interpreter::TaskEditor>& ptr)
{
    if (ptr.get() != 0) {
        // Remember the process
        interpreter::Process& proc = ptr->process();

        // Clear the TaskEditor. This will make the process runnable.
        ptr.reset();

        // Run the process
        if (proc.getFreezer() == 0) {
            interpreter::ProcessList& pl = world().processList();
            uint32_t pgid = pl.allocateProcessGroup();
            pl.resumeProcess(proc, pgid);
            pl.startProcessGroup(pgid);
            pl.run();
        }
    }
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

game::AuthCache&
game::Session::authCache()
{
    return m_authCache;
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

bool
game::Session::getReferenceName(Reference ref, ObjectName which, String_t& result)
{
    // ex PCC1.x ThingName
    // FIXME: can we find a better location for this function
    // FIXME: cannot currently be const because InterpreterInterface is not const
    switch (ref.getType()) {
     case Reference::Null:
     case Reference::Special:
        return false;

     case Reference::Player:
        // Report reference name plus player name
        if (Root* r = m_root.get()) {
            if (const Player* p = r->playerList().get(ref.getId())) {
                if (which == PlainName) {
                    result = p->getName(Player::ShortName);
                } else {
                    result = ref.toString(m_translator);
                    result += ": ";
                    result += p->getName(Player::ShortName);
                }
                return true;
            }
        }
        return false;

     case Reference::MapLocation:
        // Reference name is good enough.
        result = ref.toString(m_translator);
        return true;

     case Reference::Ship:
     case Reference::Planet:
     case Reference::Starbase:
     case Reference::Storm:
     case Reference::Minefield:
     case Reference::Ufo:
        // Return normal object's name.
        if (Game* g = m_game.get()) {
            if (Turn* t = g->getViewpointTurn().get()) {
                if (const game::map::Object* obj = t->universe().getObject(ref)) {
                    if (ref.getType() == Reference::Starbase && which != PlainName) {
                        // Special case: report the reference name plus object's name, if any.
                        // This allows a starbase reference to be shown as "Starbase #123: Melmac".
                        result = ref.toString(m_translator);
                        result += obj->getName(PlainName, m_translator, *this);
                        if (which == DetailedName) {
                            String_t comment = this->getComment(Planet, ref.getId());
                            if (!comment.empty()) {
                                result += ": ";
                                result += comment;
                            }
                        }
                        return true;
                    } else {
                        result = obj->getName(which, m_translator, *this);
                        return !result.empty();
                    }
                }
            }
        }
        return false;

     case Reference::Hull:
     case Reference::Engine:
     case Reference::Beam:
     case Reference::Torpedo:
        // Report the reference name plus component name.
        if (const game::spec::ShipList* shipList = m_shipList.get()) {
            if (const game::spec::Component* p = shipList->getComponent(ref)) {
                if (which == PlainName) {
                    result = p->getName(shipList->componentNamer());
                } else {
                    result = ref.toString(m_translator);
                    result += ": ";
                    result += p->getName(shipList->componentNamer());
                }
                return true;
            }
        }
        return false;
    }
    return false;
}

bool
game::Session::save()
{
    // Check environment
    afl::base::Ptr<Root> pRoot = getRoot();
    afl::base::Ptr<Game> pGame = getGame();
    if (pRoot.get() == 0 || pGame.get() == 0) {
        return false;
    }

    afl::base::Ptr<TurnLoader> pLoader = pRoot->getTurnLoader();
    if (pLoader.get() == 0) {
        return false;
    }

    pLoader->saveCurrentTurn(pGame->currentTurn(), *pGame, pGame->getViewpointPlayer(), *pRoot, *this);
    return true;
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
        // ex shipint.pas:EvalShip
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
game::Session::hasTask(Scope scope, int id)
{
    // FIXME: consider changing the signature to take an object,
    // to avoid the reverse-mapping into a universe.
    if (const Game* g = m_game.get()) {
        using interpreter::Process;
        const game::map::Universe& univ = g->currentTurn().universe();
        const interpreter::ProcessList& list = world().processList();
        switch (scope) {
         case Ship:
            return list.getProcessByObject(univ.ships().get(id), Process::pkShipTask) != 0;
         case Planet:
            return list.getProcessByObject(univ.planets().get(id), Process::pkPlanetTask) != 0;
         case Base:
            return list.getProcessByObject(univ.planets().get(id), Process::pkBaseTask) != 0;
        }
    }
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
    m_world.setNewGlobalValue("AUTOTASK",      new game::interface::SimpleFunction(*this, game::interface::IFAutoTask));
    m_world.setNewGlobalValue("BEAM",          new game::interface::BeamFunction(*this));
    m_world.setNewGlobalValue("CADD",          new game::interface::SimpleFunction(*this, game::interface::IFCAdd));
    m_world.setNewGlobalValue("CC$NOTIFYCONFIRMED", new game::interface::NotifyConfirmedFunction(*this));
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
    m_world.setNewGlobalValue("INMSG",         new game::interface::InboxFunction(*this));
    m_world.setNewGlobalValue("ISSPECIALFCODE", new game::interface::SimpleFunction(*this, game::interface::IFIsSpecialFCode));
    m_world.setNewGlobalValue("ITERATOR",      new game::interface::SimpleFunction(*this, game::interface::IFIterator));
    m_world.setNewGlobalValue("LAUNCHER",      new game::interface::TorpedoFunction(true, *this));
    m_world.setNewGlobalValue("MARKER" ,       new game::interface::DrawingFunction(*this));
    m_world.setNewGlobalValue("MINEFIELD",     new game::interface::MinefieldFunction(*this));
    m_world.setNewGlobalValue("MISSION",       new game::interface::MissionFunction(*this));
    m_world.setNewGlobalValue("OBJECTISAT",    new game::interface::SimpleFunction(*this, game::interface::IFObjectIsAt));
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

    m_world.setNewGlobalValue("REFERENCE",         new game::interface::SimpleFunction(*this, game::interface::IFReference));
    m_world.setNewGlobalValue("LOCATIONREFERENCE", new game::interface::SimpleFunction(*this, game::interface::IFLocationReference));
    m_world.setNewGlobalValue("REFERENCELIST",     new game::interface::SimpleFunction(*this, game::interface::IFReferenceList));

    m_world.setNewGlobalValue("CC$SELREADHEADER",  new game::interface::SimpleFunction(*this, game::interface::IFCCSelReadHeader));
    m_world.setNewGlobalValue("CC$SELREADCONTENT", new game::interface::SimpleFunction(*this, game::interface::IFCCSelReadContent));
    m_world.setNewGlobalValue("CC$SELGETQUESTION", new game::interface::SimpleFunction(*this, game::interface::IFCCSelGetQuestion));
    m_world.setNewGlobalValue("SELECTIONSAVE",     new game::interface::SimpleProcedure(*this, game::interface::IFSelectionSave));

    m_world.setNewGlobalValue("ADDCOMMAND",       new game::interface::SimpleProcedure(*this, game::interface::IFAddCommand));
    m_world.setNewGlobalValue("ADDCONFIG",        new game::interface::SimpleProcedure(*this, game::interface::IFAddConfig));
    m_world.setNewGlobalValue("ADDFCODE",         new game::interface::SimpleProcedure(*this, game::interface::IFAddFCode));
    m_world.setNewGlobalValue("ADDPREF",          new game::interface::SimpleProcedure(*this, game::interface::IFAddPref));
    m_world.setNewGlobalValue("AUTHPLAYER",       new game::interface::SimpleProcedure(*this, game::interface::IFAuthPlayer));
    m_world.setNewGlobalValue("CC$NOTIFY",        new game::interface::SimpleProcedure(*this, game::interface::IFCCNotify));
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
    m_world.setNewGlobalValue("SAVEGAME",         new game::interface::SimpleProcedure(*this, game::interface::IFSaveGame));
    m_world.setNewGlobalValue("SENDMESSAGE",      new game::interface::SimpleProcedure(*this, game::interface::IFSendMessage));
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
