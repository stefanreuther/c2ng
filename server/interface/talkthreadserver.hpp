/**
  *  \file server/interface/talkthreadserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREADSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREADSERVER_HPP

#include "server/interface/talkthread.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkThreadServer : public ComposableCommandHandler {
     public:
        TalkThreadServer(TalkThread& implementation);
        ~TalkThreadServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const TalkThread::Info& info);

     private:
        TalkThread& m_implementation;
    };

} }

#endif
