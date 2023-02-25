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

namespace game {
    class RegistrationKey;
}

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
            UnspecialCode,        ///< Not a special friendly code per se.
            PrefixCode            ///< This is a prefix, not a friendly code.
        };

        /** Set of friendly code flags. */
        typedef afl::bits::SmallSet<Flag> FlagSet_t;


        /** Friendly code filter.
            Stores information to apply a worksOn() check.
            This captures the necessary data to avoid that users trying to filter friendly codes
            need to provide many/complex function signatures. */
        class Filter {
         public:
            friend class FriendlyCode;

            /** Default constructor.
                Makes a filter that never matches. */
            Filter()
                : flags(), race()
                { }

            /** Construct from attributes.
                Makes a filter that matches according to the given parameters.

                Flags must have at least one of ShipCode, PlanetCode, StarbaseCode;
                only friendly codes that have at least one matching type are accepted
                (e.g. a friendly code with ShipCode+PlanetCode is accepted when the filter includes ShipCode).

                Friendly codes with CapitalShipCode and/or AlchemyShipCode are accepted
                only if the flags include those flags.

                \param flags  Flags.
                \param race   Race. Only friendly codes available to that race are accepted. */
            Filter(FlagSet_t flags, int race)
                : flags(flags), race(race)
                { }

            /** Construct from object.
                Automatically decides depending on the object's dynamic type.
                \param obj Object
                \param scoreDefinitions Ship score definitions
                \param shipList Ship list
                \param config Host configuration
                \return filter */
            static Filter fromObject(const game::map::Object& obj, const UnitScoreDefinitionList& scoreDefinitions, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config);

            /** Construct from ship.
                \param sh Ship
                \param scoreDefinitions Ship score definitions
                \param shipList Ship list
                \param config Host configuration
                \return filter */
            static Filter fromShip(const game::map::Ship& sh, const UnitScoreDefinitionList& scoreDefinitions, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config);

            /** Construct from planet.
                \param p Planet
                \param config Host configuration
                \return filter */
            static Filter fromPlanet(const game::map::Planet& p, const game::config::HostConfiguration& config);

         private:
            FlagSet_t flags;
            int race;
        };


        /** Default constructor.
            Not normally used. */
        FriendlyCode();

        /** Construct from definition.
            This handles a fcodes.cc line that originally contained code+","+descriptionLine.
            \param code Friendly code
            \param descriptionLine Description line, consisting of flags, a comma, and description text.
            \param tx Translator (for error messages)
            \throw std::runtime_error descriptionLine is invalid */
        FriendlyCode(String_t code, String_t descriptionLine, afl::string::Translator& tx);

        /** Destructor. */
        ~FriendlyCode();

        /** Get friendly code.
            \return code */
        const String_t& getCode() const;

        /** Get description.
            \param playerList Player list used to render player name placeholders.
            \param tx Translator (for default player names)
            \return formatted description */
        String_t getDescription(const PlayerList& playerList, afl::string::Translator& tx) const;

        /** Get flags.
            \return Flags */
        FlagSet_t getFlags() const;

        /** Get set of races who can use this friendly code.
            \return set of races */
        PlayerSet_t getRaces() const;

        /** Check whether this friendly code works on an object defined by a filter.
            \param f Filter
            \return true if friendly code is a valid/sensible choice for the unit defined by the filter */
        bool worksOn(const Filter& f) const;

        /** Check whether this friendly code is allowed according to registration status.
            \param key Key
            \return true if code is allowed */
        bool isPermitted(const RegistrationKey& key) const;

     private:
        static bool parseFlags(const String_t& s, const char* data, FlagSet_t& flags, PlayerSet_t& races);

        void initFromString(const String_t& descriptionLine, afl::string::Translator& tx);

        String_t m_code;
        String_t m_description;
        PlayerSet_t m_races;
        FlagSet_t m_flags;
    };

} }

#endif
