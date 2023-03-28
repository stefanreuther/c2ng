/**
  *  \file game/interface/hullcontext.hpp
  *  \brief Class game::interface::HullContext
  */
#ifndef C2NG_GAME_INTERFACE_HULLCONTEXT_HPP
#define C2NG_GAME_INTERFACE_HULLCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Hull context.
        Implements the result of the Hull() function.
        To create, usually use HullContext::create().

        @see HullFunction */
    class HullContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param nr       Hull number
            @param shipList Ship list
            @param root     Root */
        HullContext(int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<const Root> root);

        /** Destructor. */
        ~HullContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual HullContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create HullContext.
            @param nr      Hull number
            @param session Session
            @return newly-allocated HullContext; null if preconditions are missing */
        static HullContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<const Root> m_root;
    };

} }

#endif
