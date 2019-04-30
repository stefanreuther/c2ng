/**
  *  \file server/router/session.hpp
  *  \brief Class server::router::Session
  */
#ifndef C2NG_SERVER_ROUTER_SESSION_HPP
#define C2NG_SERVER_ROUTER_SESSION_HPP

#include <memory>
#include "afl/base/memory.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/sys/time.hpp"
#include "afl/sys/loglistener.hpp"
#include "server/interface/filebase.hpp"
#include "util/process/factory.hpp"

namespace server { namespace router {

    /** A PCC2 Web session.
        Represents the connection to a single server process and all identifying information. */
    class Session {
     public:
        Session(util::process::Factory& factory,
                afl::base::Memory<const String_t> args,
                String_t id,
                afl::sys::LogListener& log,
                server::interface::FileBase* pFileBase);

        ~Session();

        String_t getId() const;

        uint32_t getProcessId() const;

        bool isModified() const;

        bool isUsed() const;

        bool isActive() const;

        afl::sys::Time getLastAccessTime() const;

        afl::base::Memory<const String_t> getCommandLine() const;

        bool checkConflict(const Session& other) const;

        bool checkConflict(const String_t& query, bool queryIsWild) const;

        bool start(const String_t& serverPath);

        void stop();

        void save(bool notify);

        String_t talk(String_t command);

     private:
        String_t m_id;                   // ex session_id
        afl::sys::LogListener& m_log;
        server::interface::FileBase* m_pFileBase;
        afl::data::StringList_t m_args;  // ex name
        afl::sys::Time m_lastAccessTime; // ex last_access
        bool m_isModified;               // ex used
        bool m_isUsed;                   // ex modified

        std::auto_ptr<util::process::Subprocess> m_process;

        void logCommandLine();
        void logProcess(afl::sys::LogListener::Level level, const String_t& msg);
        void logProcess(afl::sys::LogListener::Level level, const String_t& msg, uint32_t pid);

        void setLastAccessTime();
        bool readLine(String_t& line);
        void readResponse(String_t& header, String_t& body);
        void notifyFileServer();
        void handleError(const char* reason);
    };

} }

#endif
