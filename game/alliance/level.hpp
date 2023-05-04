/**
  *  \file game/alliance/level.hpp
  *  \brief Class game::alliance::Level
  */
#ifndef C2NG_GAME_ALLIANCE_LEVEL_HPP
#define C2NG_GAME_ALLIANCE_LEVEL_HPP

#include <vector>
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"

namespace game { namespace alliance {

    /** Alliance level.
        Defines an alliance level.

        A Level object is effectively immutable after creation.
        Actual offers are stored in Offer.

        A Level is identified by a string, allowing alliance levels of different origin to coexist.

        Level objects are independant of each other,
        which makes it possible to copy them between game and UI thread. */
    class Level {
     public:
        /** Flag. */
        enum Flag {
            IsOffer,                 ///< This alliance level is the "offer an alliance" flag.
            NeedsOffer,              ///< This alliance level requires the "offer an alliance" flag set.
            IsEnemy,                 ///< This alliance level is an "enemy" flag.
            AllowConditional,        ///< This alliance level allows conditional offers.
            IsCombat                 ///< This alliance level is a "combat" level.
        };

        /** Set of flags. */
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Constructor.
            \param name  Human-friendly name, translated
            \param id    Internal identifier for program use; case-sensitive
            \param flags Flags describing the nature of this level. */
        Level(const String_t& name, const String_t& id, Flags_t flags);

        /** Destructor. */
        ~Level();

        /** Get human-friendly name.
            \return name */
        const String_t& getName() const;

        /** Get internal identifier.
            \return identifier */
        const String_t& getId() const;

        /** Get flags.
            \return flags */
        Flags_t getFlags() const;

        /** Check for flag.
            \param fl Flag to check for
            \return true if flag is present */
        bool hasFlag(Flag fl) const;

     private:
        String_t m_name;
        String_t m_id;
        Flags_t m_flags;
    };


    /** Vector of alliance levels. */
    // ex GAllianceLevels
    typedef std::vector<Level> Levels_t;

} }

#endif
