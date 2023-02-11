/**
  *  \file game/map/planeteffectors.hpp
  *  \brief Class game::map::PlanetEffectors
  */
#ifndef C2NG_GAME_MAP_PLANETEFFECTORS_HPP
#define C2NG_GAME_MAP_PLANETEFFECTORS_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

namespace game { namespace map {

    /** Summary of events affecting a planet.
        This is intended to be used for quick predictions.
        For simplicity, we store the totals of number-of-effects-happening. */
    class PlanetEffectors {
     public:
        enum Kind {
            Hiss,
            // RebelGroundAttack,
            // Pillage,
            // Meteor,
            HeatsTo50,
            CoolsTo50,
            HeatsTo100
        };
        static const size_t NUM_EFFECTS = HeatsTo100+1;

        /** Default constructor. */
        PlanetEffectors();

        /** Compare for equality.
            \param other Other object
            \return true if equal */
        bool operator==(const PlanetEffectors& other) const;
        bool operator!=(const PlanetEffectors& other) const;

        /** Clear.
            Sets all counters to 0. */
        void clear();

        /** Add an effect.
            \param eff Effect
            \param count Number to add */
        void add(Kind eff, int count);

        /** Set number of effect.
            \param eff Effect
            \param count Number to set */
        void set(Kind eff, int count);

        /** Get number of effect.
            \param eff Effect
            \return number */
        int get(Kind eff) const;

        /** Set number of terraformers.
            \return total number of terraformers (temperature changers) of any type */
        int getNumTerraformers() const;

        /** Describe.
            Produces a simple summary of this object.

            For simplicity, we assume all ships are owned by the same player
            (which normally is the same one as the one we play, and the one owning the planet).

            \param tx         Translator
            \param shipOwner  Owner of ship(s) invoking the effects
            \param config     Host configuration
            \return human-readable one-liner, never empty */
        String_t describe(afl::string::Translator& tx, int shipOwner, const game::config::HostConfiguration& config, const HostVersion& host) const;

     private:
        int m_effectors[NUM_EFFECTS];
    };

} }

#endif
