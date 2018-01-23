/**
  *  \file game/map/planetstorage.hpp
  */
#ifndef C2NG_GAME_MAP_PLANETSTORAGE_HPP
#define C2NG_GAME_MAP_PLANETSTORAGE_HPP

#include "game/cargocontainer.hpp"
#include "game/interpreterinterface.hpp"
#include "game/config/hostconfiguration.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace map {

    class Planet;

    class PlanetStorage : public CargoContainer {
     public:
        PlanetStorage(Planet& pl,
                      InterpreterInterface& iface,
                      const game::config::HostConfiguration& config);
        ~PlanetStorage();

        virtual String_t getName(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();

     private:
        Planet& m_planet;
        InterpreterInterface& m_interface;
        const game::config::HostConfiguration& m_hostConfiguration;
        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
