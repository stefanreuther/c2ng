/**
  *  \file game/vcr/battle.hpp
  */
#ifndef C2NG_GAME_VCR_BATTLE_HPP
#define C2NG_GAME_VCR_BATTLE_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace vcr {

    class Object;

    /** Entry in a VCR database. */
    class Battle : public afl::base::Deletable {
     public:
        static const int NeedQuickOutcome = 1;   ///< Caller needs the result of getOutcome(x).
        static const int NeedCompleteResult = 2; ///< Caller needs the result of getObject(x, true).

        enum Playability {
            IsPlayable,                        ///< Fight is playable.
            IsNotSupported,                    ///< We cannot play it and know why.
            IsDamaged                          ///< We cannot play it and don't know why. Might be host error.
        };

        /** Get number of objects. */
        virtual size_t getNumObjects() const = 0;

        /** Get an object participating in the fight.
            If the result is requested but not yet known, needs not compute it; use prepareResult() to reliably obtain results.
            If the fight cannot be played, but after=true is requested, treat that as after=false.
            \param slot Slot, [0,getNumObjects())
            \param after false to return beginning of fight, true to return after. */
        virtual Object* getObject(size_t slot, bool after) = 0;

        /** Get outcome for an object.
            Can be one of:
            - -1 = unit got destroyed
            - 0 = unit survived or fight not playable
            - positive = unit got captured by specified player
            \param slot Slot, [0,getNumObjects()) */
        virtual int getOutcome(const game::config::HostConfiguration& config,
                               const game::spec::ShipList& shipList,
                               size_t slot) = 0;

        /** Check whether this fight is playable.
            Should operate quickly. */
        virtual Playability getPlayability(const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList) = 0;

        /** Prepare the result.
            Compute this fight's result.
            If the result is already computed, just return.
            \param resultLevel Requested result level (NeedQuickOutcome, NeedCompleteResult, or combination thereof).
            \param pm Progress monitor */
        virtual void prepareResult(const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList,
                                   int resultLevel) = 0;

        /** Get name of algorithm used to play this fight.
            \param tx Translator. Pass user translator to obtain localized name, NullTranslator to obtain raw name. */
        virtual String_t getAlgorithmName(afl::string::Translator& tx) const = 0;

        /** Check whether Engine/Shield Bonus is active in this fight. */
        virtual bool isESBActive(const game::config::HostConfiguration& config) const = 0;
    };

} }

#endif
