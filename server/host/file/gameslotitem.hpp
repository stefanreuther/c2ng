/**
  *  \file server/host/file/gameslotitem.hpp
  *  \brief Class server::host::file::GameSlotItem
  */
#ifndef C2NG_SERVER_HOST_FILE_GAMESLOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_GAMESLOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/<id>/<slot>" node.
        This node contains the files for the given slot. */
    class GameSlotItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking).
            \param root Root
            \param gameId Game Id. Caller has verified that user can access the game.
            \param slotId Slot Id. Caller has verified that user can access this slot.
            \param slotName Name of this slot; returned as server::interface::HostFile::Info::slotName (and should therefore normally be given). */
        GameSlotItem(const Session& session, Root& root, int32_t gameId, int slotId, afl::base::Optional<String_t> slotName);

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
        const int m_slotId;
        const afl::base::Optional<String_t> m_slotName;
    };

} } }

#endif
