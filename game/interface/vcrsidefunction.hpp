/**
  *  \file game/interface/vcrsidefunction.hpp
  *  \brief Class game::interface::VcrSideFunction
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_VCRSIDEFUNCTION_HPP

#include "game/interface/vcrsidecontext.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Vcr().Unit()" function.
        Provides access to a VCR side's properties. */
    class VcrSideFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
            @param session        Session (for translator)
            @param root           Root (for players)
            @param turn           Turn (for battles)
            @param shipList       Ship list (for unit names) */
        VcrSideFunction(size_t battleNumber,
                        Session& session,
                        afl::base::Ref<const Root> root,
                        afl::base::Ref<const Turn> turn,
                        afl::base::Ref<const game::spec::ShipList> shipList);

        // IndexableValue:
        virtual VcrSideContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual VcrSideContext* makeFirstContext();
        virtual VcrSideFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        int32_t getNumObjects() const;

        const size_t m_battleNumber;
        Session& m_session;
        const afl::base::Ref<const Root> m_root;
        const afl::base::Ref<const Turn> m_turn;
        const afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
