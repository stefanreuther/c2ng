/**
  *  \file game/map/planetstorage.hpp
  *  \brief Class game::map::PlanetStorage
  */
#ifndef C2NG_GAME_MAP_PLANETSTORAGE_HPP
#define C2NG_GAME_MAP_PLANETSTORAGE_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/interpreterinterface.hpp"

namespace game { namespace map {

    class Planet;

    /** Planet cargo transfer.
        Implements CargoContainer for a played planet.
        Cargo can be transferred to/from mined minerals, and starbase ammo storage if present. */
    class PlanetStorage : public CargoContainer {
     public:
        /** Constructor.
            @param pl     Planet (must live longer than PlanetStorage)
            @param config Host configuration (for MaximumFightersOnBase, must live longer than PlanetStorage) */
        PlanetStorage(Planet& pl, const game::config::HostConfiguration& config);

        /** Destructor. */
        ~PlanetStorage();

        // CargoContainer:
        virtual String_t getName(afl::string::Translator& tx) const;
        virtual String_t getInfo1(afl::string::Translator& tx) const;
        virtual String_t getInfo2(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();

     private:
        Planet& m_planet;
        const game::config::HostConfiguration& m_hostConfiguration;
        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
