/**
  *  \file game/spec/basecomponentvector.hpp
  *  \brief Class game::spec::BaseComponentVector
  */
#ifndef C2NG_GAME_SPEC_BASECOMPONENTVECTOR_HPP
#define C2NG_GAME_SPEC_BASECOMPONENTVECTOR_HPP

#include "game/spec/component.hpp"
#include "afl/functional/mapping.hpp"
#include "afl/container/ptrvector.hpp"

namespace game { namespace spec {

    /** ComponentVector common base class.
        This class is the common base class to all ComponentVector instances to share common code.
        It is not intended to be used directly. */
    class BaseComponentVector {
     public:
        class Names;

        /** Get a component by number.
            \param id Component number, [1,size()]
            \return Component if id is valid, null otherwise */
        Component* get(int id) const;

        /** Clear.
            Deletes all content. */
        void clear();

        /** Get number of components.
            \return Number of components */
        int size() const;

        /** Find next component, given an Id.
            \param id Starting point. 0=get first component.
            \return Component with getId() > id, or null if none */
        Component* findNext(int id) const;

        /** Get short names.
            Returns a mapping that produces short names (Component::getShortName).
            \param provider ComponentNameProvider to use */
        Names shortNames(const ComponentNameProvider& provider) const;

        /** Get component names.
            Returns a mapping that produces component names (Component::getName).
            \param provider ComponentNameProvider to use */
        Names names(const ComponentNameProvider& provider) const;

     protected:
        /** Set new element.
            Callers are trusted to not provide unreasonable Ids.
            Invalid Ids (<= 0) cause the call to be ignored and destroy \c p.
            \param id Component number, >= 1
            \param p Newly-allocated component */
        void setNew(int id, Component* p);

     private:
        afl::container::PtrVector<Component> m_components;
    };

} }

/*
 *  Implementation of names(), shortNames()
 */
class game::spec::BaseComponentVector::Names : public afl::functional::Mapping<int,String_t> {
 public:
    Names(const BaseComponentVector& vec, const ComponentNameProvider& provider, bool shortName)
        : m_vec(vec),
          m_provider(provider),
          m_shortName(shortName)
        { }
    virtual bool getFirstKey(int& a) const;
    virtual bool getNextKey(int& a) const;
    virtual String_t get(int a) const;
 private:
    const BaseComponentVector& m_vec;
    const ComponentNameProvider& m_provider;
    const bool m_shortName;
};

// Get short names.
inline game::spec::BaseComponentVector::Names
game::spec::BaseComponentVector::shortNames(const ComponentNameProvider& provider) const
{
    return Names(*this, provider, true);
}

// Get component names.
inline game::spec::BaseComponentVector::Names
game::spec::BaseComponentVector::names(const ComponentNameProvider& provider) const
{
    return Names(*this, provider, false);
}

// Clear.
inline void
game::spec::BaseComponentVector::clear()
{
    m_components.clear();
}

// Get number of components.
inline int
game::spec::BaseComponentVector::size() const
{
    return int(m_components.size());
}

#endif
