/**
  *  \file server/interface/filegameserver.hpp
  *  \brief Class server::interface::FileGameServer
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAMESERVER_HPP
#define C2NG_SERVER_INTERFACE_FILEGAMESERVER_HPP

#include "server/interface/filegame.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    /** Game file server.
        Implements a ComposableCommandHandler that accepts game-related filer commands
        and translates them into calls on a FileGame instance. */
    class FileGameServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            \param impl Implementation */
        explicit FileGameServer(FileGame& impl);
        ~FileGameServer();
        
        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack GameInfo into transferrable object.
            \param info GameInfo object
            \return newly-allocated object; caller takes ownership */
        static Value_t* packGameInfo(const FileGame::GameInfo& info);

        /** Pack KeyInfo into transferrable object.
            \param info KeyInfo object
            \return newly-allocated object; caller takes ownership */
        static Value_t* packKeyInfo(const FileGame::KeyInfo& info);

     private:
        FileGame& m_implementation;
    };

} }

#endif
