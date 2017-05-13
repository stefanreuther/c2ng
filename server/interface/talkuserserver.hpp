/**
  *  \file server/interface/talkuserserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKUSERSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKUSERSERVER_HPP

#include "server/interface/talkuser.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkUserServer : public ComposableCommandHandler {
     public:
        TalkUserServer(TalkUser& implementation);
        ~TalkUserServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static void parseSelection(interpreter::Arguments& args, std::vector<TalkUser::Selection>& selections);

     private:
        TalkUser& m_implementation;
    };

} }

#endif
