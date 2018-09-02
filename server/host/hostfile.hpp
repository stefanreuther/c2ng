/**
  *  \file server/host/hostfile.hpp
  *  \brief Class server::host::HostFile
  */
#ifndef C2NG_SERVER_HOST_HOSTFILE_HPP
#define C2NG_SERVER_HOST_HOSTFILE_HPP

#include "server/interface/hostfile.hpp"
#include "server/host/file/item.hpp"

namespace server { namespace host {

    /** Implementation of HostFile interface.
        This interface implements LS/STAT/GET/PSTAT commands. */
    class HostFile : public server::interface::HostFile {
     public:
        /** Constructor.

            Note: to simplify testing, this takes a server::host::file::Item,
            not a Session/Root pair like the other interface handlers.
            For production, the root item will be a server::host::file::RootItem.

            \param item Root item */
        explicit HostFile(server::host::file::Item& item);

        // HostFile interface:
        virtual String_t getFile(String_t fileName);
        virtual void getDirectoryContent(String_t dirName, InfoVector_t& result);
        virtual Info getFileInformation(String_t fileName);
        virtual void getPathDescription(String_t dirName, InfoVector_t& result);

     private:
        server::host::file::Item& m_item;
    };

} }

#endif
