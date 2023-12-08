/**
  *  \file server/play/flakconfigurationpacker.hpp
  *  \brief Class server::play::FlakConfigurationPacker
  */
#ifndef C2NG_SERVER_PLAY_FLAKCONFIGURATIONPACKER_HPP
#define C2NG_SERVER_PLAY_FLAKCONFIGURATIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/root.hpp"

namespace server { namespace play {

    /** Packer for "obj/flakconfig". */
    class FlakConfigurationPacker : public Packer {
     public:
        /** Constructor.
            \param root Root */
        explicit FlakConfigurationPacker(const game::Root& root);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        const game::Root& m_root;
    };

} }

#endif
