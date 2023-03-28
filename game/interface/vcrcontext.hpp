/**
  *  \file game/interface/vcrcontext.hpp
  *  \brief Class game::interface::VcrContext
  */
#ifndef C2NG_GAME_INTERFACE_VCRCONTEXT_HPP
#define C2NG_GAME_INTERFACE_VCRCONTEXT_HPP

#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** VCR context.
        Implements the result of the "Vcr()" function.
        Create using VcrContext::create().

        @see VcrFunction */
    class VcrContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        VcrContext(size_t battleNumber,
                   Session& session,
                   afl::base::Ref<const Root> root,
                   afl::base::Ref<const Turn> turn,
                   afl::base::Ref<const game::spec::ShipList> shipList);
        ~VcrContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual VcrContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create a VcrContext for the current turn.
            \param battleNumber Number of battle (0-based!)
            \param session Session */
        static VcrContext* create(size_t battleNumber, Session& session);

     private:
        game::vcr::Battle* getBattle() const;

        size_t m_battleNumber;
        Session& m_session;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<const Turn> m_turn;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
