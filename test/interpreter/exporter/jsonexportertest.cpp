/**
  *  \file test/interpreter/exporter/jsonexportertest.cpp
  *  \brief Test for interpreter::exporter::JsonExporter
  */

#include "interpreter/exporter/jsonexporter.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/indexablevalue.hpp"

using afl::data::IntegerValue;
using afl::data::StringValue;

namespace {
    String_t trimNewlines(String_t s)
    {
        size_t i = 0;
        while (i < s.size()) {
            if (s[i] == '\n') {
                s.erase(i, 1);
            } else {
                ++i;
            }
        }
        return s;
    }

    /* Sample implementation of IndexableValue.
       This returns a dimension of 5, meaning that it produces values 1..4 in output. */
    class MyIndexable : public interpreter::IndexableValue {
     public:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            { return afl::data::Value::cloneOf(args.getNext()); }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            { rejectSet(args, value); }
        virtual int32_t getDimension(int32_t which) const
            { return which == 0 ? 1 : 5; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual interpreter::CallableValue* clone() const
            { return new MyIndexable(); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<MyIndexable>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
    };
}

/** Simple test. */
AFL_TEST("interpreter.exporter.JsonExporter:basics", a)
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("left,right");

    // Output receiver
    afl::io::InternalStream outputStream;
    afl::io::TextFile outputText(outputStream);
    outputText.setSystemNewline(false);

    // Testee
    interpreter::exporter::JsonExporter testee(outputText);
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString };

    // Test sequence
    testee.startTable(list, hints);
    testee.startRecord();
    {
        IntegerValue iv(1);
        StringValue sv("a");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(2);
        StringValue sv("Say \"hi\"!");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(3);
        StringValue sv("\xC3\xBC""nic\xC3\xB6""de");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();
    testee.endTable();

    // Verify
    outputText.flush();

    a.checkEqual("result", trimNewlines(afl::string::fromBytes(outputStream.getContent())),
                 "[{\"left\":1,\"right\":\"a\"},"
                 "{\"left\":2,\"right\":\"Say \\\"hi\\\"!\"},"
                 "{\"left\":3,\"right\":\"\\""u00FCnic\\""u00F6de\"}]");
}

/** Test empty table. */
AFL_TEST("interpreter.exporter.JsonExporter:empty", a)
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("left,right");

    // Output receiver
    afl::io::InternalStream outputStream;
    afl::io::TextFile outputText(outputStream);
    outputText.setSystemNewline(false);

    // Testee
    interpreter::exporter::JsonExporter testee(outputText);
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString };

    // Test sequence
    testee.startTable(list, hints);
    testee.endTable();

    // Verify
    outputText.flush();
    a.checkEqual("result", trimNewlines(afl::string::fromBytes(outputStream.getContent())), "[]");
}

/** Test behaviour with a vector. */
AFL_TEST("interpreter.exporter.JsonExporter:array", a)
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("a,b");

    // Output receiver
    afl::io::InternalStream outputStream;
    afl::io::TextFile outputText(outputStream);
    outputText.setSystemNewline(false);

    // Testee
    interpreter::exporter::JsonExporter testee(outputText);
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thArray };

    // Test sequence
    testee.startTable(list, hints);
    testee.startRecord();
    {
        afl::base::Ref<interpreter::ArrayData> vec = *new interpreter::ArrayData();
        vec->addDimension(3);
        vec->content().pushBackNew(new IntegerValue(7));   // Index 0, NOT shown!
        vec->content().pushBackNew(new StringValue("s"));
        IntegerValue iv(42);
        interpreter::ArrayValue vv(vec);
        testee.addField(&iv, "a", interpreter::thInt);
        testee.addField(&vv, "b", interpreter::thArray);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(43);
        MyIndexable vv;
        testee.addField(&iv, "a", interpreter::thInt);
        testee.addField(&vv, "b", interpreter::thArray);
    }
    testee.endRecord();

    testee.endTable();

    // Verify
    outputText.flush();
    a.checkEqual("result", trimNewlines(afl::string::fromBytes(outputStream.getContent())),
                 "[{\"a\":42,\"b\":[\"s\",null]},"
                 "{\"a\":43,\"b\":[1,2,3,4]}]");
}
