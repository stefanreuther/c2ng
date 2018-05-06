/**
  *  \file game/spec/friendlycode.hpp
  *  \brief Class game::spec::FriendlyCode
  */
#ifndef C2NG_GAME_SPEC_FRIENDLYCODE_HPP
#define C2NG_GAME_SPEC_FRIENDLYCODE_HPP

#include "game/playerset.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/interpreterinterface.hpp"
#include "game/playerlist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {
    class Ship;
    class Planet;
    class Object;
} }

namespace game { namespace spec {

    class ShipList;

    /** Special friendly code.
        This class defines a special friendly code.
        In particular, it associates the code with a condition and description. */
    class FriendlyCode {
     public:
        /** Friendly code flag. */
        enum Flag {
            ShipCode,             ///< Works on ships.
            PlanetCode,           ///< Works on planets.
            StarbaseCode,         ///< Works on starbases
            CapitalShipCode,      ///< Works on capital ships only.
            AlchemyShipCode,      ///< Works on alchemy ships only.
            RegisteredCode,       ///< Works for registered players only.
            UnspecialCode         ///< Not a special friendly code per se.
        };

        /** Set of friendly code flags. */
        typedef afl::bits::SmallSet<Flag> FlagSet_t;

        /** Default constructor.
            Not normally used. */
        FriendlyCode();

        /** Construct from definition.
            This handles a fcodes.cc line that originally contained code+","+descriptionLine.
            \param code Friendly code
            \param descriptionLine Description line, consisting of flags, a comma, and description text.
            \throw std::runtime_error descriptionLine is invalid */
        FriendlyCode(String_t code, String_t descriptionLine);

        /** Destructor. */
        ~FriendlyCode();

        /** Get friendly code.
            \return code */
        const String_t& getCode() const;

        /** Get description.
            \param playerList Player list used to render player name placeholders.
            \return formatted description */
        String_t getDescription(const PlayerList& playerList) const;

        /** Get flags.
            \return Flags */
        FlagSet_t getFlags() const;

        /** Get set of races who can use this friendly code.
            \return set of races */
        PlayerSet_t getRaces() const;

        /** Check whether this friendly code works on an object.
            \param o Object
            \param scoreDefinitions Ship score definitions
            \param shipList Ship list
            \param config Host configuration
            \return true if friendly code is a valid/sensible choice for this object */
        bool worksOn(const game::map::Object& o,
                     const UnitScoreDefinitionList& scoreDefinitions,
                     const game::spec::ShipList& shipList,
                     const game::config::HostConfiguration& config) const;

        /** Check whether this friendly code works on a ship.
            \param s Ship
            \param scoreDefinitions Ship score definitions
            \param shipList Ship list
            \param config Host configuration
            \return true if friendly code is a valid/sensible choice for this ship */
        bool worksOn(const game::map::Ship& s,
                     const UnitScoreDefinitionList& scoreDefinitions,
                     const game::spec::ShipList& shipList,
                     const game::config::HostConfiguration& config) const;

        /** Check whether this friendly code works on a planet.
            \param p Planet
            \param config Host configuration
            \return true if friendly code is a valid/sensible choice for this planet */
        bool worksOn(const game::map::Planet& p, const game::config::HostConfiguration& config) const;

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
