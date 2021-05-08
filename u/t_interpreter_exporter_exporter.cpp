/**
  *  \file u/t_interpreter_exporter_exporter.cpp
  *  \brief Test for interpreter::exporter::Exporter
  */

#include <stdexcept>
#include "interpreter/exporter/exporter.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/data/integervalue.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"
#include "util/answerprovider.hpp"
#include "util/constantanswerprovider.hpp"

namespace {
    /** Test implementation of Exporter. Concatenates all values as a text. */
    class TestExporter : public interpreter::exporter::Exporter {
     public:
        virtual void startTable(const interpreter::exporter::FieldList& /*fields*/, afl::base::Memory<const interpreter::TypeHint> /*types*/)
            { }
        virtual void startRecord()
            { }
        virtual void addField(afl::data::Value* value, const String_t& name, interpreter::TypeHint /*type*/)
            {
                m_result += name;
                m_result += "=";
                m_result += interpreter::toString(value, true);
                m_result += ",";
            }
        virtual void endRecord()
            {
                if (!m_result.empty() && m_result[m_result.size()-1] == ',') {
                    m_result.erase(m_result.size()-1);
                }
                m_result += "\n";
            }
        virtual void endTable()
            { }

        const String_t& getResult() const
            { return m_result; }
     private:
        String_t m_result;
    };

    /** Test implementation of map::Object. Just the minimum to get an object with Id. */
    class TestObject : public game::map::Object {
     public:
        TestObject(int id)
            : m_id(id)
            { }

        // Object:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return "obj"; }
        virtual game::Id_t getId() const
            { return m_id; }
        virtual bool getOwner(int& result) const
            { result = 0; return true; }
        virtual bool getPosition(game::map::Point& /*result*/) const
            { return false; }
     private:
        const int m_id;
    };

    const interpreter::NameTable TEST_MAP[] = {
        { "A",  1, 0, interpreter::thInt },
        { "B",  2, 0, interpreter::thInt },
        { "C",  3, 0, interpreter::thInt },
        { "D",  4, 0, interpreter::thInt },
        { "ID", 0, 0, interpreter::thInt },
    };

    /** Test implementation of Context.
        - provides object Ids up to 10
        - provides 4 properties A..D with values 1..4
        - references an ObjectVector and can provide objects from that */
    class TestContext : public interpreter::Context {
     public:
        TestContext(int id, game::map::ObjectVector<TestObject>& vec)
            : m_id(id),
              m_vector(vec)
            { }
        virtual TestContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            { return lookupName(name, TEST_MAP, result) ? this : 0; }
        virtual void set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
            { throw interpreter::Error::notAssignable(); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                int32_t x = static_cast<int32_t>(TEST_MAP[index].index);
                if (x == 0) {
                    return new afl::data::IntegerValue(m_id);
                } else {
                    return new afl::data::IntegerValue(x);
                }
            }
        virtual TestContext* clone() const
            { return new TestContext(m_id, m_vector); }
        virtual game::map::Object* getObject()
            { return m_vector.get(m_id); }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor)
            { acceptor.enumTable(TEST_MAP); }
        virtual bool next()
            {
                if (m_id < 10) {
                    ++m_id;
                    return true;
                } else {
                    return false;
                }
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "<tc>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw interpreter::Error::notSerializable(); }
     private:
        int m_id;
        game::map::ObjectVector<TestObject>& m_vector;
   };
}

/** Interface test. */
void
TestInterpreterExporterExporter::testInterface()
{
    class Tester : public interpreter::exporter::Exporter {
     public:
        virtual void startTable(const interpreter::exporter::FieldList& /*fields*/, afl::base::Memory<const interpreter::TypeHint> /*types*/)
            { }
        virtual void startRecord()
            { }
        virtual void addField(afl::data::Value* /*value*/, const String_t& /*name*/, interpreter::TypeHint /*type*/)
            { }
        virtual void endRecord()
            { }
        virtual void endTable()
            { }
    };
    Tester t;
}

