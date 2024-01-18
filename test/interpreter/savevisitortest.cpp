/**
  *  \file test/interpreter/savevisitortest.cpp
  *  \brief Test for interpreter::SaveVisitor
  */

#include "interpreter/savevisitor.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/errorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/values.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include <memory>

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
AFL_TEST("interpreter.SaveVisitor:string", a)
{
    Environment env;
    // c3 84 = U+00C4 = LATIN CAPITAL LETTER A WITH UMLAUT = cp437 142 = 0x8E
    // e2 94 80 = U+2500 = BOX DRAWINGS LIGHT HORIZONTAL = cp437 196 = 0xC4
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("x\xc3\x84y\xe2\x94\x80z"));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_LongString);
    a.checkEqual("02. value", env.tag.value, 5U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 5U);
    a.checkEqual("04. aux",   afl::string::fromBytes(env.aux.getContent()), "x\x8ey\xc4z");
}

/** Test integer; positive value. */
AFL_TEST("interpreter.SaveVisitor:int:positive", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3000));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_Integer);
    a.checkEqual("02. value", env.tag.value, 3000U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test integer; negative value. */
AFL_TEST("interpreter.SaveVisitor:int:negative", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(-2));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_Integer);
    a.checkEqual("02. value", env.tag.value, 0xFFFFFFFEU);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test float; zero. */
AFL_TEST("interpreter.SaveVisitor:float:zero", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(0.0));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_FPZero);
    a.checkEqual("02. value", env.tag.value, 0U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test float, positive value. */
AFL_TEST("interpreter.SaveVisitor:float:positive", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, 0x0081U);
    a.checkEqual("02. value", env.tag.value, 0U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test float, negative value. */
AFL_TEST("interpreter.SaveVisitor:float:negative", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(-1.0));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, 0x0081U);
    a.checkEqual("02. value", env.tag.value, 0x80000000U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test float, infinite (huge) value. */
AFL_TEST("interpreter.SaveVisitor:float:inf", a)
{
    Environment env;
    // Detected as too large quite early
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0e+300));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, 0xFFFFU);
    a.checkEqual("02. value", env.tag.value, 0x7FFFFFFFU);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test float, infinite (huge) value. */
AFL_TEST("interpreter.SaveVisitor:float:huge", a)
{
    Environment env;
    // Max REAL is 1.7e+38, implementation's cutoff point is 1.0e+39,
    // so this should hit the case where we detect overflow during conversion
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(9.0e+38));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, 0xFFFFU);
    a.checkEqual("02. value", env.tag.value, 0x7FFFFFFFU);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test boolean, false. */
AFL_TEST("interpreter.SaveVisitor:bool:false", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(false));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_Boolean);
    a.checkEqual("02. value", env.tag.value, 0U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test boolean, true. */
AFL_TEST("interpreter.SaveVisitor:bool:true", a)
{
    Environment env;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(true));
    env.visitor.visit(p.get());
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_Boolean);
    a.checkEqual("02. value", env.tag.value, 1U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test hash. Native afl::data hashes cannot be serialized. */
AFL_TEST("interpreter.SaveVisitor:native-hash", a)
{
    Environment env;
    afl::data::HashValue hv(afl::data::Hash::create());
    AFL_CHECK_THROWS(a, env.visitor.visit(&hv), interpreter::Error);
}

/** Test vector. Native afl::data vectors cannot be serialized. */
AFL_TEST("interpreter.SaveVisitor:native-vector", a)
{
    Environment env;
    afl::data::VectorValue vv(afl::data::Vector::create());
    AFL_CHECK_THROWS(a, env.visitor.visit(&vv), interpreter::Error);
}

/** Test serializing unknown types. */
AFL_TEST("interpreter.SaveVisitor:other", a)
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
    AFL_CHECK_THROWS(a, env.visitor.visit(&ov), interpreter::Error);
}

/** Test serializing BaseValue. */
AFL_TEST("interpreter.SaveVisitor:BaseValue", a)
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
    a.checkEqual("01. tag",   env.tag.tag, 0x4444U);
    a.checkEqual("02. value", env.tag.value, 0x55555555U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test null. */
AFL_TEST("interpreter.SaveVisitor:null", a)
{
    Environment env;
    env.visitor.visit(0);
    a.checkEqual("01. tag",   env.tag.tag, interpreter::TagNode::Tag_Empty);
    a.checkEqual("02. value", env.tag.value, 0U);
    a.checkEqual("03. aux",   env.aux.getContent().size(), 0U);
}

/** Test errors. */
AFL_TEST("interpreter.SaveVisitor:native-error", a)
{
    Environment env;
    afl::data::ErrorValue ev("a", "b");
    AFL_CHECK_THROWS(a, env.visitor.visit(&ev), interpreter::Error);
}

/** Test saveNames(), count shorter than list. */
AFL_TEST("interpreter.SaveVisitor:saveNames", a)
{
    afl::io::InternalSink out;
    afl::data::NameMap map;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    map.add("A");
    map.add("BB");
    map.add("C");
    map.add("DDD");

    interpreter::SaveVisitor::saveNames(out, map, 3, cs);

    a.checkEqual("01. content", afl::string::fromBytes(out.getContent()), "\1A\2BB\1C");
}

AFL_TEST("interpreter.SaveVisitor:saveNames:extra", a)
{
    afl::io::InternalSink out;
    afl::data::NameMap map;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    map.add("A");
    map.add("BB");
    map.add("C");
    map.add("DDD");

    interpreter::SaveVisitor::saveNames(out, map, 5, cs);

    a.checkEqual("01", out.getContent().size(), 12U);
    a.checkEqual("02", afl::string::fromBytes(out.getContent()), String_t("\1A\2BB\1C\3DDD\0", 12));
}
