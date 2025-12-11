/**
  *  \file server/file/filesnapshot.hpp
  *  \brief Class server::file::FileSnapshot
  */
#ifndef C2NG_SERVER_FILE_FILESNAPSHOT_HPP
#define C2NG_SERVER_FILE_FILESNAPSHOT_HPP

#include "server/interface/filesnapshot.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file {

    class Session;
    class Root;

    /** Implementation of the FileSnapshot interface. */
    class FileSnapshot : public server::interface::FileSnapshot {
     public:
        /** Constructor.
            \param session Session object (provides user context)
            \param root Root (provides file space, logging, config) */
        FileSnapshot(Session& session, Root& root);

        virtual void createSnapshot(String_t name);
        virtual void copySnapshot(String_t oldName, String_t newName);
        virtual void removeSnapshot(String_t name);
        virtual void listSnapshots(afl::data::StringList_t& out);

     private:
        Session& m_session;
        Root& m_root;

        DirectoryHandler::SnapshotHandler& handler();
        static void verifyName(const String_t& name);
    };

} }

#endif
