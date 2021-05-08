/**
  *  \file game/spec/fighter.hpp
  *  \brief Class game::spec::Fighter
  */
#ifndef C2NG_GAME_SPEC_FIGHTER_HPP
#define C2NG_GAME_SPEC_FIGHTER_HPP

#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/playerlist.hpp"
#include "game/spec/weapon.hpp"
#include "util/range.hpp"

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
        typedef util::Range<int32_t> Range_t;

        /** Upper limit for intervals we return, for formatting. */
        static const int32_t MAX_INTERVAL = 10000;

        /** Constructor.
            \param fighter Id (=player number)
            \param config Host configuration
            \param players Player list
            \param tx Translator */
        explicit Fighter(int id,
                         const game::config::HostConfiguration& config,
                         const PlayerList& players,
                         afl::string::Translator& tx);

        /** Get average recharge time.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param host Host version
            \param config Host configuration
            \return average time between firing twice range */
        Range_t getRechargeTime(const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get average number of strikes.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param host Host version
            \param config Host configuration
            \return number of strikes range */
        Range_t getNumStrikes(const HostVersion& host, const game::config::HostConfiguration& config) const;
    };

} }

#endif
