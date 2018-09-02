/**
  *  \file server/host/file/rootitem.hpp
  *  \brief Class server::host::file::RootItem
  */
#ifndef C2NG_SERVER_HOST_FILE_ROOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_ROOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: Root node. */
    class RootItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking)
            \param root Root */
        RootItem(Session& session, Root& root);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        Session& m_session;
        Root& m_root;
    };

} } }

#endif
