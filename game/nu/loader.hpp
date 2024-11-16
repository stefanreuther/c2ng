/**
  *  \file game/nu/loader.hpp
  *  \brief Class game::nu::Loader
  */
#ifndef C2NG_GAME_NU_LOADER_HPP
#define C2NG_GAME_NU_LOADER_HPP

#include "afl/data/access.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/timestamp.hpp"
#include "game/turn.hpp"

namespace game { namespace nu {

    /** Nu Loader.
        Aggregates most result parsing logic (conversion of JSON tree into native data). */
    class Loader {
     public:
        /** Constructor.
            @param tx           Translator (for log/exception messages)
            @param log          Logger (for log messages) */
        Loader(afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Load ship list.
            Loads applicable parts of the ship list.
            - advantages
            - hulls
            - beams
            - torpedoes
            - engines
            - hull assignments
            - hull functions

            Updates the root with result file information:
            - host configuration
            - race names

            Errors are logged.

            @param [out]    shipList   Ship list
            @param [in,out] root       Root, containing configuration and player names
            @param [in]     in         Data. Must be a hash containing a "rst" element */
        void loadShipList(game::spec::ShipList& shipList, Root& root, afl::data::Access in) const;

        /** Load turn data.
            Loads
            - turn metadata
            - ships
            - planets
            - starbases
            - minefields
            - ion storms
            - VCRs

            Errors are logged.
            Fatal errors cause an afl::except::InvalidDataException to be thrown.
            Fatal errors include un-representable Ids and starbases without planet.

            @param [in,out] turn       Turn
            @param [in]     playerSet  Source flag to use for own units
            @param [in]     in         Data. Must be a hash containing a "rst" element */
        void loadTurn(Turn& turn, PlayerSet_t playerSet, afl::data::Access in) const;

        /** Load timestamp.
            Converts the format used by Nu into a Timestamp object.
            If format cannot be recognized, returns a default-initialized timestap.

            @param a  Data element
            @return timestamp */
        static Timestamp loadTime(afl::data::Access a);

     private:
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
