/**
  *  \file game/interface/vcrsidecontext.hpp
  *  \brief Class game::interface::VcrSideContext
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP
#define C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/vcr/battle.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Properties of a VCR side.
        Implements the result of the "Vcr().Unit()" function.

        @see VcrContext, VcrSideFunction */
    class VcrSideContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
            @param side           Side, index into game::vcr::Battle::getObject()
            @param tx             Translator
            @param root           Root (for players, config)
            @param battles        Battles
            @param shipList       Ship list (for component names, battle outcome) */
        VcrSideContext(size_t battleNumber,
                       size_t side,
                       afl::string::Translator& tx,
                       const afl::base::Ref<const Root>& root,
                       const afl::base::Ptr<game::vcr::Database>& battles,
                       const afl::base::Ref<const game::spec::ShipList>& shipList);

        /** Destructor. */
        ~VcrSideContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual VcrSideContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        game::vcr::Battle* getBattle() const;

     private:
        const size_t m_battleNumber;
        size_t m_side;
        afl::string::Translator& m_translator;
        afl::base::Ref<const Root> m_root;
        afl::base::Ptr<game::vcr::Database> m_battles;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
