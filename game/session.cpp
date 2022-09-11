/**
  *  \file game/session.cpp
  *  \brief Class game::Session
  */

#include "game/session.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/time.hpp"
#include "game/game.hpp"
#include "game/interface/beamfunction.hpp"
#include "game/interface/cargofunctions.hpp"
#include "game/interface/commandinterface.hpp"
#include "game/interface/drawingfunction.hpp"
#include "game/interface/enginefunction.hpp"
#include "game/interface/explosionfunction.hpp"
#include "game/interface/friendlycodefunction.hpp"
#include "game/interface/globalactioncontext.hpp"
#include "game/interface/globalcommands.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/interface/globalfunctions.hpp"
#include "game/interface/hullfunction.hpp"
#include "game/interface/inboxfunction.hpp"
#include "game/interface/ionstormfunction.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/minefieldfunction.hpp"
#include "game/interface/missionfunction.hpp"
#include "game/interface/notificationfunctions.hpp"
#include "game/interface/planetfunction.hpp"
#include "game/interface/playerfunction.hpp"
#include "game/interface/pluginfunction.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/interface/referencelistcontext.hpp"
#include "game/interface/richtextfunctions.hpp"
#include "game/interface/selectionfunctions.hpp"
#include "game/interface/shipfunction.hpp"
#include "game/interface/torpedofunction.hpp"
#include "game/interface/ufofunction.hpp"
#include "game/interface/vcrfunction.hpp"
#include "game/map/object.hpp"
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
#include "interpreter/simplefunction.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"

namespace {
    using afl::string::Format;

    /** Maximum number of user files.

        - PCC1: 20, defining a range of 1..20 for user, 0 for internal use.
        - PCC2: 101, defining a range of allowing 0..100, which are all accessible to the user
          (but slot 0 is never returned by FreeFile()) */
    const size_t MAX_SCRIPT_FILES = 101;
}

game::Session::Session(afl::string::Translator& tx, afl::io::FileSystem& fs)
    : sig_runRequest(),
      sig_connectionChange(),
      m_log(),
      m_root(),
      m_shipList(),
      m_game(),
      m_uiPropertyStack(),
      m_editableAreas(),
      m_world(m_log, tx, fs),
      m_systemInformation(),
      m_processList(),
      m_rng(afl::sys::Time::getTickCounter()),
      m_plugins(tx, m_log),
      m_authCache(),
      m_extra(),
      m_notifications(m_processList),
      conn_hostConfigToMap(),
      conn_userConfigToMap()
{
    initWorld();
}

game::Session::~Session()
{ }

void
game::Session::setRoot(afl::base::Ptr<Root> root)
{
    m_root = root;
    connectSignals();
}

void
game::Session::setShipList(afl::base::Ptr<game::spec::ShipList> shipList)
{
    m_shipList = shipList;
    connectSignals();
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
    Process* proc = processList().getProcessByObject(obj, kind);
    if (proc == 0 && create) {
        // Create process
        String_t fmt = (kind == Process::pkShipTask
                        ? translator()("Auto Task Ship %d")
                        : kind == Process::pkPlanetTask
                        ? translator()("Auto Task Planet %d")
                        : translator()("Auto Task Starbase %d"));
        proc = &processList().create(m_world, afl::string::Format(fmt, id));

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
            interpreter::ProcessList& pl = processList();
            uint32_t pgid = pl.allocateProcessGroup();
            pl.resumeProcess(proc, pgid);
            pl.startProcessGroup(pgid);
            pl.run();
        }
    }
}

game::Session::TaskStatus
game::Session::getTaskStatus(const game::map::Object* obj, interpreter::Process::ProcessKind kind, bool waitOnly) const
{
    // ex getControlScreenFrameColor (waitOnly=false), getAutoTaskFrameColor (waitOnly=true)
    using interpreter::Process;
    using game::interface::NotificationStore;
    if (waitOnly) {
        if (const Process* proc = m_processList.getProcessByObject(obj, kind)) {
            if (m_notifications.isMessageConfirmed(m_notifications.findMessageByProcessId(proc->getProcessId()))) {
                return WaitingTask;
            } else {
                return NoTask;
            }
        } else {
            return NoTask;
        }
    } else {
        const interpreter::ProcessList::Vector_t& pl = m_processList.getProcessList();
        bool any = false;
        for (size_t i = 0, n = pl.size(); i != n; ++i) {
            if (const Process* proc = pl[i]) {
                // Check for a process which is started from this object, and
                // which is currently runnable/suspended/frozen. Those are the
                // states usually assumed by auto tasks or long-running scripts.
                // Running scripts do not count here, as they are usually (but
                // not always!) temporary UI processes.
                if ((proc->getState() == Process::Runnable
                     || proc->getState() == Process::Suspended
                     || proc->getState() == Process::Frozen)
                    && proc->getInvokingObject() == obj)
                {
                    any = true;
                    if (proc->getProcessKind() == kind) {
                        // Found the auto task
                        if (m_notifications.isMessageConfirmed(m_notifications.findMessageByProcessId(proc->getProcessId()))) {
                            return WaitingTask;
                        } else {
                            return ActiveTask;
                        }
                    }
                }
            }
        }
        if (any) {
            return OtherTask;
        } else {
            return NoTask;
        }
    }
}

