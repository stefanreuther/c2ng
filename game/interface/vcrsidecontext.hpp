/**
  *  \file game/interface/vcrsidecontext.hpp
  *  \brief Class game::interface::VcrSideContext
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP
#define C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/vcr/battle.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Properties of a VCR side.
        Implements the result of the "Vcr().Unit()" function.
        Use VcrSideContext::create() to create.

        @see VcrContext, VcrSideFunction */
    class VcrSideContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
            @param side           Side, index into game::vcr::Battle::getObject()
            @param session        Session (for translator)
            @param root           Root (for players)
            @param turn           Turn (for battles)
            @param shipList       Ship list (for unit names) */
        VcrSideContext(size_t battleNumber,
                       size_t side,
                       Session& session,
                       afl::base::Ref<const Root> root,
                       afl::base::Ref<const Turn> turn,
                       afl::base::Ref<const game::spec::ShipList> shipList);

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

        /** Create a VcrSideContext for the current turn.
            @param battleNumber   Battle number, index into game::vcr::Database::getBattle()
            @param side           Side, index into game::vcr::Battle::getObject()
            @param session        Session
            @return newly-allocated VcrSideContext or null */
        static VcrSideContext* create(size_t battleNumber, size_t side, Session& session);

     private:
        const size_t m_battleNumber;
        size_t m_side;
        Session& m_session;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<const Turn> m_turn;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