/** Test the doExport function, simple standard case. */
void
TestInterpreterExporterExporter::testIt()
{
    interpreter::exporter::FieldList fields;
    fields.addList("ID,A");

    game::map::ObjectVector<TestObject> vec;
    TestContext ctx(5, vec);

    TestExporter t;
    t.doExport(ctx, util::ConstantAnswerProvider::sayYes, fields);

    TS_ASSERT_EQUALS(t.getResult(),
                     "ID=5,A=1\n"
                     "ID=6,A=1\n"
                     "ID=7,A=1\n"
                     "ID=8,A=1\n"
                     "ID=9,A=1\n"
                     "ID=10,A=1\n");
}

/** Test doExport(), invalid fields. */
void
TestInterpreterExporterExporter::testError()
{
    interpreter::exporter::FieldList fields;
    fields.addList("ID,NAME,A");

    game::map::ObjectVector<TestObject> vec;
    TestContext ctx(5, vec);

    TestExporter t;
    TS_ASSERT_THROWS(t.doExport(ctx, util::ConstantAnswerProvider::sayYes, fields), std::exception);
}

/** Test doExport() with negative filter but no object. */
void
TestInterpreterExporterExporter::testNoObject()
{
    interpreter::exporter::FieldList fields;
    fields.addList("ID,A");

    game::map::ObjectVector<TestObject> vec;
    TestContext ctx(5, vec);

    // Export with "No" filter. However, because we don't have an object, that filter is not applied.
    TestExporter t;
    t.doExport(ctx, util::ConstantAnswerProvider::sayNo, fields);

    TS_ASSERT_EQUALS(t.getResult(),
                     "ID=5,A=1\n"
                     "ID=6,A=1\n"
                     "ID=7,A=1\n"
                     "ID=8,A=1\n"
                     "ID=9,A=1\n"
                     "ID=10,A=1\n");
}

/** Test doExport() with filter. */
void
TestInterpreterExporterExporter::testFilter()
{
    // A filter that accepts only odd objects
    class OddFilter : public util::AnswerProvider {
     public:
        virtual Result ask(int questionId, String_t /*question*/)
            { return (questionId & 1) != 0 ? Yes : No; }
    };
    OddFilter f;

    interpreter::exporter::FieldList fields;
    fields.addList("ID,A");

    // Populate vector with objects. Do not add #6.
    game::map::ObjectVector<TestObject> vec;
    for (int i = 1; i < 20; ++i) {
        if (i != 6) {
            vec.create(i);
        }
    }
    TestContext ctx(1, vec);

    // Export with filter. This will produce only odd objects.
    // However, the filter is not applied to #6.
    TestExporter t;
    t.doExport(ctx, f, fields);

    TS_ASSERT_EQUALS(t.getResult(),
                     "ID=1,A=1\n"
                     "ID=3,A=1\n"
                     "ID=5,A=1\n"
                     "ID=6,A=1\n"
                     "ID=7,A=1\n"
                     "ID=9,A=1\n");
}

/** Test doExport() with cancellation. */
void
TestInterpreterExporterExporter::testCancel()
{
    // A filter that cancels at #4.
    class CancelFilter : public util::AnswerProvider {
     public:
        virtual Result ask(int questionId, String_t /*question*/)
            { return questionId == 4 ? Cancel : Yes; }
    };
    CancelFilter f;

    // For a change, use some more fields.
    interpreter::exporter::FieldList fields;
    fields.addList("ID,A,C,B");

    // Populate vector with objects.
    game::map::ObjectVector<TestObject> vec;
    for (int i = 1; i < 20; ++i) {
        vec.create(i);
    }
    TestContext ctx(1, vec);

    // Export with filter. This will produce only odd objects.
    // However, the filter is not applied to #6.
    TestExporter t;
    t.doExport(ctx, f, fields);

    TS_ASSERT_EQUALS(t.getResult(),
                     "ID=1,A=1,C=3,B=2\n"
                     "ID=2,A=1,C=3,B=2\n"
                     "ID=3,A=1,C=3,B=2\n");
}

