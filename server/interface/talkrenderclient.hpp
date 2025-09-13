/**
  *  \file server/interface/talkrenderclient.hpp
  *  \brief Class server::interface::TalkRenderClient
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDERCLIENT_HPP

#include "server/interface/talkrender.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for rendering server.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class TalkRenderClient : public TalkRender {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the TalkRenderClient. */
        explicit TalkRenderClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~TalkRenderClient();

        // TalkRender:
        virtual void setOptions(const Options& opts);
        virtual String_t render(const String_t& text, const Options& opts);
        virtual void check(const String_t& text, std::vector<Warning>& out);

        /** Pack options into a command.
            @param command [out]  Command to which options are appended
            @param opts    [in]   Options */
        static void packOptions(afl::data::Segment& command, const Options& opts);

        /** Convert warning from network format.
            @param a Handle to warning object received from server
            @return Warning object */
        static Warning unpackWarning(afl::data::Access a);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
