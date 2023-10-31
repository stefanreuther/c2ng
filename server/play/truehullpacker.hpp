/**
  *  \file server/play/truehullpacker.hpp
  *  \brief Class server::play::TruehullPacker
  */
#ifndef C2NG_SERVER_PLAY_TRUEHULLPACKER_HPP
#define C2NG_SERVER_PLAY_TRUEHULLPACKER_HPP

#include "server/play/packer.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

namespace server { namespace play {

    /** Packer for "obj/truehull". */
    class TruehullPacker : public Packer {
     public:
        /** Constructor.
            @param shipList  Ship list (must be dynamically allocated)
            @param root      Root (must be dynamically allocated)
            @param firstSlot First slot to return (0=start with empty slot, 1=start with first player) */
        TruehullPacker(const game::spec::ShipList& shipList, const game::Root& root, int firstSlot);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        const game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        int m_firstSlot;
    };

} }

#endif
