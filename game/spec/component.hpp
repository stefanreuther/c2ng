/**
  *  \file game/spec/component.hpp
  */
#ifndef C2NG_GAME_SPEC_COMPONENT_HPP
#define C2NG_GAME_SPEC_COMPONENT_HPP

#include "game/spec/componentnameprovider.hpp"
#include "afl/base/deletable.hpp"
#include "game/spec/cost.hpp"

namespace game { namespace spec {

    class Component : public afl::base::Deletable {
     public:
        Component(ComponentNameProvider::Type type, int id);

        int getId() const;

        int getMass() const;
        void setMass(int mass);

        int getTechLevel() const;
        void setTechLevel(int level);

        Cost& cost();
        const Cost& cost() const;

        String_t getName(const ComponentNameProvider& provider) const;
        void setName(String_t name);

        String_t getShortName(const ComponentNameProvider& provider) const;
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

#endif
