/**
  *  \file game/ref/sortbytransfertarget.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYTRANSFERTARGET_HPP
#define C2NG_GAME_REF_SORTBYTRANSFERTARGET_HPP

#include <utility>
#include "game/ref/sortpredicate.hpp"
#include "game/map/ship.hpp"

namespace game { namespace ref {

    class SortByTransferTarget : public SortPredicate {
     public:
        /** Constructor.
            \param univ Universe
            \param transporterId Transporter to check
            \param checkOther true to check the other transporter, too.
                              Pass !HostVersion::hasParallelShipTransfers() here.
            \param tx Translator */
        SortByTransferTarget(const game::map::Universe& univ,
                             game::map::Ship::Transporter transporterId,
                             bool checkOther,
                             afl::string::Translator& tx);
        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        Reference getTarget(const Reference a) const;

     private:
        const game::map::Universe& m_universe;
        game::map::Ship::Transporter m_transporterId;
        bool m_checkOther;
        afl::string::Translator& m_translator;
    };

} }

#endif
