/**
  *  \file game/interface/engineproperty.cpp
  *  \brief Engine Properties
  */

#include "game/interface/engineproperty.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "interpreter/indexablevalue.hpp"

using interpreter::makeIntegerValue;
using interpreter::checkIntegerArg;

namespace {
    class EngineFuelFactor : public interpreter::IndexableValue {
     public:
        EngineFuelFactor(const game::spec::Engine& e)
            : m_engine(e)
            { }

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                int32_t warp;
                args.checkArgumentCount(1);
                if (!checkIntegerArg(warp, args.getNext(), 0, m_engine.MAX_WARP)) {
                    return 0;
                }

                int32_t ff;
                if (m_engine.getFuelFactor(warp, ff)) {
                    return makeIntegerValue(ff);
                } else {
                    return 0;
                }
            }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            { rejectSet(args, value); }

        // CallableValue:
        virtual size_t getDimension(size_t which) const
            { return which==0 ? 1 : m_engine.MAX_WARP+1; }
        virtual interpreter::Context* makeFirstContext()
            { return rejectFirstContext(); }
        virtual EngineFuelFactor* clone() const
            { return new EngineFuelFactor(m_engine); }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<array>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

     private:
        /** Subject engine.
            We store a copy of the engine.
            This works as long as we only read, and isn't too inefficient.
            If we'd like to store a modifyable reference, we'd have to make sure to keep it alive,
            by storing a smart-pointer to the containing ship list. */
        game::spec::Engine m_engine;
    };
}

afl::data::Value*
game::interface::getEngineProperty(const game::spec::Engine& e, EngineProperty iep)
{
    // ex int/if/specif.h:getEngineProperty
    switch (iep) {
     case iepEfficientWarp:
        /* @q Speed$:Int (Engine Property)
           Nominal speed of this engine.
           This is the speed PCC considers "optimal" for this engine.
           It defaults to the lowest speed at which the engine runs at 120% fuel consumption or less.
           You can assign a value between 1 and 9 to this property to change what PCC considers optimal.
           @since PCC 1.1.15, PCC2 1.99.8 */
        return makeIntegerValue(e.getMaxEfficientWarp());
     case iepFuelFactor:
        /* @q FuelFactor:Int() (Engine Property)
           Array of fuel factors for warp factors from 0 to 9.
           This value is used in the computation of fuel usage. */
        return new EngineFuelFactor(e);
    }
    return 0;
}

void
game::interface::setEngineProperty(game::spec::Engine& e,
                                   EngineProperty iep,
                                   const afl::data::Value* value,
                                   game::spec::ShipList& list)
{
    // ex int/if/specif.h:setEngineProperty
    int32_t val;
    switch (iep) {
     case iepEfficientWarp:
        if (checkIntegerArg(val, value, 0, e.MAX_WARP)) {
            e.setMaxEfficientWarp(val);
            list.sig_change.raise();
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
