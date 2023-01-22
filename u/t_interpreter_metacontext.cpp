/**
  *  \file u/t_interpreter_metacontext.cpp
  *  \brief Test for interpreter::MetaContext
  */

#include <memory>
#include "interpreter/metacontext.hpp"

#include "t_interpreter.hpp"
#include "afl/base/memory.hpp"
#include "afl/io/nullstream.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

namespace {
    class TestContext : public interpreter::Context {
     public:
        TestContext(afl::base::Memory<const interpreter::NameTable> tab)
            : m_table(tab)
            { }
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { throw interpreter::Error("unexpected"); }
        virtual bool next()
            { throw interpreter::Error("unexpected"); }
        virtual interpreter::Context* clone() const
            { throw interpreter::Error("unexpected"); }
        virtual afl::base::Deletable* getObject()
            { throw interpreter::Error("unexpected"); }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
            { acceptor.enumTable(m_table); }
        virtual void onContextEntered(interpreter::Process& /*proc*/)
            { throw interpreter::Error("unexpected"); }
        virtual void onContextLeft()
            { throw interpreter::Error("unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { throw interpreter::Error("unexpected"); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw interpreter::Error("unexpected"); }
     private:
        afl::base::Memory<const interpreter::NameTable> m_table;
    };
}

/** Test behaviour on empty context (no properties). */
void
TestInterpreterMetaContext::testEmpty()
{
    TestContext ctx(afl::base::Nothing);
    TS_ASSERT(interpreter::MetaContext::create(ctx) == 0);
}

/** Test behaviour on normal context. */
void
TestInterpreterMetaContext::testNormal()
{
    static const interpreter::NameTable tab[] = {
        { "IV",    0, 0, interpreter::thInt },
        { "ANYV",  0, 0, interpreter::thNone },
        { "BV",    0, 0, interpreter::thBool },
        { "FV",    0, 0, interpreter::thFloat },
        { "SV",    0, 0, interpreter::thString },
        { "PROCV", 0, 0, interpreter::thProcedure },
        { "FUNCV", 0, 0, interpreter::thFunction },
        { "AV",    0, 0, interpreter::thArray },
    };
    TestContext ctx(tab);

    std::auto_ptr<interpreter::MetaContext> t(interpreter::MetaContext::create(ctx));
    TS_ASSERT(t.get() != 0);

    // Verify clone
    std::auto_ptr<interpreter::MetaContext> clone(t->clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT(t.get() != clone.get());

    // Verify toString()
    TS_ASSERT_DIFFERS(t->toString(false), "");
    TS_ASSERT_EQUALS(t->toString(false), clone->toString(false));

    // Verify dummies
    TS_ASSERT(t->getObject() == 0);
    interpreter::TagNode tag;
    afl::io::NullStream sink;
    interpreter::vmio::NullSaveContext saveContext;
    TS_ASSERT_THROWS(t->store(tag, sink, saveContext), interpreter::Error);

    // Verify attributes
    interpreter::test::ContextVerifier verif(*t, "testNormal");
    verif.verifyTypes();
    verif.verifyString("NAME", "IV");
    verif.verifyString("TYPE", "int");
    verif.verifyInteger("ID", 0);

    // Verify iteration and other types (coverage)
    TS_ASSERT(t->next());
    verif.verifyString("NAME", "ANYV");
    verif.verifyString("TYPE", "any");
    verif.verifyInteger("ID", 1);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "BV");
    verif.verifyString("TYPE", "bool");
    verif.verifyInteger("ID", 2);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "FV");
    verif.verifyString("TYPE", "float");
    verif.verifyInteger("ID", 3);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "SV");
    verif.verifyString("TYPE", "string");
    verif.verifyInteger("ID", 4);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "PROCV");
    verif.verifyString("TYPE", "procedure");
    verif.verifyInteger("ID", 5);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "FUNCV");
    verif.verifyString("TYPE", "function");
    verif.verifyInteger("ID", 6);

    TS_ASSERT(t->next());
    verif.verifyString("NAME", "AV");
    verif.verifyString("TYPE", "array");
    verif.verifyInteger("ID", 7);

    TS_ASSERT(!t->next());
}

