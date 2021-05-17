/**
  *  \file game/vcr/flak/battle.hpp
  *  \brief Class game::vcr::flak::Battle
  */
#ifndef C2NG_GAME_VCR_FLAK_BATTLE_HPP
#define C2NG_GAME_VCR_FLAK_BATTLE_HPP

#include <memory>
#include "afl/container/ptrvector.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/flak/object.hpp"
#include "game/vcr/flak/setup.hpp"

namespace game { namespace vcr { namespace flak {

    /** FLAK Battle.
        Implements the c2ng battle abstraction for FLAK.
        This stores a Setup and will, on demand, play that using the Algorithm. */
    class Battle : public game::vcr::Battle {
     public:
        /** Constructor.
            \param setup Setup, must not be null */
        Battle(std::auto_ptr<Setup> setup);

        // Interface methods:
        virtual size_t getNumObjects() const;
        virtual const Object* getObject(size_t slot, bool after) const;
        virtual int getOutcome(const game::config::HostConfiguration& config,
                               const game::spec::ShipList& shipList,
                               size_t slot);
        virtual Playability getPlayability(const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList);
        virtual void prepareResult(const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList,
                                   int resultLevel);
        virtual String_t getAlgorithmName(afl::string::Translator& tx) const;
        virtual bool isESBActive(const game::config::HostConfiguration& config) const;
        virtual bool getPosition(game::map::Point& result) const;
        virtual afl::base::Optional<int32_t> getAuxiliaryInformation(AuxInfo info) const;
        virtual String_t getResultSummary(int viewpointPlayer,
                                          const game::config::HostConfiguration& config, const game::spec::ShipList& shipList,
                                          util::NumberFormatter fmt, afl::string::Translator& tx) const;

        const Setup& setup() const;

     private:
        std::auto_ptr<Setup> m_setup;

        afl::container::PtrVector<Object> m_after;
        bool m_haveAfter;
    };

} } }

inline const game::vcr::flak::Setup&
game::vcr::flak::Battle::setup() const
{
    return *m_setup;
}

#endif
