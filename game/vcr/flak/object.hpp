/**
  *  \file game/vcr/flak/object.hpp
  *  \brief Class game::vcr::flak::Object
  */
#ifndef C2NG_GAME_VCR_FLAK_OBJECT_HPP
#define C2NG_GAME_VCR_FLAK_OBJECT_HPP

#include "afl/charset/charset.hpp"
#include "game/vcr/flak/structures.hpp"
#include "game/vcr/object.hpp"

namespace game { namespace vcr { namespace flak {

    class Configuration;

    /** FLAK Object.
        Extends the regular Object with additional fields for FLAK. */
    class Object : public game::vcr::Object {
     public:
        /** Constructor.
            Make an empty object. */
        Object();

        /** Construct from disk representation.
            \param ship    Disk representation
            \param charset Character set to decode the name */
        Object(const game::vcr::flak::structures::Ship& ship, afl::charset::Charset& charset);

        /** Pack into disk representation.
            \param [out] ship     Disk representation
            \param [in]  charset  Character set to encode the name */
        void pack(game::vcr::flak::structures::Ship& ship, afl::charset::Charset& charset) const;

        /** Get maximum number of fighters launched concurrently.
            \return maximum number of fighters */
        int getMaxFightersLaunched() const;

        /** Get rating for targeting.
            \return rating */
        int32_t getRating() const;

        /** Get compensation for weapon hits.
            \return compensation */
        int getCompensation() const;

        /** Get ending status.
            FLAK transmits a hint for the ending status with the fight.
            \return status (-1: destroyed, 0: survived, otherwise: captured) */
        int getEndingStatus() const;

        /** Set maximum number of fighters launched concurrently.
            \param n Number of fighters */
        void setMaxFightersLaunched(int n);

        /** Set rating for targeting.
            \param rating Rating */
        void setRating(int32_t rating);

        /** Set compensation for weapon hits.
            \param comp Compensation */
        void setCompensation(int comp);

        /** Set ending status.
            \param status Status (-1: destroyed, 0: survived, otherwise: captured) */
        void setEndingStatus(int status);

        /** Initialize extra fields.
            Sets the extra fields from values computed according to the configuration.
            \param config FLAK configuration */
        void init(const Configuration& config);

     private:
        int m_maxFightersLaunched;
        int32_t m_rating;
        int m_compensation;
        int m_endingStatus;
    };

} } }

#endif
