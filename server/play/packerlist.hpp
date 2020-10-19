/**
  *  \file server/play/packerlist.hpp
  *  \brief Class server::play::PackerList
  */
#ifndef C2NG_SERVER_PLAY_PACKERLIST_HPP
#define C2NG_SERVER_PLAY_PACKERLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "server/play/packer.hpp"
#include "server/types.hpp"

namespace server { namespace play {

    /** List of Packer objects.
        A list of commands may produce a list of output objects, represented as a list of Packer objects.
        For example, the command "sendmessage" command addressed at obj/main will invalidate
        obj/main as well as obj/outidx.

        PackerList maintains a list of Packer objects, making sure each one (identified by its Packer::getName())
        appears only once.

        We need to store the Packers and not the values they produce because during collection
        of the values, data may change. */
    class PackerList {
     public:
        /** Default constructor.
            Make empty list. */
        PackerList();

        /** Destructor. */
        ~PackerList();

        /** Add new packer.
            \param p Packer. PackerList takes ownership. */
        void addNew(Packer* p);

        /** Build result value.
            Produces a hash with all the Packer's buildValue()s.
            \return newly-allocated hash; caller takes ownership */
        Value_t* buildValue() const;

     private:
        afl::container::PtrVector<Packer> m_packers;
    };

} }

#endif
