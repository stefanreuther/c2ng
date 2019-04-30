/**
  *  \file server/interface/talkaddressserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKADDRESSSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKADDRESSSERVER_HPP

#include "server/interface/talkpm.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkAddress;

    class TalkAddressServer : public ComposableCommandHandler {
     public:
        TalkAddressServer(TalkAddress& impl);
        ~TalkAddressServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        TalkAddress& m_implementation;
    };

} }

#endif
