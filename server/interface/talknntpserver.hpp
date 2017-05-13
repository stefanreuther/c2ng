/**
  *  \file server/interface/talknntpserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKNNTPSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKNNTPSERVER_HPP

#include "server/interface/talknntp.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkNNTPServer : public ComposableCommandHandler {
     public:
        TalkNNTPServer(TalkNNTP& impl);
        ~TalkNNTPServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        Value_t* packInfo(const TalkNNTP::Info& info);

     private:
        TalkNNTP& m_implementation;
    };

} }

#endif
