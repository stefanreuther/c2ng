/**
  *  \file server/interface/talkrenderserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDERSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDERSERVER_HPP

#include "server/interface/talkrender.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkRenderServer : public ComposableCommandHandler {
     public:
        TalkRenderServer(TalkRender& impl);
        ~TalkRenderServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static void parseOptions(interpreter::Arguments& args, TalkRender::Options& opts);

     private:
        TalkRender& m_implementation;
    };

} } 

#endif
