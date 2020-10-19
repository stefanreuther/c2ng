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
        /** Constructor.
            \param factory   Process factory to create the server process
            \param args      Parameter list (not including command name)
            \param id        Session Id
            \param log       Logger
            \param pFileBase FileBase implementation for notifications, can be null */
        Session(util::process::Factory& factory,
                afl::base::Memory<const String_t> args,
                String_t id,
                afl::sys::LogListener& log,
                server::interface::FileBase* pFileBase);

        /** Destructor.
            If the process has not been stopped, this will stop it. */
        ~Session();

        /** Get Id.
            \return Id as passed to constructor. */
        String_t getId() const;

        /** Get process Id.
            \return process Id (assigned by operating system / process factory) */
        uint32_t getProcessId() const;

        /** Check whether session was modified and needs to be saved.
            \return true if session was modified */
        bool isModified() const;

        /** Check whether session was used (normalTimeout applies instead of virginTimeout).
            \return true if session was used */
        bool isUsed() const;

        /** Check whether session is active (process has been started).
            \return true if session is active */
        bool isActive() const;

        /** Get time of last access.
            \return timestamp */
        afl::sys::Time getLastAccessTime() const;

        /** Get command line.
            \return command line (copy of the constructor parameter) */
        afl::base::Memory<const String_t> getCommandLine() const;

        /** Check for conflict with another session.
            Checks for conflict with every of the other session's parameters.
            \param other Other session
            \return true if sessions conflict */
        bool checkConflict(const Session& other) const;

        /** Check for conflict with a keyword.
            \param query       Keyword
            \param queryIsWild Use wildcard semantics ("-Wfoo*" matches "-Wfoo" and "-Wfoo/bar")
            \return true if conflict found */
        bool checkConflict(const String_t& query, bool queryIsWild) const;

        /** Start this session.
            \param serverPath Program name
            \return true if session started successfully (process started; greeting received) */
        bool start(const String_t& serverPath);

        /** Stop this session. */
        void stop();

        /** Save this session.
            Submits a SAVE command to the server process.
            \param notify Notify file server */
        void save(bool notify);

        /** Send command to server.
            \param command Command (either "GET /url", or "POST/url" followed by newline and JSON data)
            \return response Header line, optionally followed by newline and JSON data */
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
