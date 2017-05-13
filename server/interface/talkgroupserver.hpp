/**
  *  \file server/interface/talkgroupserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKGROUPSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKGROUPSERVER_HPP

#include "server/interface/talkgroup.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkGroupServer : public ComposableCommandHandler {
     public:
        TalkGroupServer(TalkGroup& impl);
        ~TalkGroupServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static TalkGroup::Description parseDescription(interpreter::Arguments& args);
        static Value_t* formatDescription(const TalkGroup::Description& desc);

     private:
        TalkGroup& m_implementation;
    };


} }

#endif
