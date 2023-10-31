/**
  *  \file server/play/configurationpacker.hpp
  *  \brief Class server::play::ConfigurationPacker
  */
#ifndef C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP
#define C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/root.hpp"

namespace server { namespace play {

    /** Packer for "obj/cfgX". */
    class ConfigurationPacker : public Packer {
     public:
        /** Constructor.
            \param root    Root
            \param slice   Slice (0=everything, others=subset) */
        ConfigurationPacker(game::Root& root, int slice);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Root& m_root;
        int m_slice;
    };

} }

#endif
