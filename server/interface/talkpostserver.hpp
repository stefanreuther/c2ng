/**
  *  \file server/interface/talkpostserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPOSTSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKPOSTSERVER_HPP

#include "server/interface/talkpost.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkPostServer : public ComposableCommandHandler {
     public:
        TalkPostServer(TalkPost& impl);
        ~TalkPostServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const TalkPost::Info& info);

     private:
        TalkPost& m_implementation;
    };


} }

#endif
