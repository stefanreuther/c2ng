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
        Users will do "With Lock(...)", causing an IntMutexContext be created.
        As long as this context lives, the mutex will be held.
        Since the interpreter may copy around the object, we must use reference-counting. */
    class MutexContext : public Context {
     public:
        /** Constructor.
            \param mtx Mutex. Must have one reference allocated to this object. */
        MutexContext(MutexList::Mutex* mtx);
        ~MutexContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual bool next();
        virtual MutexContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

     private:
        MutexList::Mutex* m_mutex;
    };

}

#endif
