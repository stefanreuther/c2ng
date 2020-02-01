/**
  *  \file game/spec/fighter.hpp
  *  \brief Class game::spec::Fighter
  */
#ifndef C2NG_GAME_SPEC_FIGHTER_HPP
#define C2NG_GAME_SPEC_FIGHTER_HPP

#include "game/spec/weapon.hpp"
#include "game/config/hostconfiguration.hpp"
#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"

namespace game { namespace spec {

    /** A fighter.
        This class only holds data which it does not interpret or limit.

        Fighters can be different for each player due to arrayized configuration.

        The intention is to have short-lived Fighter objects,
        and construct them whenever needed.
        The Fighter object does not automatically update when configuration/players change.
        If that is desired, it's up to the user. */
    class Fighter : public Weapon {
     public:
        /** Constructor.
            \param fighter Id (=player number)
            \param config Host configuration
            \param players Player list
            \param tx Translator */
        explicit Fighter(int id,
                         const game::config::HostConfiguration& config,
                         const PlayerList& players,
                         afl::string::Translator& tx);
    };

} }

#endif
