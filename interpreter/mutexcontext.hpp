/**
  *  \file interpreter/mutexcontext.hpp
  *  \brief Class interpreter::MutexContext
  */
#ifndef C2NG_INTERPRETER_MUTEXCONTEXT_HPP
#define C2NG_INTERPRETER_MUTEXCONTEXT_HPP

#include "interpreter/context.hpp"
#include "interpreter/mutexlist.hpp"

namespace interpreter {

    /** Mutex context.
        This is the main primitive exposed to the script interface.
        Users will do "With Lock(...)", causing n MutexContext be created and be pushed to the context stack.

        As of 20220801, a mutex is owned as long as it is on a context stack;
        previously, a mutex was owned as long as a MutexContext object existed somwhere.
        Mutex objects are rarely copied (not at all if the only recommended syntax, "With Lock(...)" is used),
        so copying needn't be absolutely cheap. */
    class MutexContext : public Context {
     public:
        /** Constructor.
            \param name Mutex name (by convention, in upper-case)
            \param note Note associated with it */
        MutexContext(const String_t& name, const String_t& note);
        ~MutexContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual MutexContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor);
        virtual void onContextEntered(Process& proc);
        virtual void onContextLeft();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

     private:
        MutexList::Mutex* m_mutex;
        String_t m_name;
        String_t m_note;
    };

}

#endif
