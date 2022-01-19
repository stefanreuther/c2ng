/**
  *  \file game/browser/session.hpp
  *  \brief Class game::browser::Session
  */
#ifndef C2NG_GAME_BROWSER_SESSION_HPP
#define C2NG_GAME_BROWSER_SESSION_HPP

#include <memory>
#include "afl/container/ptrqueue.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/usercallbackproxy.hpp"

namespace game { namespace browser {

    class Session {
     public:
        Session(afl::string::Translator& tx, afl::sys::LogListener& log);
        ~Session();

        afl::string::Translator& translator();
        afl::sys::LogListener& log();

        std::auto_ptr<Browser>& browser();
        std::auto_ptr<AccountManager>& accountManager();
        UserCallbackProxy& userCallbackProxy();

        void addTask(std::auto_ptr<Task_t> task);
        void finishTask();

     private:
        // Infrastructure
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;

        // Data
        std::auto_ptr<Browser> m_browser;
        std::auto_ptr<AccountManager> m_accountManager;
        UserCallbackProxy m_userCallbackProxy;

        afl::container::PtrQueue<Task_t> m_tasks;
    };

} }

#endif
