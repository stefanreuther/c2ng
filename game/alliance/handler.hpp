/**
  *  \file game/alliance/handler.hpp
  */
#ifndef C2NG_GAME_ALLIANCE_HANDLER_HPP
#define C2NG_GAME_ALLIANCE_HANDLER_HPP

#include "afl/base/deletable.hpp"

namespace game { namespace alliance {

    class Container;

    // /** Alliance handler.
    //     Provides a callback to update alliances. */
    // FIXME: rework this?
    class Handler : public afl::base::Deletable {
     public:
        // Initialize. Called by addNewHandler().
        virtual void init(Container&) = 0;

        // Postprocess after loading: update the container
        virtual void postprocess(Container&) = 0;

        // Write changes to the container back
        virtual void handleChanges(const Container&) = 0;
    };

} }

#endif
