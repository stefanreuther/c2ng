/**
  *  \file server/interface/sessionrouter.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_SESSIONROUTER_HPP
#define C2NG_SERVER_INTERFACE_SESSIONROUTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace interface {

    /** Session router base interface.
        This interface allows access to a game session multiplexer.
        A session is identified by a SessionId_t.
        You can start sessions, send commands to sessions, or operate on sessions as a group. */
    class SessionRouter : public afl::base::Deletable {
     public:
        /** Action for single/bulk operation. */
        enum Action {
            /** Close session (CLOSE).
                Session Id becomes invalid. */
            Close,

            /** Restart session (RESTART).
                Mostly for internal use; should not have a user-perceived effect.
                c2router-server will restart the c2play-server instance. */
            Restart,

            /** Save session (SAVE). */
            Save,

            /** Save session, but do not notify file server (SAVENN).
                This is intended for saves initiated by another server,
                and only has a meaning if the c2play-server instance operates
                on the same filespace as the file server. */
            SaveNN
        };

        /** Type of a session Id.
            Should be a non-empty sequence of non-blank (alphanumeric) characters.
            Session Ids are not re-used.
            Traditionally this was a number, but changed to string to allow future cryptographic session Ids. */
        typedef String_t SessionId_t;

        /** List sessions (LIST).
            Produces a table, mainly for human consumption.
            Currently not interpreted by the protocol handlers. */
        virtual String_t getStatus() = 0;

        /** Get information about a session (INFO).
            Produces the list of parameters the session was started with.
            Currently not interpreted by the protocol handlers.
            \param sessionId Session Id */
        virtual String_t getInfo(SessionId_t sessionId) = 0;

        /** Talk to session (S).
            \param sessionId Session Id
            \param command Command. Could be a one-line command ("GET obj/main",
                           or a multi-line command ("POST obj/planet2\n[[...]]").
                           Trailing newline is optional.
            \return Result of command, starting with a one-line response line ("200 OK"),
            followed by a newline and a response body, if any.

            FIXME: reconsider the parameter formats */
        virtual String_t talk(SessionId_t sessionId, String_t command) = 0;

        /** Act on single session (CLOSE/RESTART/SAVE/SAVENN).
            \param sessionId Session
            \param action Action */
        virtual void sessionAction(SessionId_t sessionId, Action action) = 0;

        /** Act on multiple sessions (CLOSE/RESTART/SAVE/SAVENN).
            Sessions are matched using conflict tokens, i.e. "WGAME=3" operates on all sessions
            that have a "-WGAME=3" conflict token on their command line.
            \param key    [in] Conflict token ("WDIR")
            \param action [in] Action
            \param result [out] Session Ids of affected sessions */
        virtual void groupAction(String_t key, Action action, afl::data::StringList_t& result) = 0;

        /** Create new session.
            \param args Parameters for c2play-server.
            \return Session Id */
        virtual String_t create(afl::base::Memory<const String_t> args) = 0;

        /** Get configuration.
            Produces a table, mainly for human consumption.
            Currently not interpreted by the protocol handlers. */
        virtual String_t getConfiguration() = 0;

        /** Convert Action to string.
            \param action Action
            \return string representation */
        static String_t formatAction(Action action);

        /** Parse string into Action.
            \param str [in] String
            \param result [out] Action
            \return true on success */
        static bool parseAction(const String_t& str, Action& result);
    };

} }

#endif
