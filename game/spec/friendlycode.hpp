/**
  *  \file game/spec/friendlycode.hpp
  */
#ifndef C2NG_GAME_SPEC_FRIENDLYCODE_HPP
#define C2NG_GAME_SPEC_FRIENDLYCODE_HPP

#include "game/playerset.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "game/playerlist.hpp"
#include "game/map/planet.hpp"
#include "game/interpreterinterface.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

// /*! \class GFCode

//     This class defines a special friendly code. In particular, it
//     associates the code with a condition and description.

//     There also are methods for generating random codes and for
//     inquiring/managing the special fcode definitions. */
// //! A friendly code definition.
    class FriendlyCode {
     public:
        enum Flag {
            ShipCode,             ///< Works on ships.
            PlanetCode,           ///< Works on planets.
            StarbaseCode,         ///< Works on starbases
            CapitalShipCode,      ///< Works on capital ships only.
            AlchemyShipCode,      ///< Works on alchemy ships only.
            RegisteredCode,       ///< Works for registered players only.
            UnspecialCode         ///< Not a special friendly code per se.
        };
        typedef afl::bits::SmallSet<Flag> FlagSet_t;



        FriendlyCode();

        FriendlyCode(String_t code, String_t descriptionLine);

        ~FriendlyCode();

        const String_t& getCode() const;

        String_t getDescription(const PlayerList& playerList) const;

//     /** Get flags.
//         \return bitfield of fc_XXX */
        FlagSet_t getFlags() const;

//     /** Get set of players who can use this FCode. */
        PlayerSet_t getRaces() const;


        bool worksOn(const game::map::Object& o, game::config::HostConfiguration& config) const;
//     bool worksOn(const GShip& s) const;
        bool worksOn(const game::map::Planet& p, game::config::HostConfiguration& config) const;



     private:
        static bool parseFlags(const String_t& s, const char* data, FlagSet_t& flags, PlayerSet_t& races);

        void initFromString(const String_t& descriptionLine);

        String_t m_code;
        String_t m_description;
        PlayerSet_t m_races;
        FlagSet_t m_flags;
    };

} }

#endif
