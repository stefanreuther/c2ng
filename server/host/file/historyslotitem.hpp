/**
  *  \file server/host/file/historyslotitem.hpp
  *  \brief Class server::host::file::HistorySlotItem
  */
#ifndef C2NG_SERVER_HOST_FILE_HISTORYSLOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_HISTORYSLOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/<id>/history/<turn>/<slot>" node.
        This node contains a game's previous turn's data for one player. */
    class HistorySlotItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking).
            \param root Root
            \param gameId Game Id. Caller has verified that user can access the game.
            \param turnNumber Turn number. Caller has verified that user can access this turn.
            \param slotNumber Slot number.
            \param slotName Slot name
            \param resultAccess true if player can access result files
            \param turnAccess true if player can access turn files */
        HistorySlotItem(Session& session, Root& root, int32_t gameId, int turnNumber, int slotNumber, String_t slotName,
                        bool resultAccess, bool turnAccess);

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
        int m_turnNumber;
        int m_slotNumber;
        String_t m_slotName;
        bool m_resultAccess;
        bool m_turnAccess;
    };

} } }

#endif
