/**
  *  \file game/extracontainer.hpp
  *  \brief Template class ExtraContainer
  */
#ifndef C2NG_GAME_EXTRACONTAINER_HPP
#define C2NG_GAME_EXTRACONTAINER_HPP

#include "afl/container/ptrmap.hpp"
#include "game/extra.hpp"
#include "game/extraidentifier.hpp"

namespace game {

    /** Base class for ExtraContainer.
        This class is used for sharing implementations. */
    class BaseExtraContainer {
     protected:
        BaseExtraContainer();
        ~BaseExtraContainer();
        afl::container::PtrMap<const BaseExtraIdentifier*, Extra> m_data;
    };

    /** Container for extra data items.
        It might be needed to associate extra data with an object.
        For example, a loader implementation might need to attach a particular information to a game::Turn which is needed for saving later.
        An ExtraContainer manages this information.

        The ExtraContainer has a template parameter (Container) that serves to distinguish between different ExtraContainer instances.
        The ExtraContainer only accepts extra information identified by a matching ExtraIdentifier.
        By convention, this is the same type as the object containing the ExtraContainer;
        for example, Turn contains an ExtraContainer<Turn>.

        Extra information is identified by ExtraIdentifier instances.
        The instance's address serves as a primary key.
        This way, we can use the linker to generate unique identifiers and don't have to resort to strings.
        Each module that stores extra information will create an ExtraIdentifier that specifies the container and object type of the desired object.

        All extra information objects must be derived from Extra.

        ExtraContainer takes ownership of the objects it contains.

        \tparam Container identifies the containing object */
    template<typename Container>
    class ExtraContainer : public BaseExtraContainer {
     public:
        /** Default constructor.
            Makes an empty container. */
        ExtraContainer()
            { }

        /** Destructor. */
        ~ExtraContainer()
            { }

        /** Get existing value.
            \param id Identifier
            \return Existing value; null if none. */
        template<typename Value>
        Value* get(const ExtraIdentifier<Container,Value>& id)
            { return static_cast<Value*>(m_data[&id.base]); }

        /** Get existing or create new value.
            \param id Identifier
            \return Reference to value; default-constructed if it did not exist before */
        template<typename Value>
        Value& create(const ExtraIdentifier<Container,Value>& id)
            {
                Value* p = get(id);
                if (!p) {
                    p = setNew(id, new Value());
                }
                return *p;
            }

        /** Set new value.
            \param id Identifier
            \param p Newly-allocated value or null. ExtraContainer takes ownership.
            \return p */
        template<typename Value>
        Value* setNew(const ExtraIdentifier<Container,Value>& id, Value* p)
            {
                m_data.insertNew(&id.base, p);
                return p;
            }
    };

}

#endif
