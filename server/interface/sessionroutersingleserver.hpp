/**
  *  \file server/interface/sessionroutersingleserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_SESSIONROUTERSINGLESERVER_HPP
#define C2NG_SERVER_INTERFACE_SESSIONROUTERSINGLESERVER_HPP

#include "afl/net/line/linehandler.hpp"
#include "server/interface/sessionrouter.hpp"

namespace server { namespace interface {

    /** SessionRouter server implementation: classic single-command server.
        This server accepts a single command on each network connection.

        Most commands are one-liners.
        The talk() command is either two lines ("S n" to select session n, then the command to send to the session),
        or multiple lines for POST ("S n" to select session n, "POST addr" to start posting, POST body, ".").
        Multi-line responses are delimited by connection-close. */
    class SessionRouterSingleServer : public afl::net::line::LineHandler {
     public:
        SessionRouterSingleServer(SessionRouter& impl);

        virtual bool handleOpening(afl::net::line::LineSink& response);
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& response);
        virtual void handleConnectionClose();

        static bool isPOST(const String_t& cmd);
        static bool isSAVE(const String_t& cmd);

     private:
        enum State {
            ReadCommand,
            ReadTalkCommand,
            ReadTalkBody,
            Finished
        };
        SessionRouter& m_impl;
        State m_state;
        String_t m_talkCommand;
        String_t m_talkSession;

        bool handleCommand(const String_t& line, afl::net::line::LineSink& response);
        bool handleAction(const String_t& arg, SessionRouter::Action action, afl::net::line::LineSink& response);
        bool finish();
    };

} }

#endif
