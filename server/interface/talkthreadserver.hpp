/**
  *  \file server/interface/talkthreadserver.hpp
  *  \brief Class server::interface::TalkThreadServer
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREADSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREADSERVER_HPP

#include "server/interface/talkthread.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    /** Server for TalkThread interface.
        Implements a ComposableCommandHandler and dispatches received commands to a TalkThread implementation. */
    class TalkThreadServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        TalkThreadServer(TalkThread& implementation);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Serialize a TalkThread::Info.
            @param info Info
            @return Serialized result */
        static Value_t* packInfo(const TalkThread::Info& info);

     private:
        TalkThread& m_implementation;
    };

} }

#endif
