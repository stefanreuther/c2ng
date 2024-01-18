/**
  *  \file test/interpreter/exporter/fieldlisttest.cpp
  *  \brief Test for interpreter::exporter::FieldList
  */

#include "interpreter/exporter/fieldlist.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

/** Test add(). */
AFL_TEST("interpreter.exporter.FieldList:add", a)
{
    interpreter::exporter::FieldList t;
    a.checkEqual("01. size", t.size(), 0U);

    t.add("foo");
    t.add("bar@10");
    t.add("qux @ 30");
    a.checkEqual("11. size", t.size(), 3U);

    String_t name;
    int width;
    a.check("21. getField", t.getField(0, name, width));
    a.checkEqual("22. name",  name, "FOO");
    a.checkEqual("23. width", width, 0);

    a.check("31. getField", t.getField(1, name, width));
    a.checkEqual("32. name",  name, "BAR");
    a.checkEqual("33. width", width, 10);

    a.check("41. getField", t.getField(2, name, width));
    a.checkEqual("42. name",  name, "QUX");
    a.checkEqual("43. width", width, 30);

    a.checkEqual("51. getFieldName",  t.getFieldName(2), "QUX"); // FIXME: function is pending-delete
    a.checkEqual("52. getFieldWidth", t.getFieldWidth(2), 30);   // FIXME: function is pending-delete

    a.check("61. getField", !t.getField(3, name, width));
    a.check("62. getField", !t.getField(interpreter::exporter::FieldList::Index_t(-1), name, width));

    AFL_CHECK_THROWS(a("71. add"), t.add(""), interpreter::Error);
    AFL_CHECK_THROWS(a("72. add"), t.add("a@"), interpreter::Error);
    AFL_CHECK_THROWS(a("73. add"), t.add("a@b"), interpreter::Error);
    AFL_CHECK_THROWS(a("74. add"), t.add("@1"), interpreter::Error);
}

/** Test addFieldList(). */
AFL_TEST("interpreter.exporter.FieldList:addList", a)
{
    interpreter::exporter::FieldList t;
    a.checkEqual("01. size", t.size(), 0U);

    t.addList("foo,bar@10");
    a.checkEqual("11. size", t.size(), 2U);

    String_t name;
    int width;
    a.check("21. getField", t.getField(0, name, width));
    a.checkEqual("22. name",  name, "FOO");
    a.checkEqual("23. width", width, 0);

    a.check("31. getField", t.getField(1, name, width));
    a.checkEqual("32. name",  name, "BAR");
    a.checkEqual("33. width", width, 10);

    a.check("41. getField", !t.getField(2, name, width));

    AFL_CHECK_THROWS(a("51. addList"), t.addList(""), interpreter::Error);
    AFL_CHECK_THROWS(a("52. addList"), t.addList("a,,b"), interpreter::Error);
    AFL_CHECK_THROWS(a("53. addList"), t.addList("a,@1,"), interpreter::Error);
    AFL_CHECK_THROWS(a("54. addList"), t.addList("a,@,"), interpreter::Error);
}

/** Test modification and toString(). */
AFL_TEST("interpreter.exporter.FieldList:basics", a)
{
    interpreter::exporter::FieldList t;
    a.checkEqual("01. toString", t.toString(), "");

    t.addList("a,b,c,d,e");
    a.checkEqual("11. toString", t.toString(), "A,B,C,D,E");

    t.swap(2, 3);
    a.checkEqual("21. toString", t.toString(), "A,B,D,C,E");

    t.swap(0, 0);
    a.checkEqual("31. toString", t.toString(), "A,B,D,C,E");

    t.swap(100, 100);
    a.checkEqual("41. toString", t.toString(), "A,B,D,C,E");

    t.remove(2);
    a.checkEqual("51. toString", t.toString(), "A,B,C,E");

    t.remove(0);
    a.checkEqual("61. toString", t.toString(), "B,C,E");

    t.remove(3);
    a.checkEqual("71. toString", t.toString(), "B,C,E");

    t.addList("x@5,y");
    a.checkEqual("81. toString", t.toString(), "B,C,E,X@5,Y");

    t.setFieldName(1, "D");
    t.setFieldWidth(2, 9);
    a.checkEqual("91. toString", t.toString(), "B,D,E@9,X@5,Y");

    t.setFieldName(3, "f");
    a.checkEqual("101. toString", t.toString(), "B,D,E@9,F@5,Y");

    t.toggleFieldAlignment(2);
    a.checkEqual("111. toString", t.toString(), "B,D,E@-9,F@5,Y");

    t.clear();
    a.checkEqual("121. size", t.size(), 0U);
    a.checkEqual("122. toString", t.toString(), "");
}

/** Test copying. */
AFL_TEST("interpreter.exporter.FieldList:copy", a)
{
    // (I admit that this test only serves to fill an ugly red gap in the coverage report :)
    interpreter::exporter::FieldList fa, fb;
    fa.addList("a,b@2,x");

    interpreter::exporter::FieldList fc(fa);
    fb = fa;

    a.checkEqual("01. toString", fa.toString(), "A,B@2,X");
    a.checkEqual("02. toString", fb.toString(), "A,B@2,X");
    a.checkEqual("03. toString", fc.toString(), "A,B@2,X");
    a.checkEqual("04. size", fa.size(), 3U);
    a.checkEqual("05. size", fb.size(), 3U);
    a.checkEqual("06. size", fc.size(), 3U);
}

/** Test changeFieldWidth(). */
AFL_TEST("interpreter.exporter.FieldList:changeFieldWidth", a)
{
    interpreter::exporter::FieldList f;
    f.addList("a,b@2,x");

    a.checkEqual("01. getFieldWidth", f.getFieldWidth(1), 2);

    f.changeFieldWidth(1, 10);
    a.checkEqual("11. getFieldWidth", f.getFieldWidth(1), 12);

    f.changeFieldWidth(1, -600);
    a.checkEqual("21. getFieldWidth", f.getFieldWidth(1), 0);

    f.changeFieldWidth(1, -600);
    a.checkEqual("31. getFieldWidth", f.getFieldWidth(1), -600);

    f.changeFieldWidth(1, -600);
    a.checkEqual("41. getFieldWidth", f.getFieldWidth(1), -999);
}
