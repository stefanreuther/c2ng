/**
  *  \file game/browser/session.hpp
  *  \brief Class game::browser::Session
  */
#ifndef C2NG_GAME_BROWSER_SESSION_HPP
#define C2NG_GAME_BROWSER_SESSION_HPP

#include <memory>
#include "afl/container/ptrqueue.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/optionalusercallback.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    /** Browser session.
        Aggregates all the objects needed for browsing through a game.
        To use, create Session and attach all required Handler objects to the embedded Browser.

        The browser session includes a task queue.
        When using a method that produces as task (e.g. Browser::loadGameRoot),
        post that task using Session::addTask().
        The task's @c then handler must then eventually call Session::finishTask(). */
    class Session {
     public:
        /** Constructor.
            @param fileSystem   File system
            @param tx           Translator
            @param log          Logger
            @param profile      Profile directory */
        Session(afl::io::FileSystem& fileSystem, afl::string::Translator& tx, afl::sys::LogListener& log, util::ProfileDirectory& profile);
        ~Session();

        /** Access translator.
            @return translator as passed to constructor */
        afl::string::Translator& translator();

        /** Access logger.
            @return logger as passed to constructor */
        afl::sys::LogListener& log();

        /** Access browser.
            @return embedded Browser object */
        Browser& browser();

        /** Access account manager.
            @return embedded AccountManager object */
        AccountManager& accountManager();

        /** Access browser callback.
            @return embedded OptionalUserCallback object */
        OptionalUserCallback& callback();

        /** Add a task.
            Used to serialize tasks that potentially suspend.
            The task must call finishTask() when done.
            @param task Task */
        void addTask(std::auto_ptr<Task_t> task);

        /** Register completion of a task. */
        void finishTask();

     private:
        // Infrastructure
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        // Data
        AccountManager m_accountManager;
        OptionalUserCallback m_callback;

        // Browser (after other objects because it refers to them)
        Browser m_browser;

        // Tasks (last, so it is deleted first and causes tasks to take their hands off other objects)
        afl::container::PtrQueue<Task_t> m_tasks;
    };

} }

#endif
