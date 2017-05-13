/**
  *  \file server/interface/talkforumserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFORUMSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKFORUMSERVER_HPP

#include "server/interface/talkforum.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkForumServer : public ComposableCommandHandler {
     public:
        TalkForumServer(TalkForum& impl);
        ~TalkForumServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const TalkForum::Info& info);
        static void parseListParameters(TalkForum::ListParameters& p, interpreter::Arguments& args);

     private:
        TalkForum& m_implementation;
    };

} }

#endif
