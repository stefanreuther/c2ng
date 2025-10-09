/**
  *  \file game/proxy/simulationadaptor.hpp
  *  \brief Interface game::proxy::SimulationAdaptor
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONADAPTOR_HPP
#define C2NG_GAME_PROXY_SIMULATIONADAPTOR_HPP

#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/root.hpp"
#include "game/sim/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/object.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace proxy {

    /** Adaptor to access a simulator session and surroundings. */
    class SimulationAdaptor {
     public:
        /** Virtual destructor. */
        virtual ~SimulationAdaptor()
            { }

        /** Access simulator session.
            @return session */
        virtual game::sim::Session& simSession() = 0;

        /** Access game root.
            Required for host configuration, preferences, host type.
            User must deal with null return value.
            @return root */
        virtual afl::base::Ptr<const Root> getRoot() const = 0;

        /** Access ship list.
            Required for component names.
            User must deal with null return value.
            @return ship list */
        virtual afl::base::Ptr<const game::spec::ShipList> getShipList() const = 0;

        /** Access team settings.
            Required to determine player relations.
            Return null if you don't have one.
            @return TeamSettings, or null */
        virtual const TeamSettings* getTeamSettings() const = 0;

        /** Access translator.
            @return translator */
        virtual afl::string::Translator& translator() = 0;

        /** Access logger.
            @return logger */
        virtual afl::sys::LogListener& log() = 0;

        /** Access file system.
            Note that the file system is not required for simulation as is, but will be needed for export.
            @return file system */
        virtual afl::io::FileSystem& fileSystem() = 0;

        /** Access random number generator.
            @return random number generator */
        virtual util::RandomNumberGenerator& rng() = 0;

        /** Check for presence of a VCR object in game.
            @param obj VCR object
            @return true if object corresponds to a game unit */
        virtual bool isGameObject(const game::vcr::Object& obj) const = 0;

        /** Get number of processors.
            @return number of processors */
        virtual size_t getNumProcessors() const = 0;
    };

} }

#endif