// Access process list.
interpreter::ProcessList&
game::Session::processList()
{
    return m_processList;
}

// Access process list (const).
const interpreter::ProcessList&
game::Session::processList() const
{
    return m_processList;
}

game::InterpreterInterface&
game::Session::interface()
{
    return *this;
}

// Access SystemInformation.
const util::SystemInformation&
game::Session::getSystemInformation() const
{
    return m_systemInformation;
}

// Set SystemInformation.
void
game::Session::setSystemInformation(const util::SystemInformation& info)
{
    m_systemInformation = info;
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
                    result = p->getName(Player::ShortName, translator());
                } else {
                    result = ref.toString(translator());
                    result += ": ";
                    result += p->getName(Player::ShortName, translator());
                }
                return true;
            }
        }
        return false;

     case Reference::MapLocation:
        // Reference name is good enough.
        result = ref.toString(translator());
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
                        result = ref.toString(translator());
                        result += ": ";
                        result += obj->getName(PlainName, translator(), *this);
                        if (which == DetailedName) {
                            String_t comment = this->getComment(Planet, ref.getId());
                            if (!comment.empty()) {
                                result += ": ";
                                result += comment;
                            }
                        }
                        return true;
                    } else {
                        result = obj->getName(which, translator(), *this);
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
                    result = ref.toString(translator());
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

void
game::Session::postprocessTurn(Turn& t, PlayerSet_t playingSet, PlayerSet_t availablePlayers, game::map::Object::Playability playability)
{
    const Game* g = m_game.get();
    const Root* r = m_root.get();
    const game::spec::ShipList* sl = m_shipList.get();
    if (g != 0 && r != 0 && sl != 0) {
        t.universe().postprocess(playingSet, availablePlayers, playability, g->mapConfiguration(), r->hostVersion(), r->hostConfiguration(),
                                 t.getTurnNumber(), *sl, translator(), log());
    }
}

std::auto_ptr<afl::base::Closure<void()> >
game::Session::save(TurnLoader::SaveOptions_t opts, std::auto_ptr<afl::base::Closure<void(bool)> > then)
{
    std::auto_ptr<afl::base::Closure<void()> > result;

    // Check environment
    afl::base::Ptr<Root> pRoot = getRoot();
    afl::base::Ptr<Game> pGame = getGame();
    if (pRoot.get() == 0 || pGame.get() == 0) {
        return result;
    }

    afl::base::Ptr<TurnLoader> pLoader = pRoot->getTurnLoader();
    if (pLoader.get() == 0) {
        return result;
    }

    return pLoader->saveCurrentTurn(pGame->currentTurn(), *pGame, PlayerSet_t(pGame->getViewpointPlayer()), opts, *pRoot, *this, then);
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
        const interpreter::ProcessList& list = processList();
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
            out = player->getName(Player::AdjectiveName, translator());
            return true;
        }
    }
    return false;
}

