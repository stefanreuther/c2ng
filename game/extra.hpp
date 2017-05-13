/**
  *  \file game/extra.hpp
  *  \brief Base class game::Extra
  */
#ifndef C2NG_GAME_EXTRA_HPP
#define C2NG_GAME_EXTRA_HPP

#include "afl/base/deletable.hpp"

namespace game {

    /** Extra information container.
        This class serves as a base class for opaque extra information.
        Extra information is objects that don't exist in every game type, for example,
        - nonstandard map objects
        - password and file keys

        \see ExtraIdentifier, ExtraContainer */
    class Extra : public afl::base::Deletable { };

}

#endif
