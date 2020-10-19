/**
  *  \file game/alliance/phosthandler.hpp
  *  \brief Class game::alliance::PHostHandler
  */
#ifndef C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP
#define C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP

#include "game/alliance/handler.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

namespace game { namespace alliance {

    /** Implementation of Handler for Host.
        Converts between alliances and
        - game::v3::Command::AddDropAlly
        - game::v3::Command::ConfigAlly
        - game::v3::Command::Enemies
        commands */
    class PHostHandler : public Handler {
     public:
        /** Constructor.
            \param version Version (used to determine available levels; see HostVersion::getVersion())
            \param turn    Turn (used to obtain game::v3::CommandExtra)
            \param session Session (used for Root > HostConfiguration)
            \param player  Player */
        PHostHandler(int32_t version, Turn& turn, Session& session, int player);
        ~PHostHandler();

        virtual void init(Container& allies, afl::string::Translator& tx);
        virtual void postprocess(Container& allies);
        virtual void handleChanges(const Container& allies);

     private:
        int32_t m_version;
        Turn& m_turn;

        // FIXME: I'm giving this guy a Session because that is guaranteed to out-live
        // the PHostHandler and the Turn. May we give it a Ref<Root> instead?
        Session& m_session;

        int m_player;
    };

} }

#endif
