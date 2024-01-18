/**
  *  \file test/interpreter/metacontexttest.cpp
  *  \brief Test for interpreter::MetaContext
  */

#include "interpreter/metacontext.hpp"

#include "afl/base/memory.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/contextverifier.hpp"
#include <memory>

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
AFL_TEST("interpreter.MetaContext:empty", a)
{
    TestContext ctx(afl::base::Nothing);
    a.checkNull("01. create", interpreter::MetaContext::create(ctx));
}

/** Test behaviour on normal context. */
AFL_TEST("interpreter.MetaContext:normal", a)
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
    a.checkNonNull("01. get", t.get());

    // Verify basics
    interpreter::test::ContextVerifier verif(*t, a);
    verif.verifyNotSerializable();
    verif.verifyBasics();
    a.checkNull("11. getObject", t->getObject());

    // Verify attributes
    verif.verifyTypes();
    verif.verifyString("NAME", "IV");
    verif.verifyString("TYPE", "int");
    verif.verifyInteger("ID", 0);

    // Verify iteration and other types (coverage)
    a.check("21. next", t->next());
    verif.verifyString("NAME", "ANYV");
    verif.verifyString("TYPE", "any");
    verif.verifyInteger("ID", 1);

    a.check("31. next", t->next());
    verif.verifyString("NAME", "BV");
    verif.verifyString("TYPE", "bool");
    verif.verifyInteger("ID", 2);

    a.check("41. next", t->next());
    verif.verifyString("NAME", "FV");
    verif.verifyString("TYPE", "float");
    verif.verifyInteger("ID", 3);

    a.check("51. next", t->next());
    verif.verifyString("NAME", "SV");
    verif.verifyString("TYPE", "string");
    verif.verifyInteger("ID", 4);

    a.check("61. next", t->next());
    verif.verifyString("NAME", "PROCV");
    verif.verifyString("TYPE", "procedure");
    verif.verifyInteger("ID", 5);

    a.check("71. next", t->next());
    verif.verifyString("NAME", "FUNCV");
    verif.verifyString("TYPE", "function");
    verif.verifyInteger("ID", 6);

    a.check("81. next", t->next());
    verif.verifyString("NAME", "AV");
    verif.verifyString("TYPE", "array");
    verif.verifyInteger("ID", 7);

    a.check("91. next", !t->next());
}
