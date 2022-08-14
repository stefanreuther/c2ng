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
        HistorySlotItem(const Session& session, Root& root, int32_t gameId, int turnNumber, int slotNumber, String_t slotName,
                        bool resultAccess, bool turnAccess);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        const Session& m_session;
        Root& m_root;
        const int m_gameId;
        const int m_turnNumber;
        const int m_slotNumber;
        const String_t m_slotName;
        const bool m_resultAccess;
        const bool m_turnAccess;
    };

} } }

#endif
