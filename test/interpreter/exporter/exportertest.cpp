/**
  *  \file test/interpreter/exporter/exportertest.cpp
  *  \brief Test for interpreter::exporter::Exporter
  */

#include "interpreter/exporter/exporter.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/values.hpp"
#include <stdexcept>

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
            : Object(id)
            { }

        // Object:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return "obj"; }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
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
    class TestContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        TestContext(int id, game::map::ObjectVector<TestObject>& vec)
            : m_id(id),
              m_vector(vec)
            { }
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            { return lookupName(name, TEST_MAP, result) ? this : 0; }
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
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
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
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        int m_id;
        game::map::ObjectVector<TestObject>& m_vector;
   };
}

/** Interface test. */
AFL_TEST_NOARG("interpreter.exporter.Exporter:interface")
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
AFL_TEST("interpreter.exporter.Exporter:doExport", a)
{
    interpreter::exporter::FieldList fields;
    fields.addList("ID,A");

    game::map::ObjectVector<TestObject> vec;
    TestContext ctx(5, vec);

    TestExporter t;
    t.doExport(ctx, fields);

    a.checkEqual("result", t.getResult(),
                 "ID=5,A=1\n"
                 "ID=6,A=1\n"
                 "ID=7,A=1\n"
                 "ID=8,A=1\n"
                 "ID=9,A=1\n"
                 "ID=10,A=1\n");
}

/** Test doExport(), invalid fields. */
AFL_TEST("interpreter.exporter.Exporter:error", a)
{
    interpreter::exporter::FieldList fields;
    fields.addList("ID,NAME,A");

    game::map::ObjectVector<TestObject> vec;
    TestContext ctx(5, vec);

    TestExporter t;
    AFL_CHECK_THROWS(a, t.doExport(ctx, fields), std::exception);
}
