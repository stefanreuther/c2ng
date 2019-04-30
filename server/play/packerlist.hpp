/**
  *  \file server/play/packerlist.hpp
  */
#ifndef C2NG_SERVER_PLAY_PACKERLIST_HPP
#define C2NG_SERVER_PLAY_PACKERLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "server/play/packer.hpp"
#include "server/types.hpp"

namespace server { namespace play {

    class PackerList {
     public:
        PackerList();
        ~PackerList();

        void addNew(Packer* p);

        Value_t* buildValue() const;

     private:
        afl::container::PtrVector<Packer> m_packers;
    };

} }

#endif
