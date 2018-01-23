/**
  *  \file game/spec/componentnameprovider.hpp
  *  \brief Interface game::spec::ComponentNameProvider
  */
#ifndef C2NG_GAME_SPEC_COMPONENTNAMEPROVIDER_HPP
#define C2NG_GAME_SPEC_COMPONENTNAMEPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace game { namespace spec {

    /** Component Name Provider.
        This interface allows adjusting the names reported by a Component (see also there).
        Possible features include
        - translating the names
        - generating short names from long names
        - decorating names, e.g. to show the component number */
    class ComponentNameProvider : public afl::base::Deletable {
     public:
        /** Type of a component. */
        enum Type {
            Hull,
            Engine,
            Beam,
            Torpedo
        };

        /** Get regular name.
            \param type   Component type
            \param index  Index of component (e.g. hull number, beam number, etc.)
            \param name   Raw component name as stored in specification file
            \return adjusted name */
        virtual String_t getName(Type type, int index, const String_t& name) const = 0;

        /** Get short name.
            \param type   Component type
            \param index  Index of component (e.g. hull number, beam number, etc.)
            \param name   Raw component name as stored in specification file
            \param shortName Short component name as stored in specification file, if any
            \return adjusted name */
        virtual String_t getShortName(Type type, int index, const String_t& name, const String_t& shortName) const = 0;
    };

} }

#endif
