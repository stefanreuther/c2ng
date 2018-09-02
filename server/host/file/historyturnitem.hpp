/**
  *  \file server/host/file/historyturnitem.hpp
  *  \brief Class server::host::file::HistoryTurnItem
  */
#ifndef C2NG_SERVER_HOST_FILE_HISTORYTURNITEM_HPP
#define C2NG_SERVER_HOST_FILE_HISTORYTURNITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "game/playerset.hpp"

namespace server { namespace host { namespace file {

    /** Host File Hierarchy: "game/<id>/history/<turn>" node.
        This node contains a game's previous turn's data. */
    class HistoryTurnItem : public Item {
     public:
        /** Constructor.
            \param session Session (for access checking).
            \param root Root
            \param gameId Game Id. Caller has verified that user can access the game.
            \param turnNumber Turn number. Caller has verified that user can access this turn.
            \param resultAccess Set of result files accessible to the player
            \param turnAccess Set of turn files accessible to the player */
        HistoryTurnItem(Session& session, Root& root, int32_t gameId, int turnNumber, game::PlayerSet_t resultAccess, game::PlayerSet_t turnAccess);

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
        game::PlayerSet_t m_resultAccess;
        game::PlayerSet_t m_turnAccess;
    };

} } }

#endif
