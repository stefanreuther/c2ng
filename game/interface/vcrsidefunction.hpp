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
#include "game/vcr/database.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Vcr().Unit()" function.
        Provides access to a VCR side's properties. */
    class VcrSideFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
            @param tx             Translator
            @param root           Root (for players, config)
            @param battles        Battles
            @param shipList       Ship list (for component names, battle outcome) */
        VcrSideFunction(size_t battleNumber,
                        afl::string::Translator& tx,
                        const afl::base::Ref<const Root>& root,
                        const afl::base::Ptr<game::vcr::Database>& battles,
                        const afl::base::Ref<const game::spec::ShipList>& shipList);

        // IndexableValue:
        virtual VcrSideContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual VcrSideContext* makeFirstContext();
        virtual VcrSideFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t getNumObjects() const;

        const size_t m_battleNumber;
        afl::string::Translator& m_translator;
        const afl::base::Ref<const Root> m_root;
        const afl::base::Ptr<game::vcr::Database> m_battles;
        const afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
