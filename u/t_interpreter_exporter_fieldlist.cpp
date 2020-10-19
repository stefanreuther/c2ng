/**
  *  \file u/t_interpreter_exporter_fieldlist.cpp
  *  \brief Test for interpreter::exporter::FieldList
  */

#include "interpreter/exporter/fieldlist.hpp"

#include "t_interpreter_exporter.hpp"
#include "interpreter/error.hpp"

/** Test add(). */
void
TestInterpreterExporterFieldList::testAdd()
{
    interpreter::exporter::FieldList t;
    TS_ASSERT_EQUALS(t.size(), 0U);

    t.add("foo");
    t.add("bar@10");
    t.add("qux @ 30");
    TS_ASSERT_EQUALS(t.size(), 3U);

    String_t name;
    int width;
    TS_ASSERT(t.getField(0, name, width));
    TS_ASSERT_EQUALS(name, "FOO");
    TS_ASSERT_EQUALS(width, 0);

    TS_ASSERT(t.getField(1, name, width));
    TS_ASSERT_EQUALS(name, "BAR");
    TS_ASSERT_EQUALS(width, 10);

    TS_ASSERT(t.getField(2, name, width));
    TS_ASSERT_EQUALS(name, "QUX");
    TS_ASSERT_EQUALS(width, 30);

    TS_ASSERT_EQUALS(t.getFieldName(2), "QUX"); // FIXME: function is pending-delete
    TS_ASSERT_EQUALS(t.getFieldWidth(2), 30);   // FIXME: function is pending-delete

    TS_ASSERT(!t.getField(3, name, width));
    TS_ASSERT(!t.getField(interpreter::exporter::FieldList::Index_t(-1), name, width));

    TS_ASSERT_THROWS(t.add(""), interpreter::Error);
    TS_ASSERT_THROWS(t.add("a@"), interpreter::Error);
    TS_ASSERT_THROWS(t.add("a@b"), interpreter::Error);
    TS_ASSERT_THROWS(t.add("@1"), interpreter::Error);
}

/** Test addFieldList(). */
void
TestInterpreterExporterFieldList::testAddList()
{
    interpreter::exporter::FieldList t;
    TS_ASSERT_EQUALS(t.size(), 0U);

    t.addList("foo,bar@10");
    TS_ASSERT_EQUALS(t.size(), 2U);

    String_t name;
    int width;
    TS_ASSERT(t.getField(0, name, width));
    TS_ASSERT_EQUALS(name, "FOO");
    TS_ASSERT_EQUALS(width, 0);

    TS_ASSERT(t.getField(1, name, width));
    TS_ASSERT_EQUALS(name, "BAR");
    TS_ASSERT_EQUALS(width, 10);

    TS_ASSERT(!t.getField(2, name, width));

    TS_ASSERT_THROWS(t.addList(""), interpreter::Error);
    TS_ASSERT_THROWS(t.addList("a,,b"), interpreter::Error);
    TS_ASSERT_THROWS(t.addList("a,@1,"), interpreter::Error);
    TS_ASSERT_THROWS(t.addList("a,@,"), interpreter::Error);
}

/** Test modification and toString(). */
void
TestInterpreterExporterFieldList::testModify()
{
    interpreter::exporter::FieldList t;
    TS_ASSERT_EQUALS(t.toString(), "");

    t.addList("a,b,c,d,e");
    TS_ASSERT_EQUALS(t.toString(), "A,B,C,D,E");

    t.swap(2, 3);
    TS_ASSERT_EQUALS(t.toString(), "A,B,D,C,E");

    t.swap(0, 0);
    TS_ASSERT_EQUALS(t.toString(), "A,B,D,C,E");

    t.swap(100, 100);
    TS_ASSERT_EQUALS(t.toString(), "A,B,D,C,E");

    t.remove(2);
    TS_ASSERT_EQUALS(t.toString(), "A,B,C,E");

    t.remove(0);
    TS_ASSERT_EQUALS(t.toString(), "B,C,E");

    t.remove(3);
    TS_ASSERT_EQUALS(t.toString(), "B,C,E");

    t.addList("x@5,y");
    TS_ASSERT_EQUALS(t.toString(), "B,C,E,X@5,Y");

    t.setFieldName(1, "D");
    t.setFieldWidth(2, 9);
    TS_ASSERT_EQUALS(t.toString(), "B,D,E@9,X@5,Y");

    t.setFieldName(3, "f");
    TS_ASSERT_EQUALS(t.toString(), "B,D,E@9,F@5,Y");
}

/** Test copying. */
void
TestInterpreterExporterFieldList::testCopy()
{
    // (I admit that this test only serves to fill an ugly red gap in the coverage report :)
    interpreter::exporter::FieldList a, b;
    a.addList("a,b@2,x");

    interpreter::exporter::FieldList c(a);
    b = a;

    TS_ASSERT_EQUALS(a.toString(), "A,B@2,X");
    TS_ASSERT_EQUALS(b.toString(), "A,B@2,X");
    TS_ASSERT_EQUALS(c.toString(), "A,B@2,X");
    TS_ASSERT_EQUALS(a.size(), 3U);
    TS_ASSERT_EQUALS(b.size(), 3U);
    TS_ASSERT_EQUALS(c.size(), 3U);
}

