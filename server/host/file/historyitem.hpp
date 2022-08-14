/**
  *  \file historyitem.hpp
  *  \brief Class server::host::file::HistoryItem
  */
#ifndef HISTORYITEM_HPP_INCLUDED
#define HISTORYITEM_HPP_INCLUDED

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/<id>/history" node.
        This node contains the game's previous turns. */
    class HistoryItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking).
            \param root Root
            \param gameId Game Id. Caller has verified that user can access the game. */
        HistoryItem(const Session& session, Root& root, int32_t gameId);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        class Loader;

        const Session& m_session;
        Root& m_root;
        const int m_gameId;
    };

} } }

#endif
