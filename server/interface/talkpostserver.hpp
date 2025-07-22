/**
  *  \file server/interface/talkpostserver.hpp
  *  \brief Class server::interface::TalkPostServer
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPOSTSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKPOSTSERVER_HPP

#include "server/interface/talkpost.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    /** Server for TalkPost interface.
        Implements a ComposableCommandHandler and dispatches received commands to a TalkPost implementation. */
    class TalkPostServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long */
        explicit TalkPostServer(TalkPost& impl);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Serialize a TalkPost::Info.
            @param info Info
            @return Serialized result */
        static Value_t* packInfo(const TalkPost::Info& info);

     private:
        TalkPost& m_implementation;
    };

} }

#endif
