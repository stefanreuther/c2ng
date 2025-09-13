/**
  *  \file server/interface/talkrenderserver.hpp
  *  \brief Class server::interface::TalkRenderServer
  */
#ifndef C2NG_SERVER_INTERFACE_TALKRENDERSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKRENDERSERVER_HPP

#include "server/interface/talkrender.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    /** Server for rendering.
        Implements a ComposableCommandHandler and dispatches received commands to a TalkRender implementation. */
    class TalkRenderServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        TalkRenderServer(TalkRender& impl);

        /** Destructor. */
        ~TalkRenderServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Parse options.
            @param [in]  args  Command-line given by client
            @param [out] opts  Options; will be updated */
        static void parseOptions(interpreter::Arguments& args, TalkRender::Options& opts);

        /** Pack a warning into a value.
            @param w Warning
            @return Newly-allocated value; caller takes responsibility */
        static Value_t* packWarning(const TalkRender::Warning& w);

     private:
        TalkRender& m_implementation;
    };

} }

#endif
