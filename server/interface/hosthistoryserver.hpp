/**
  *  \file server/interface/hosthistoryserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTHISTORYSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTHISTORYSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostHistory;

    class HostHistoryServer : public ComposableCommandHandler {
     public:
        explicit HostHistoryServer(HostHistory& impl);
        ~HostHistoryServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostHistory& m_implementation;
    };

} }

#endif
