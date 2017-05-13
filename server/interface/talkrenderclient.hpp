/**
  *  \file server/interface/talkrenderclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDERCLIENT_HPP

#include "server/interface/talkrender.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/data/segment.hpp"

namespace server { namespace interface {

    class TalkRenderClient : public TalkRender {
     public:
        TalkRenderClient(afl::net::CommandHandler& commandHandler);
        ~TalkRenderClient();

        virtual void setOptions(const Options& opts);
        virtual String_t render(const String_t& text, const Options& opts);

        static void packOptions(afl::data::Segment& command, const Options& opts);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
