/**
  *  \file server/interface/filegameserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAMESERVER_HPP
#define C2NG_SERVER_INTERFACE_FILEGAMESERVER_HPP

#include "server/interface/filegame.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class FileGameServer : public ComposableCommandHandler {
     public:
        FileGameServer(FileGame& impl);
        ~FileGameServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packGameInfo(const FileGame::GameInfo& info);
        static Value_t* packKeyInfo(const FileGame::KeyInfo& info);

     private:
        FileGame& m_implementation;
    };

} }

#endif
