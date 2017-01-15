/**
  *  \file game/v3/trn/filter.hpp
  *  \brief Class game::v3::trn::Filter
  */
#ifndef C2NG_GAME_V3_TRN_FILTER_HPP
#define C2NG_GAME_V3_TRN_FILTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/deleter.hpp"
#include "game/v3/turnfile.hpp"

namespace game { namespace v3 { namespace trn {

    /** Base class for a turn command filter. */
    class Filter : public afl::base::Deletable {
     public:
        /** Check acceptance of a command.
            \param trn turn file we're looking at
            \param index command number we're looking at, 0-based
            \return true on match */
        virtual bool accept(const TurnFile& trn, size_t index) const = 0;

        /** Parse filter expression.
            \param text Filter expression
            \param deleter Deleter to manage lifetime of result structur
            \return Filter instance that lives at least as long as the deleter
            \throw ParseException on error */
        static const Filter& parse(String_t text, afl::base::Deleter& deleter);
    };

} } }

#endif
