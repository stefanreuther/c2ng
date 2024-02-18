/**
  *  \file game/session.hpp
  *  \brief Class game::Session
  */
#ifndef C2NG_GAME_SESSION_HPP
#define C2NG_GAME_SESSION_HPP

#include <memory>
#include "afl/base/optional.hpp"
#include "afl/base/ptr.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/log.hpp"
#include "game/authcache.hpp"
#include "game/extracontainer.hpp"
#include "game/interface/notificationstore.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "game/interface/userinterfacepropertystack.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/object.hpp"
#include "game/reference.hpp"
#include "game/spec/shiplist.hpp"
#include "game/task.hpp"
#include "game/turnloader.hpp"
#include "interpreter/error.hpp"
#include "interpreter/processlist.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/world.hpp"
#include "util/plugin/manager.hpp"
#include "util/systeminformation.hpp"

namespace game {

    class Root;
    class Game;
    class Turn;

    /** Session.
        This aggregates the dynamic parts of a game session and controls lifetime of all child components.

        - It has a log node. Child objects use this log node, observers observe this log node.
        - It knows a Translator. Child objects use this translator.
        - It has an optional Root that can be reset or modified.
        - It has an optional ShipList that can be reset or modified.
        - It has an optional Game that can be reset or modified.
        - It has an interpreter::World. All interpreter objects live in that.
        - It provides an InterpreterInterface.

        Because all interpreter objects live in the embedded interpreter::World,
        it follows that no interpreter object can outlive a Session,
        but interpreter objects can outlive a Root, ShipList, or Game.
        Therefore,
        - interpreter objects can refer to a Session, World, InterpreterInterface, or Translator using a dumb reference.
        - interpreter objects must refer to Root, ShipList, or Game using a smart pointer.
        - interpreter objects must not be transferred between sessions.
        - Root, Session, Game, or Turn objects must not refer to each other. */
    class Session : private InterpreterInterface {
     public:
        /** Task status.
            \see getTaskStatus() */
        enum TaskStatus {
            NoTask,             ///< No auto-task active.
            ActiveTask,         ///< Task active and operable (green).
            WaitingTask,        ///< Task waiting for user interaction (red).
            OtherTask           ///< No auto-task, but another task active on this unit (yellow).
        };


        /** Constructor.
            \param tx Translator
            \param fs File system */
        explicit Session(afl::string::Translator& tx, afl::io::FileSystem& fs);

        /** Destructor. */
        ~Session();

        /** Access logger.
            \return logger */
        afl::sys::Log& log();

        /** Log an error.
            \param e Error
            \see interpreter::World::logError */
        void logError(const interpreter::Error& e);

        /** Access translator.
            \return translator */
        afl::string::Translator& translator() const;

        /** Get root.
            \return root. Can be null. */
        const afl::base::Ptr<Root>& getRoot();

        /** Set root.
            \param root New root */
        void setRoot(afl::base::Ptr<Root> root);

        /** Get ship list.
            \return ship list. Can be null. */
        const afl::base::Ptr<game::spec::ShipList>& getShipList();

        /** Set ship list.
            \param shipList new ship list */
        void setShipList(afl::base::Ptr<game::spec::ShipList> shipList);

        /** Get game.
            \return game. Can be null. */
        const afl::base::Ptr<Game>& getGame();

        /** Set game.
            \param game New game */
        void setGame(afl::base::Ptr<Game> game);

        /** Access user-interface property stack.
            \return user-interface property stack */
        game::interface::UserInterfacePropertyStack& uiPropertyStack();
        const game::interface::UserInterfacePropertyStack& uiPropertyStack() const;

        /** Access notification store.
            \return notification store */
        game::interface::NotificationStore& notifications();
        const game::interface::NotificationStore& notifications() const;


        /** Get auto-task editor.
            If the object does not have an auto-task, this creates one.
            If the auto-task is not being edited already, creates an editor; otherwise, re-uses the existing one.

            If you're done with the TaskEditor, use releaseAutoTaskEditor().
            Failing to do so will not run the task.

            \param id     Object Id
            \param kind   Object (and therfore, task) kind
            \param create true to create a task if it does not exist (you want to edit it); false to return null in this case
            \return TaskEditor; null if preconditions not satisfied:
            - object does not exist
            - task does not exist and create=false
            - task is frozen by someone other than a TaskEditor */
        afl::base::Ptr<interpreter::TaskEditor> getAutoTaskEditor(Id_t id, interpreter::Process::ProcessKind kind, bool create);

        /** Release auto-task editor.
            If this is the last reference to the auto-task editor, this will release the editor and run the task.
            \param [in,out] ptr Task editor. Will be set to null. */
        void releaseAutoTaskEditor(afl::base::Ptr<interpreter::TaskEditor>& ptr);

        /** Get task status for an object.
            \param obj       Invoking object
            \param kind      Process kind
            \param waitOnly  Only produce information about waiting notification (i.e. for auto-task editor)
            \return status */
        TaskStatus getTaskStatus(const game::map::Object* obj, interpreter::Process::ProcessKind kind, bool waitOnly) const;

        /** Access interpreter world.
            \return world */
        interpreter::World& world();

        /** Access process list.
            \return process list */
        interpreter::ProcessList& processList();
        const interpreter::ProcessList& processList() const;

        /** Access InterpreterInterface.
            \return InterpreterInterface */
        InterpreterInterface& interface();

