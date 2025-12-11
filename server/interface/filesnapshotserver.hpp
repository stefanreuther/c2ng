/**
  *  \file server/interface/filesnapshotserver.hpp
  *  \brief Class server::interface::FileSnapshotServer
  */
#ifndef C2NG_SERVER_INTERFACE_FILESNAPSHOTSERVER_HPP
#define C2NG_SERVER_INTERFACE_FILESNAPSHOTSERVER_HPP

#include "server/interface/filesnapshot.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class FileSnapshotServer : public ComposableCommandHandler {
     public:
        explicit FileSnapshotServer(FileSnapshot& impl);
        ~FileSnapshotServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        FileSnapshot& m_impl;
    };

} }

#endif
