/**
  *  \file server/host/file/gamerootitem.hpp
  *  \brief Class server::host::file::GameRootItem
  */
#ifndef C2NG_SERVER_HOST_FILE_GAMEROOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_GAMEROOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/" node.
        This node contains a list of games. */
    class GameRootItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking)
            \param root Root */
        GameRootItem(Session& session, Root& root);

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
