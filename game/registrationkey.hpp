/**
  *  \file game/registrationkey.hpp
  *  \brief Base class game::RegistrationKey
  */
#ifndef C2NG_GAME_REGISTRATIONKEY_HPP
#define C2NG_GAME_REGISTRATIONKEY_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/types.hpp"

namespace game {

    /** Registration key / "premium" status.
        Loader provides a key depending on the game type.
        The key informs about gaming restrictions and bears some identifying information. */
    class RegistrationKey : public afl::base::Deletable {
     public:
        /** Key status. */
        enum Status {
            Unknown,            ///< Unknown.
            Unregistered,       ///< Unregistered/standard.
            Registered          ///< Registered/premium.
        };

        /** Identifying information. */
        enum Line {
            Line1,              ///< First line (registration number or name).
            Line2,              ///< Second line (registration date or location).
            Line3,              ///< Third line (purely informational).
            Line4               ///< Fourth line (purely informational).
        };

        /** Get status.
            \return Status */
        virtual Status getStatus() const = 0;

        /** Get line.
            \param which Line to query
            \return Content (may be empty) */
        virtual String_t getLine(Line which) const = 0;

        /** Set line.
            A key may or may not accept changes.
            \param which Line to change
            \param value Value
            \return true if change accepted. */
        virtual bool setLine(Line which, String_t value) = 0;

        /** Get maximum tech level that can be bought.
            \param area Area to query
            \return value */
        virtual int getMaxTechLevel(TechLevel area) const = 0;
    };

}

#endif
