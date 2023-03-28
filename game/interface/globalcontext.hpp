/**
  *  \file game/interface/globalcontext.hpp
  *  \brief Class game::interface::GlobalContext
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALCONTEXT_HPP
#define C2NG_GAME_INTERFACE_GLOBALCONTEXT_HPP

#include "game/session.hpp"
#include "interpreter/context.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Global context.
        Publishes
        - global variables (interpreter::World::globalPropertyNames() / interpreter::World::globalValues())
        - global properties (GlobalProperty)
        - user interface properties (UserInterfaceProperty / UserInterfacePropertyStack)
        - properties of viewpoint player (PlayerProperty)

        Compatibility with the external interface (VM file format) requires that all global properties are
        published by one context implementation (Tag_Global). */
    class GlobalContext : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param session Session */
        explicit GlobalContext(Session& session);

        /** Destructor. */
        ~GlobalContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual GlobalContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
