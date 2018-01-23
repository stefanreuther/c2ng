/**
  *  \file server/interface/hostturnserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURNSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURNSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class HostTurn;

    class HostTurnServer : public ComposableCommandHandler {
     public:
        explicit HostTurnServer(HostTurn& impl);
        ~HostTurnServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostTurn& m_implementation;
    };

} }

#endif
