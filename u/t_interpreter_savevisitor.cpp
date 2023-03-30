/**
  *  \file u/t_interpreter_savevisitor.cpp
  *  \brief Test for interpreter::SaveVisitor
  */

#include <memory>
#include "interpreter/savevisitor.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/errorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/values.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

namespace {
    struct Environment {
        interpreter::TagNode tag;
        afl::io::InternalSink aux;
        afl::charset::CodepageCharset charset;
        interpreter::vmio::NullSaveContext ctx;

        interpreter::SaveVisitor visitor;
        Environment()
            : tag(), aux(), charset(afl::charset::g_codepage437), ctx(),
              visitor(tag, aux, charset, ctx)
            { }
    };
}

/** Test string. */
void
TestInterpreterSaveVisitor::testString()
{
    Environment env;
    // c3 84 = U+00C4 = LATIN CAPITAL LETTER A WITH UMLAUT = cp437 142 = 0x8E
    // e2 94 80 = U+2500 = BOX DRAWINGS LIGHT HORIZONTAL = cp437 196 = 0xC4
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("x\xc3\x84y\xe2\x94\x80z"));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_LongString);
    TS_ASSERT_EQUALS(env.tag.value, 5U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 5U);
    TS_ASSERT_EQUALS(afl::string::fromBytes(env.aux.getContent()), "x\x8ey\xc4z");
}


/** Test integer; positive value. */
void
TestInterpreterSaveVisitor::testInteger()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3000));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_Integer);
    TS_ASSERT_EQUALS(env.tag.value, 3000U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test integer; negative value. */
void
TestInterpreterSaveVisitor::testInteger2()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(-2));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_Integer);
    TS_ASSERT_EQUALS(env.tag.value, 0xFFFFFFFEU);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test float; zero. */
void
TestInterpreterSaveVisitor::testFloat()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(0.0));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_FPZero);
    TS_ASSERT_EQUALS(env.tag.value, 0U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test float, positive value. */
void
TestInterpreterSaveVisitor::testFloat2()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, 0x0081U);
    TS_ASSERT_EQUALS(env.tag.value, 0U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test float, negative value. */
void
TestInterpreterSaveVisitor::testFloat3()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(-1.0));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, 0x0081U);
    TS_ASSERT_EQUALS(env.tag.value, 0x80000000U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test float, infinite (huge) value. */
void
TestInterpreterSaveVisitor::testFloatInf()
{
    Environment env;
    // Detected as too large quite early
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0e+300));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, 0xFFFFU);
    TS_ASSERT_EQUALS(env.tag.value, 0x7FFFFFFFU);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test float, infinite (huge) value. */
void
TestInterpreterSaveVisitor::testFloatInf2()
{
    Environment env;
    // Max REAL is 1.7e+38, implementation's cutoff point is 1.0e+39,
    // so this should hit the case where we detect overflow during conversion
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(9.0e+38));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, 0xFFFFU);
    TS_ASSERT_EQUALS(env.tag.value, 0x7FFFFFFFU);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test boolean, false. */
void
TestInterpreterSaveVisitor::testBooleanFalse()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(false));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_Boolean);
    TS_ASSERT_EQUALS(env.tag.value, 0U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test boolean, true. */
void
TestInterpreterSaveVisitor::testBooleanTrue()
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(true));
    env.visitor.visit(p.get());
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_Boolean);
    TS_ASSERT_EQUALS(env.tag.value, 1U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test hash. Native afl::data hashes cannot be serialized. */
void
TestInterpreterSaveVisitor::testHash()
{
    Environment env;
    afl::data::HashValue hv(afl::data::Hash::create());
    TS_ASSERT_THROWS(env.visitor.visit(&hv), interpreter::Error);
}

/** Test vector. Native afl::data vectors cannot be serialized. */
void
TestInterpreterSaveVisitor::testVector()
{
    Environment env;
    afl::data::VectorValue vv(afl::data::Vector::create());
    TS_ASSERT_THROWS(env.visitor.visit(&vv), interpreter::Error);
}

/** Test serializing unknown types. */
void
TestInterpreterSaveVisitor::testOther()
{
    class OtherValue : public afl::data::Value {
     public:
        virtual void visit(afl::data::Visitor& v) const
            { v.visitOther(*this); }
        virtual afl::data::Value* clone() const
            { return new OtherValue(); }
    };
    Environment env;
    OtherValue ov;
    TS_ASSERT_THROWS(env.visitor.visit(&ov), interpreter::Error);
}

/** Test serializing BaseValue. */
void
TestInterpreterSaveVisitor::testOther2()
{
    class OtherBaseValue : public interpreter::BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "#<other>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { out.tag = 0x4444; out.value = 0x55555555; }
        virtual interpreter::BaseValue* clone() const
            { return new OtherBaseValue(); }
    };
    Environment env;
    OtherBaseValue ov;
    env.visitor.visit(&ov);
    TS_ASSERT_EQUALS(env.tag.tag, 0x4444U);
    TS_ASSERT_EQUALS(env.tag.value, 0x55555555U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test null. */
void
TestInterpreterSaveVisitor::testNull()
{
    Environment env;
    env.visitor.visit(0);
    TS_ASSERT_EQUALS(env.tag.tag, interpreter::TagNode::Tag_Empty);
    TS_ASSERT_EQUALS(env.tag.value, 0U);
    TS_ASSERT_EQUALS(env.aux.getContent().size(), 0U);
}

/** Test errors. */
void
TestInterpreterSaveVisitor::testError()
{
    Environment env;
    afl::data::ErrorValue ev("a", "b");
    TS_ASSERT_THROWS(env.visitor.visit(&ev), interpreter::Error);
}

/** Test saveNames(), count shorter than list. */
void
TestInterpreterSaveVisitor::testSaveNames()
{
    afl::io::InternalSink out;
    afl::data::NameMap map;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    map.add("A");
    map.add("BB");
    map.add("C");
    map.add("DDD");

    interpreter::SaveVisitor::saveNames(out, map, 3, cs);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()), "\1A\2BB\1C");
}

void
TestInterpreterSaveVisitor::testSaveNames2()
{
    afl::io::InternalSink out;
    afl::data::NameMap map;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    map.add("A");
    map.add("BB");
    map.add("C");
    map.add("DDD");

    interpreter::SaveVisitor::saveNames(out, map, 5, cs);

    TS_ASSERT_EQUALS(out.getContent().size(), 12U);
    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()), String_t("\1A\2BB\1C\3DDD\0", 12));
}

