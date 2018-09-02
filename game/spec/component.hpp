/**
  *  \file game/spec/component.hpp
  *  \brief Class game::spec::Component
  */
#ifndef C2NG_GAME_SPEC_COMPONENT_HPP
#define C2NG_GAME_SPEC_COMPONENT_HPP

#include "game/spec/componentnameprovider.hpp"
#include "afl/base/deletable.hpp"
#include "game/spec/cost.hpp"

namespace game { namespace spec {

    /** A starship component.
        This is the base class for all starship components (hulls, beams, etc.).
        It only holds data which it does not interpret or limit.

        Each component has a positive, nonzero Id that is immutable and defines its place in a ComponentVector.

        Each component has a name and an optional short name.
        These are stored as they are in the specification files.
        The ComponentNameProvider allows translation and formatting of the names.
        For this to work, the Component also has an immutable type.

        All other attributes can be changed.

        Components are not expected to change much during normal operation and therefore have no change tracking.
        Because components only store data and have no behaviour,
        the Component hierarchy has no virtual methods to obtain properties, but implement the data storage directly. */
    class Component : public afl::base::Deletable {
     public:
        /** Constructor.
            \param type Component type
            \param id Id */
        Component(ComponentNameProvider::Type type, int id);

        /** Get Id.
            \return Id */
        int getId() const;

        /** Get mass of this component.
            \return mass (defaults to 1) */
        int getMass() const;

        /** Set mass of this component.
            \param mass new mass */
        void setMass(int mass);

        /** Get tech level of this component.
            \param tech level (defaults to 1) */
        int getTechLevel() const;

        /** Set tech level of this component.
            \return tech level */
        void setTechLevel(int level);

        /** Get cost of this component.
            \return cost object */
        Cost& cost();

        /** Get cost of this component.
            \return cost object
            \overload */
        const Cost& cost() const;

        /** Get name of this component.
            \param provider ComponentNameProvider providing translation and formatting of the name
            \return name */
        String_t getName(const ComponentNameProvider& provider) const;

        /** Set name of this component.
            \param name New name */
        void setName(String_t name);

        /** Get short name of this component.
            \param provider ComponentNameProvider providing translation and formatting of the name
            \return short name */
        String_t getShortName(const ComponentNameProvider& provider) const;

        /** Set short name of this component.
            \param name New short name */
        void setShortName(String_t shortName);

     private:
        const ComponentNameProvider::Type m_type;
        const int m_id;

        int m_mass;
        int m_techLevel;
        Cost m_cost;
        String_t m_name;
        String_t m_shortName;
    };

} }


// Get Id.
inline int
game::spec::Component::getId() const
{
    return m_id;
}

// Get mass of this component.
inline int
game::spec::Component::getMass() const
{
    return m_mass;
}

// Set mass of this component.
inline void
game::spec::Component::setMass(int mass)
{
    m_mass = mass;
}

// Get tech level of this component.
inline int
game::spec::Component::getTechLevel() const
{
    return m_techLevel;
}

// Set tech level of this component.
inline void
game::spec::Component::setTechLevel(int level)
{
    m_techLevel = level;
}

// Get cost of this component.
inline game::spec::Cost&
game::spec::Component::cost()
{
    return m_cost;
}

// Get cost of this component.
inline const game::spec::Cost&
game::spec::Component::cost() const
{
    return m_cost;
}

// Get name of this component.
inline String_t
game::spec::Component::getName(const ComponentNameProvider& provider) const
{
    return provider.getName(m_type, m_id, m_name);
}

// Get short name of this component.
inline String_t
game::spec::Component::getShortName(const ComponentNameProvider& provider) const
{
    return provider.getShortName(m_type, m_id, m_name, m_shortName);
}

#endif
