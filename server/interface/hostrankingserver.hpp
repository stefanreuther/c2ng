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

    /** Server for host ranking list access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostRanking implementation. */
    class HostRankingServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostRankingServer(HostRanking& impl);

        /** Destructor. */
        ~HostRankingServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostRanking& m_implementation;
    };

} }

#endif