        /** Access SystemInformation.
            \return SystemInformation. */
        const util::SystemInformation& getSystemInformation() const;

        /** Set SystemInformation.
            \param info New system information */
        void setSystemInformation(const util::SystemInformation& info);

        /** Access random-number generator.
            \return random-number generator */
        util::RandomNumberGenerator& rng();

        /** Access plugin manager.
            \return plugin manager */
        util::plugin::Manager& plugins();

        /** Set plugin directory name.
            \param name Name. Empty (default) means plugins cannot be installed. */
        void setPluginDirectoryName(String_t name);

        /** Get plugin directory name.
            \return name */
        String_t getPluginDirectoryName() const;

        /** Access authentication cache.
            \return authentication cache */
        AuthCache& authCache();

        /** Access session extras.
            \return session extras */
        ExtraContainer<Session>& extra();

        /** Notify listeners.
            Invokes listeners for all sub-objects (Game, Root, World). */
        void notifyListeners();

        /** Get name, given a reference.
            \param ref    Reference
            \param which  Which name to return
            \return Name if reference can successfully be resolved */
        afl::base::Optional<String_t> getReferenceName(Reference ref, ObjectName which) const;

        /** Postprocess a turn.
            Used as part of the loading sequence.
            This completes the turn data.

            Requires that game, root, and shiplist are set.
            Call is ignored if they are not.

            \param t                Turn to work on
            \param playingSet       Set of players we're playing.
                                    Those players will be set to the given \c playability;
                                    others will at best be ReadOnly.
            \param availablePlayers Available players (set of loaded result files).
                                    Used for hasFullData().
            \param playability      Playability to use for players in \c playingSet.
            \see game::map::Universe::postprocess() */
        void postprocessTurn(Turn& t, PlayerSet_t playingSet, PlayerSet_t availablePlayers, game::map::Object::Playability playability);

        /** Save current status.
            \param opts Options, see TurnLoader::saveCurrentTurn()
            \param then Closure to execute after saving; will receive success/failure status
            \return Save action; null if no turn loaded */
        std::auto_ptr<Task_t> save(TurnLoader::SaveOptions_t opts, std::auto_ptr<StatusTask_t> then);

        /** Save configuration.
            If no game is loaded, returns the @c then action as-is, making this a no-op.
            \param then Action to execute after saving
            \return Save action; never null */
        std::auto_ptr<Task_t> saveConfiguration(std::auto_ptr<Task_t> then);

        /** Signal: request ProcessList::run() to be run.
            Code that sets a process to runnable should raise this signal.
            This signal will be handled by a component that can pick a good point to run processes
            and perform necessary pre- and post-processing.
            As a minimum implementation, this signal can be hooked to ProcessList::run(). */
        afl::base::Signal<void()> sig_runRequest;

        /** Signal: change of a connected object (Root, Game, ShipList). */
        afl::base::Signal<void()> sig_connectionChange;

     private:
        afl::sys::Log m_log;
        afl::base::Ptr<Root> m_root;
        afl::base::Ptr<game::spec::ShipList> m_shipList;
        afl::base::Ptr<Game> m_game;
        game::interface::UserInterfacePropertyStack m_uiPropertyStack;
        interpreter::World m_world;

        /** System information. */
        util::SystemInformation m_systemInformation;

        /** Process list.
            Must be after World, because processes reference World.
            Process List used to be part of World, but has been moved here to reduce cyclic dependencies:
            a lot of code used to build processes references World. */
        // ex int/process.cc:process_list
        interpreter::ProcessList m_processList;

        util::RandomNumberGenerator m_rng;
        util::plugin::Manager m_plugins;
        String_t m_pluginDirectoryName;
        AuthCache m_authCache;
        ExtraContainer<Session> m_extra;
        game::interface::NotificationStore m_notifications;

        afl::base::SignalConnection conn_hostConfigToMap;
        afl::base::SignalConnection conn_userConfigToMap;

        // InterpreterInterface:
        virtual String_t getComment(Scope scope, int id) const;
        virtual bool hasTask(Scope scope, int id) const;
        virtual afl::base::Optional<String_t> getHullShortName(int nr) const;
        virtual afl::base::Optional<String_t> getPlayerAdjective(int nr) const;

        void initWorld();
        void onProcessStateChange(const interpreter::Process& proc, bool willDelete);

        // Signals:
        void connectSignals();
        void updateMap();
    };

}

// Note 20201010: these functions needs to be inline.
// Otherwise, c2simtool will needlessly blow up by pulling in all of Session and, with that, all of the interpreter.
//   game::vcr::classic::getDatabase() -> getGame
//   game::v3::Loader -> game::v3::Reverter -> getRoot, getShipList

inline afl::sys::Log&
game::Session::log()
{
    return m_log;
}

inline void
game::Session::logError(const interpreter::Error& e)
{
    m_world.logError(m_log.Error, e);
}

inline afl::string::Translator&
game::Session::translator() const
{
    return m_world.translator();
}

inline const afl::base::Ptr<game::Root>&
game::Session::getRoot()
{
    return m_root;
}

inline const afl::base::Ptr<game::spec::ShipList>&
game::Session::getShipList()
{
    return m_shipList;
}

inline const afl::base::Ptr<game::Game>&
game::Session::getGame()
{
    return m_game;
}

inline interpreter::World&
game::Session::world()
{
    return m_world;
}

#endif
