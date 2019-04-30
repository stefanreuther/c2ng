/**
  *  \file game/alliance/handler.hpp
  *  \brief Interface game::alliance::Handler
  */
#ifndef C2NG_GAME_ALLIANCE_HANDLER_HPP
#define C2NG_GAME_ALLIANCE_HANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace alliance {

    class Container;

    /** Alliance handler.
        Different host versions have different types of alliances.
        This interface defines the host's capabilities and how to map it to game data. */
    class Handler : public afl::base::Deletable {
     public:
        /** Initialize this alliance handler.
            Called by Container::addNewHandler().
            This function should add all this handler's levels to the container.
            \param allies Container
            \param tx Translator */
        virtual void init(Container& allies, afl::string::Translator& tx) = 0;

        /** Postprocess after loading (import).
            If alliance information is stored elsewhere in turn data, this imports the data, and updates the container.
            Called after loading.
            \param allies Container */
        virtual void postprocess(Container& allies) = 0;

        /** Write changes to the container back (export).
            If alliance information is stored elsewhere in turn data, this exports the data from the container content.
            Called after a change to the container.
            \param allies Container */
        virtual void handleChanges(const Container& allies) = 0;
    };

} }

#endif
