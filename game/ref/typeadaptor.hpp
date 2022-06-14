/**
  *  \file game/ref/typeadaptor.hpp
  *  \brief Class game::ref::TypeAdaptor
  */
#ifndef C2NG_GAME_REF_TYPEADAPTOR_HPP
#define C2NG_GAME_REF_TYPEADAPTOR_HPP

#include "game/map/universe.hpp"
#include "game/map/objecttype.hpp"

namespace game { namespace ref {

    class List;

    /** Adaptor to access a List using ObjectType interface.
        TypeAdaptor accepts 1-based indexes that are forwarded (minus one) into the list. */
    class TypeAdaptor : public game::map::ObjectType {
     public:
        /** Constructor.
            @param list  List; must live sufficiently long
            @param univ  Universe to resolve references; must live sufficiently long */
        TypeAdaptor(const List& list, game::map::Universe& univ);

        // ObjectType:
        virtual game::map::Object* getObjectByIndex(Id_t index);
        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;

     private:
        const List& m_list;
        game::map::Universe& m_universe;
    };

} }

#endif
