/**
  *  \file game/proxy/vcrdatabaseadaptor.hpp
  *  \brief Interface game::proxy::VcrDatabaseAdaptor
  */
#ifndef C2NG_GAME_PROXY_VCRDATABASEADAPTOR_HPP
#define C2NG_GAME_PROXY_VCRDATABASEADAPTOR_HPP

#include <cstddef>
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/root.hpp"
#include "game/sim/setup.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/database.hpp"
#include "game/vcr/object.hpp"

namespace game { namespace proxy {

    /** Adaptor to access a VCR database and environment.

        Used by VCR-related proxies.

        Note that if any method of this class throws, the proxy will proceed in degraded mode
        (=may stop answering requests). */
    class VcrDatabaseAdaptor {
     public:
        /** Virtual destructor. */
        virtual ~VcrDatabaseAdaptor() { }

        /** Access game root.
            Required for host configuration, preferences, host type.
            \return root */
        virtual const Root& root() const = 0;

        /** Access ship list.
            Required for component names.
            \return ship list */
        virtual const game::spec::ShipList& shipList() const = 0;

        /** Access team settings.
            Required to determine player relations.
            Return null if you don't have one.
            \return TeamSettings, or null */
        virtual const TeamSettings* getTeamSettings() const = 0;

        /** Access battles.
            \return battles */
        virtual game::vcr::Database& battles() = 0;

        /** Access translator.
            \return translator */
        virtual afl::string::Translator& translator() = 0;

        /** Access logger.
            \return logger */
        virtual afl::sys::LogListener& log() = 0;

        /** Get index of last viewed battle.
            If you don't persist that status, return 0.
            \return index */
        virtual size_t getCurrentBattle() const = 0;

        /** Set current battle index.
            Store the index that the next getCurrentBattle() (in a new Adaptor instance, maybe) can find it.
            If you don't persist that status, ignore the call.
            \param n Index */
        virtual void setCurrentBattle(size_t n) = 0;

        /** Get associated simulation setup.
            \return setup, or null */
        virtual game::sim::Setup* getSimulationSetup() const = 0;

        /** Check for presence of a VCR object in game.
            \param obj VCR object
            \return true if object corresponds to a game unit */
        virtual bool isGameObject(const game::vcr::Object& obj) const = 0;
    };

} }

#endif