void
game::Session::initWorld()
{
    // ex initInterpreterGameInterface()
    typedef interpreter::SimpleFunction<Session&> SessionFunction_t;
    typedef interpreter::SimpleProcedure<Session&> SessionProcedure_t;
    m_world.setNewGlobalValue("AUTOTASK",      new SessionFunction_t(*this, game::interface::IFAutoTask));
    m_world.setNewGlobalValue("BEAM",          new game::interface::BeamFunction(*this));
    m_world.setNewGlobalValue("CADD",          new SessionFunction_t(*this, game::interface::IFCAdd));
    m_world.setNewGlobalValue("CC$NOTIFYCONFIRMED", new game::interface::NotifyConfirmedFunction(*this));
    m_world.setNewGlobalValue("CCOMPARE",      new SessionFunction_t(*this, game::interface::IFCCompare));
    m_world.setNewGlobalValue("CDIV",          new SessionFunction_t(*this, game::interface::IFCDiv));
    m_world.setNewGlobalValue("CEXTRACT",      new SessionFunction_t(*this, game::interface::IFCExtract));
    m_world.setNewGlobalValue("CFG",           new SessionFunction_t(*this, game::interface::IFCfg));
    m_world.setNewGlobalValue("CMUL",          new SessionFunction_t(*this, game::interface::IFCMul));
    m_world.setNewGlobalValue("CREMOVE",       new SessionFunction_t(*this, game::interface::IFCRemove));
    m_world.setNewGlobalValue("CSUB",          new SessionFunction_t(*this, game::interface::IFCSub));
    m_world.setNewGlobalValue("DISTANCE",      new SessionFunction_t(*this, game::interface::IFDistance));
    m_world.setNewGlobalValue("ENGINE",        new game::interface::EngineFunction(*this));
    m_world.setNewGlobalValue("EXPLOSION",     new game::interface::ExplosionFunction(*this));
    m_world.setNewGlobalValue("FORMAT",        new SessionFunction_t(*this, game::interface::IFFormat));
    m_world.setNewGlobalValue("FCODE",         new game::interface::FriendlyCodeFunction(*this));
    m_world.setNewGlobalValue("GETCOMMAND",    new SessionFunction_t(*this, game::interface::IFGetCommand));
    m_world.setNewGlobalValue("HULL",          new game::interface::HullFunction(*this));
    m_world.setNewGlobalValue("INMSG",         new game::interface::InboxFunction(*this));
    m_world.setNewGlobalValue("ISSPECIALFCODE", new SessionFunction_t(*this, game::interface::IFIsSpecialFCode));
    m_world.setNewGlobalValue("ITERATOR",      new SessionFunction_t(*this, game::interface::IFIterator));
    m_world.setNewGlobalValue("LAUNCHER",      new game::interface::TorpedoFunction(true, *this));
    m_world.setNewGlobalValue("MARKER" ,       new game::interface::DrawingFunction(*this));
    m_world.setNewGlobalValue("MINEFIELD",     new game::interface::MinefieldFunction(*this));
    m_world.setNewGlobalValue("MISSION",       new game::interface::MissionFunction(*this));
    m_world.setNewGlobalValue("OBJECTISAT",    new SessionFunction_t(*this, game::interface::IFObjectIsAt));
    m_world.setNewGlobalValue("PLANET",        new game::interface::PlanetFunction(*this));
    m_world.setNewGlobalValue("PLANETAT",      new SessionFunction_t(*this, game::interface::IFPlanetAt));
    m_world.setNewGlobalValue("PLAYER",        new game::interface::PlayerFunction(*this));
    m_world.setNewGlobalValue("PREF",          new SessionFunction_t(*this, game::interface::IFPref));
    m_world.setNewGlobalValue("QUOTE",         new SessionFunction_t(*this, game::interface::IFQuote));
    m_world.setNewGlobalValue("RANDOM",        new SessionFunction_t(*this, game::interface::IFRandom));
    m_world.setNewGlobalValue("RANDOMFCODE",   new SessionFunction_t(*this, game::interface::IFRandomFCode));
    m_world.setNewGlobalValue("SHIP",          new game::interface::ShipFunction(*this));
    m_world.setNewGlobalValue("STORM",         new game::interface::IonStormFunction(*this));
    m_world.setNewGlobalValue("SYSTEM.PLUGIN", new game::interface::PluginFunction(*this));
    m_world.setNewGlobalValue("TORPEDO",       new game::interface::TorpedoFunction(false, *this));
    m_world.setNewGlobalValue("TRANSLATE",     new SessionFunction_t(*this, game::interface::IFTranslate));
    m_world.setNewGlobalValue("TRUEHULL",      new SessionFunction_t(*this, game::interface::IFTruehull));
    m_world.setNewGlobalValue("UFO",           new game::interface::UfoFunction(*this));
    m_world.setNewGlobalValue("VCR",           new game::interface::VcrFunction(*this));

    m_world.setNewGlobalValue("RADD",          new SessionFunction_t(*this, game::interface::IFRAdd));
    m_world.setNewGlobalValue("RALIGN",        new SessionFunction_t(*this, game::interface::IFRAlign));
    m_world.setNewGlobalValue("RLEN",          new SessionFunction_t(*this, game::interface::IFRLen));
    m_world.setNewGlobalValue("RLINK",         new SessionFunction_t(*this, game::interface::IFRLink));
    m_world.setNewGlobalValue("RMID",          new SessionFunction_t(*this, game::interface::IFRMid));
    m_world.setNewGlobalValue("RSTRING",       new SessionFunction_t(*this, game::interface::IFRString));
    m_world.setNewGlobalValue("RSTYLE",        new SessionFunction_t(*this, game::interface::IFRStyle));
    m_world.setNewGlobalValue("RXML",          new SessionFunction_t(*this, game::interface::IFRXml));

    m_world.setNewGlobalValue("REFERENCE",         new SessionFunction_t(*this, game::interface::IFReference));
    m_world.setNewGlobalValue("LOCATIONREFERENCE", new SessionFunction_t(*this, game::interface::IFLocationReference));
    m_world.setNewGlobalValue("REFERENCELIST",     new SessionFunction_t(*this, game::interface::IFReferenceList));

    m_world.setNewGlobalValue("CC$SELREADHEADER",  new SessionFunction_t(*this, game::interface::IFCCSelReadHeader));
    m_world.setNewGlobalValue("CC$SELREADCONTENT", new SessionFunction_t(*this, game::interface::IFCCSelReadContent));
    m_world.setNewGlobalValue("CC$SELGETQUESTION", new SessionFunction_t(*this, game::interface::IFCCSelGetQuestion));
    m_world.setNewGlobalValue("SELECTIONSAVE",     new SessionProcedure_t(*this, game::interface::IFSelectionSave));

    m_world.setNewGlobalValue("ADDCOMMAND",       new SessionProcedure_t(*this, game::interface::IFAddCommand));
    m_world.setNewGlobalValue("ADDCONFIG",        new SessionProcedure_t(*this, game::interface::IFAddConfig));
    m_world.setNewGlobalValue("ADDFCODE",         new SessionProcedure_t(*this, game::interface::IFAddFCode));
    m_world.setNewGlobalValue("ADDPREF",          new SessionProcedure_t(*this, game::interface::IFAddPref));
    m_world.setNewGlobalValue("AUTHPLAYER",       new SessionProcedure_t(*this, game::interface::IFAuthPlayer));
    m_world.setNewGlobalValue("CC$HISTORY.SHOWTURN", new SessionProcedure_t(*this, game::interface::IFCCHistoryShowTurn));
    m_world.setNewGlobalValue("CC$NOTIFY",        new SessionProcedure_t(*this, game::interface::IFCCNotify));
    m_world.setNewGlobalValue("CC$NUMNOTIFICATIONS", new SessionFunction_t(*this, game::interface::IFCCNumNotifications));
    m_world.setNewGlobalValue("CC$SELECTIONEXEC", new SessionProcedure_t(*this, game::interface::IFCCSelectionExec));
    m_world.setNewGlobalValue("CREATECONFIGOPTION", new SessionProcedure_t(*this, game::interface::IFCreateConfigOption));
    m_world.setNewGlobalValue("CREATEPREFOPTION", new SessionProcedure_t(*this, game::interface::IFCreatePrefOption));
    m_world.setNewGlobalValue("DELETECOMMAND",    new SessionProcedure_t(*this, game::interface::IFDeleteCommand));
    m_world.setNewGlobalValue("EXPORT",           new SessionProcedure_t(*this, game::interface::IFExport));
    m_world.setNewGlobalValue("HISTORY.LOADTURN", new SessionProcedure_t(*this, game::interface::IFHistoryLoadTurn));
    m_world.setNewGlobalValue("NEWCANNEDMARKER",  new SessionProcedure_t(*this, game::interface::IFNewCannedMarker));
    m_world.setNewGlobalValue("NEWCIRCLE",        new SessionProcedure_t(*this, game::interface::IFNewCircle));
    m_world.setNewGlobalValue("NEWLINE",          new SessionProcedure_t(*this, game::interface::IFNewLine));
    m_world.setNewGlobalValue("NEWLINERAW",       new SessionProcedure_t(*this, game::interface::IFNewLineRaw));
    m_world.setNewGlobalValue("NEWMARKER",        new SessionProcedure_t(*this, game::interface::IFNewMarker));
    m_world.setNewGlobalValue("NEWRECTANGLE",     new SessionProcedure_t(*this, game::interface::IFNewRectangle));
    m_world.setNewGlobalValue("NEWRECTANGLERAW",  new SessionProcedure_t(*this, game::interface::IFNewRectangleRaw));
    m_world.setNewGlobalValue("SAVEGAME",         new SessionProcedure_t(*this, game::interface::IFSaveGame));
    m_world.setNewGlobalValue("SENDMESSAGE",      new SessionProcedure_t(*this, game::interface::IFSendMessage));

    m_world.setNewGlobalValue("GLOBALACTIONCONTEXT", new interpreter::SimpleFunction<void>(game::interface::IFGlobalActionContext));

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

    sig_connectionChange.raise();
}

void
game::Session::updateMap()
{
    if (m_root.get() != 0 && m_game.get() != 0) {
        m_game->mapConfiguration().initFromConfiguration(m_root->hostConfiguration(), m_root->userConfiguration());
    }
}
