/**
  *  \file game/interface/globalactioncontext.hpp
  *  \brief Class game::interface::GlobalActionContext
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALACTIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_GLOBALACTIONCONTEXT_HPP

#include "afl/base/refcounted.hpp"
#include "game/interface/globalactions.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"
#include "util/treelist.hpp"

namespace game { namespace interface {

    /** Global Action Context.
        Represents the definitions of a set of Global Actions.
        Used from a script as "Dim g As GlobalActionContext"; see IFGlobalActionContext.

        Each newly-created GlobalActionContext maintains a separate set of actions.
        Cloning it will produce handles to the same set. */
    class GlobalActionContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Data. */
        struct Data : public afl::base::RefCounted {
            GlobalActions actions;                         ///< Action definitions.
            util::TreeList actionNames;                    ///< Names of the actions.
        };

        /** Make new GlobalActionContext. */
        GlobalActionContext();

        /** Destructor. */
        ~GlobalActionContext();

        /** Access underlying data.
            @return data */
        const afl::base::Ref<Data>& data() const
            { return m_data; }

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual GlobalActionContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        afl::base::Ref<Data> m_data;
    };

    /** Implementation of "GlobalActionContext()" function.
        For use with SimpleFunction<void>.
        @param args Script-provided arguments */
    afl::data::Value* IFGlobalActionContext(interpreter::Arguments& args);

} }

#endif
