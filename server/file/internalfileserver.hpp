/**
  *  \file server/file/internalfileserver.hpp
  *  \brief Class server::file::InternalFileServer
  */
#ifndef C2NG_SERVER_FILE_INTERNALFILESERVER_HPP
#define C2NG_SERVER_FILE_INTERNALFILESERVER_HPP

#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace file {

    /** In-Memory implementation of the File service.
        This is a all-in-one instantiation of the File service, intended for testing.
        Whenever a CommandHandler is needed that works like a File service, you can use InternalFileServer.
        It will store files in memory.
        Otherwise, there are no particular optimisations enabled.

        It uses server::file::CommandHandler and therefore implements the Base, FileBase, and FileGame interfaces.
        It implements a single session.

        This shouldn't normally appear in production code. */
    class InternalFileServer : public server::interface::ComposableCommandHandler {
     public:
        /** Default constructor.
            Makes an empty filespace. */
        InternalFileServer();

        /** Destructor. */
        ~InternalFileServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        InternalDirectoryHandler::Directory m_rootDir;
        DirectoryItem m_rootDirItem;
        Root m_root;
        Session m_session;
    };

} }

#endif
