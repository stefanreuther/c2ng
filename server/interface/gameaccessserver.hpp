/**
  *  \file server/interface/gameaccessserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_GAMEACCESSSERVER_HPP
#define C2NG_SERVER_INTERFACE_GAMEACCESSSERVER_HPP

#include "afl/net/line/linehandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class GameAccess;

    class GameAccessServer : public afl::net::line::LineHandler {
     public:
        explicit GameAccessServer(GameAccess& impl);

        virtual bool handleOpening(afl::net::line::LineSink& response);
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& response);
        virtual void handleConnectionClose();

     private:
        enum State {
            Normal,
            Posting
        };
        GameAccess& m_implementation;
        State m_state;
        String_t m_postTarget;
        String_t m_postBody;

        void sendResponse(afl::net::line::LineSink& response, Value_t* value);
        void sendResponse(afl::net::line::LineSink& response, const String_t& lines);

        void sendMemoryResponse(afl::net::line::LineSink& response, afl::base::ConstBytes_t mem);
    };

} }

#endif
