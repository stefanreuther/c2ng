/**
  *  \file game/interface/torpedocontext.hpp
  *  \brief Class game::interface::TorpedoContext
  */
#ifndef C2NG_GAME_INTERFACE_TORPEDOCONTEXT_HPP
#define C2NG_GAME_INTERFACE_TORPEDOCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Torpedo context.
        Implements the result of the Torpedo() and Launcher() function.
        To create, usually use TorpedoContext::create().

        @see TorpedoFunction */
    class TorpedoContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param useLauncher true to publish launcher properties, false for torpedo properties
            @param nr          Torpedo number
            @param shipList    Ship list
            @param root        Root (for host version/configuration) */
        TorpedoContext(bool useLauncher, int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<const Root> root);

        /** Destructor. */
        ~TorpedoContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual TorpedoContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create TorpedoContext.
            @param useLauncher true to publish launcher properties, false for torpedo properties
            @param nr          Torpedo number
            @param session     Session
            @return newly-allocated TorpedoContext; null if preconditions are missing */
        static TorpedoContext* create(bool useLauncher, int nr, Session& session);

     private:
        const bool m_useLauncher;
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<const Root> m_root;

        afl::data::Value* getProperty(const game::spec::Weapon& w, PropertyIndex_t index);
    };

} }

#endif
