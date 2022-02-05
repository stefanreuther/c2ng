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

    class Session {
     public:
        Session(afl::io::FileSystem& fileSystem,
                afl::string::Translator& tx,
                afl::sys::LogListener& log,
                util::ProfileDirectory& profile);
        ~Session();

        afl::string::Translator& translator();
        afl::sys::LogListener& log();

        Browser& browser();
        AccountManager& accountManager();
        OptionalUserCallback& callback();

        void addTask(std::auto_ptr<Task_t> task);
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
