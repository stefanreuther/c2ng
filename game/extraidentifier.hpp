/**
  *  \file game/extraidentifier.hpp
  *  \brief Class game::ExtraIdentifier
  */
#ifndef C2NG_GAME_EXTRAIDENTIFIER_HPP
#define C2NG_GAME_EXTRAIDENTIFIER_HPP

namespace game {

    struct BaseExtraIdentifier { };

    /** Identifier for an extra data item.
        See ExtraContainer for a description.

        A module that wishes to create extra data item must create a static const instance of ExtraIdentifier.
        This instance identifies the data item.
        It contains no data; it's just its address.
        It is usually an error to create an ExtraIdentifier that is not static const.

        \tparam Container identifies the containing object to avoid misplacing the ExtraIdentifier
        \tparam Value type of extra data item; must be derived from Extra. */
    template<typename Container, typename Value>
    struct ExtraIdentifier { BaseExtraIdentifier base; };

}

#endif
