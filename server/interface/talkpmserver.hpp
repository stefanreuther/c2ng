/**
  *  \file server/interface/talkpmserver.hpp
  *  \brief Class server::interface::TalkPMServer
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPMSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKPMSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/talkpm.hpp"

namespace server { namespace interface {

    /** User mail server.
        Implements a ComposableCommandHandler that accepts commands addressed to a TalkPM instance.
        TalkPMServer has no local state and can be short-lived. */
    class TalkPMServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Interface implementation */
        explicit TalkPMServer(TalkPM& impl);

        /** Destructor. */
        ~TalkPMServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack a TalkPM::Info into transferrable object.
            @param info Info object
            @return newly-allocated object; caller takes ownership */
        static Value_t* packInfo(const TalkPM::Info& info);

     private:
        TalkPM& m_implementation;
    };

} }

#endif
