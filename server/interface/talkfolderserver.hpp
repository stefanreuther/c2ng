/**
  *  \file server/interface/talkfolderserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFOLDERSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKFOLDERSERVER_HPP

#include "server/interface/talkfolder.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkFolderServer : public ComposableCommandHandler {
     public:
        TalkFolderServer(TalkFolder& impl);
        ~TalkFolderServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const TalkFolder::Info& info);

     private:
        TalkFolder& m_implementation;
    };

} }

#endif
