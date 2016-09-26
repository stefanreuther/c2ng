/**
  *  \file game/map/objectlist.hpp
  */
#ifndef C2NG_GAME_MAP_OBJECTLIST_HPP
#define C2NG_GAME_MAP_OBJECTLIST_HPP

#include <vector>
#include "game/map/objecttype.hpp"
#include "game/map/objectreference.hpp"

namespace game { namespace map {

    // /** Object list. Container for a list of objects (GObjectReference's). It offers a number
    //     of functions to add objects and bring them into a form convenient to the user.
    //     It also contains a GNoteContainer to allow adding additional selectable items
    //     and dividers. */
    class ObjectList : public ObjectType {
     public:
        // typedef std::vector<GObjectReference> container_t;
        // typedef container_t::const_iterator iterator;
        // typedef container_t::size_type size_type;

        // Construction:
        ObjectList();
        ~ObjectList();

        // Adding things:
        void         addObject(ObjectReference ref);
        void         addObject(ObjectType& type, int index);
        // void         addObjectsAt(GUniverse& univ, GPoint pt, int flags, int exclude_ship);
        // void         addNonObject(int id, string_t name);

        // // Manipulation:
        // void         sort(int func(GObject&,GObject&));
        // void         sort(int32_t func(GObject&));
        // void         sort(GObjectCompare& cmp);
        void clear();
        // void         addDividers(string_t func(GObject&));
        // void         addDividers(GObjectDivider& func);
        // void         removeDividers();
        // void         postprocessUser(bool with_dividers);

        // // Container access: (0-based)
        // iterator     begin() const;
        // iterator     end() const;
        // size_type    size() const;
        // const GObjectReference& operator[](size_type index) const;

        // // GObjectType: (1-based!)
        virtual Object* getObjectByIndex(Id_t index);
        virtual Universe* getUniverseByIndex(Id_t index);
        virtual Id_t getNextIndex(Id_t index) const;
        virtual Id_t getPreviousIndex(Id_t index) const;

        ObjectReference getObjectReferenceByIndex(Id_t index) const;

        Id_t getIndexFor(ObjectReference ref) const;
        Id_t getIndexFor(const Object& obj) const;

        // size_type    countGameObjects() const;

     private:
        typedef std::vector<ObjectReference> Container_t;
        Container_t m_list;
        // GNoteContainer notes;
    };
    

} }

#endif
