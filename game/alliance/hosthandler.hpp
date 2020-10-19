/**
  *  \file game/alliance/hosthandler.hpp
  *  \brief Class game::alliance::HostHandler
  */
#ifndef C2NG_GAME_ALLIANCE_HOSTHANDLER_HPP
#define C2NG_GAME_ALLIANCE_HOSTHANDLER_HPP

#include "game/alliance/handler.hpp"
#include "afl/base/types.hpp"
#include "game/turn.hpp"

namespace game { namespace alliance {

    /** Implementation of Handler for Host.
        Converts between alliances and game::v3::Command::TAlliance command
        (ffX/FFX/eeX friendly codes). */
    class HostHandler : public Handler {
     public:
        /** Constructor.
            \param version Version (used to determine available levels; see HostVersion::getVersion())
            \param turn    Turn (used to obtain game::v3::CommandExtra)
            \param player  Player */
        HostHandler(int32_t version, Turn& turn, int player);
        ~HostHandler();

        // Handler:
        virtual void init(Container& allies, afl::string::Translator& tx);
        virtual void postprocess(Container& allies);
        virtual void handleChanges(const Container& allies);

     private:
        int32_t m_version;
        Turn& m_turn;
        int m_player;
    };

} }

#endif
