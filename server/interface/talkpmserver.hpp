/**
  *  \file server/interface/talkpmserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPMSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKPMSERVER_HPP

#include "server/interface/talkpm.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkPMServer : public ComposableCommandHandler {
     public:
        TalkPMServer(TalkPM& impl);
        ~TalkPMServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const TalkPM::Info& info);

     private:
        TalkPM& m_implementation;
    };

} }

#endif
