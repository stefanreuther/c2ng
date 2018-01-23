/**
  *  \file server/interface/hostplayerserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYERSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYERSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostplayer.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostPlayerServer : public ComposableCommandHandler {
     public:
        explicit HostPlayerServer(HostPlayer& impl);
        ~HostPlayerServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const HostPlayer::Info& i);

     private:
        HostPlayer& m_implementation;
    };

} }

#endif
