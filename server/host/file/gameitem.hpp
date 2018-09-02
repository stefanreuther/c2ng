/**
  *  \file server/host/file/gameitem.hpp
  *  \brief Class server::host::file::GameItem
  */
#ifndef C2NG_SERVER_HOST_FILE_GAMEITEM_HPP
#define C2NG_SERVER_HOST_FILE_GAMEITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/<id>" node.
        This node contains the game's player files. */
    class GameItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking).
            \param root Root
            \param gameId Game Id. Caller has verified that user can access the game. */
        GameItem(Session& session, Root& root, int32_t gameId);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        Session& m_session;
        Root& m_root;
        int m_gameId;
    };

} } }

#endif
