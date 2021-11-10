/**
  *  \file game/interface/referencelistcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_REFERENCELISTCONTEXT_HPP
#define C2NG_GAME_INTERFACE_REFERENCELISTCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/ref/list.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Reference list context: publish properties of a game::ref::List. */
    class ReferenceListContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        struct Data : public afl::base::RefCounted {
            game::ref::List list;
        };

        /** Constructor.
            \param list List to publish
            \param session Game session */
        ReferenceListContext(afl::base::Ref<Data> list, Session& session);

        /** Destructor. */
        ~ReferenceListContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual ReferenceListContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        const game::ref::List& getList() const;

     private:
        class ProcedureValue;
        class IterableReferenceContext;
        class ObjectArrayValue;

        afl::base::Ref<Data> m_list;
        Session& m_session;
    };

    void IFReferenceList_Add(game::ref::List& list, Session& session, interpreter::Arguments& args);
    void IFReferenceList_AddObjects(game::ref::List& list, Session& session, interpreter::Arguments& args);
    void IFReferenceList_AddObjectsAt(game::ref::List& list, Session& session, interpreter::Arguments& args);

    afl::data::Value* IFReferenceList(game::Session& session, interpreter::Arguments& args);

} }

#endif
