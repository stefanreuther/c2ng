/**
  *  \file game/alliance/phosthandler.hpp
  *  \brief Class game::alliance::PHostHandler
  */
#ifndef C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP
#define C2NG_GAME_ALLIANCE_PHOSTHANDLER_HPP

#include "game/alliance/handler.hpp"
#include "game/root.hpp"
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
            \param turn    Turn (used to obtain game::v3::CommandExtra)
            \param root    Root (used for game::config::HostConfiguration, game::HostVersion)
            \param player  Player */
        PHostHandler(Turn& turn, const afl::base::Ref<const Root>& root, int player);
        ~PHostHandler();

        virtual void init(Container& allies, afl::string::Translator& tx);
        virtual void postprocess(Container& allies);
        virtual void handleChanges(const Container& allies);

     private:
        Turn& m_turn;                       ///< Turn. As reference because PHostHandler lives inside it.
        afl::base::Ref<const Root> m_root;  ///< Root. Reference-counted because it is a separate object.
        int m_player;
    };

} }

#endif
