/**
  *  \file server/interface/hostrankingserver.hpp
  *  \brief Class server::interface::HostRankingServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTRANKINGSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTRANKINGSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostRanking;

    class HostRankingServer : public ComposableCommandHandler {
     public:
        explicit HostRankingServer(HostRanking& impl);
        ~HostRankingServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostRanking& m_implementation;
    };

} }

#endif
