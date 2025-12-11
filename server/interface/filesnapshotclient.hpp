/**
  *  \file server/interface/filesnapshotclient.hpp
  *  \brief Class server::interface::FileSnapshotClient
  */
#ifndef C2NG_SERVER_INTERFACE_FILESNAPSHOTCLIENT_HPP
#define C2NG_SERVER_INTERFACE_FILESNAPSHOTCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/filesnapshot.hpp"

namespace server { namespace interface {

    /** Client for File Server Snapshot Interface. */
    class FileSnapshotClient : public FileSnapshot {
     public:
        /** Constructor.
            @param commandHandler CommandHandler to transmit commands */
        explicit FileSnapshotClient(afl::net::CommandHandler& commandhandler);
        ~FileSnapshotClient();

        // FileSnapshot:
        virtual void createSnapshot(String_t name);
        virtual void copySnapshot(String_t oldName, String_t newName);
        virtual void removeSnapshot(String_t name);
        virtual void listSnapshots(afl::data::StringList_t& out);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
