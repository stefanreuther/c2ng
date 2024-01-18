/**
  *  \file test/util/datatabletest.cpp
  *  \brief Test for util::DataTable
  */

#include "util/datatable.hpp"
#include "afl/test/testrunner.hpp"

using util::DataTable;

/** Test basics: adding and querying data. */
AFL_TEST("util.DataTable:basics", a)
{
    // Initial state
    DataTable testee;
    const DataTable& ct = testee;

    a.checkEqual("01. getNumRows", testee.getNumRows(), 0U);
    a.checkEqual("02. getNumRows", ct.getNumRows(), 0U);

    a.checkEqual("11. getNumColumns", testee.getNumColumns(), 0);
    a.checkEqual("12. getNumColumns", ct.getNumColumns(), 0);

    a.checkNull("21. getRow", testee.getRow(0));
    a.checkNull("22. getRow", ct.getRow(0));

    a.check("31. getValueRange", ct.getValueRange().empty());

    a.checkNull("41. findRowById", testee.findRowById(10));
    a.checkNull("42. findRowById", ct.findRowById(10));

    a.checkNull("51. findNextRowById", testee.findNextRowById(0));
    a.checkNull("52. findNextRowById", ct.findNextRowById(0));

    // Add two rows
    DataTable::Row& c1 = testee.addRow(10);
    DataTable::Row& c2 = testee.addRow(20);

    a.checkEqual("61. getNumRows", testee.getNumRows(), 2U);
    a.checkEqual("62. getName", c1.getName(), "");
    a.checkEqual("63. getName", c2.getName(), "");

    a.checkEqual("71. getRow", testee.getRow(0), &c1);
    a.checkEqual("72. getRow", ct.getRow(0), &c1);

    a.checkEqual("81. getRow", testee.getRow(1), &c2);
    a.checkEqual("82. getRow", ct.getRow(1), &c2);

    a.checkEqual("91. getIndex", c1.getIndex(), 0U);
    a.checkEqual("92. getId", c1.getId(), 10);

    a.checkEqual("101. getIndex", c2.getIndex(), 1U);
    a.checkEqual("102. getId", c2.getId(), 20);

    a.checkEqual("111. findRowById", testee.findRowById(10), &c1);
    a.checkEqual("112. findRowById", ct.findRowById(10), &c1);

    a.checkEqual("121. findRowById", testee.findRowById(20), &c2);
    a.checkEqual("122. findRowById", ct.findRowById(20), &c2);

    a.check("131. getValueRange", c1.getValueRange().empty());
    a.check("132. getValueRange", c2.getValueRange().empty());

    a.checkEqual("141. getNumColumns", c1.getNumColumns(), 0);
    a.checkEqual("142. getNumColumns", c2.getNumColumns(), 0);

    a.check("151. isValid", !c1.get(0).isValid());

    // Add values
    c1.set(0, 5);
    c1.set(4, 3);
    c1.set(7, DataTable::Value_t());
    // --> c1 now [5,-,-,-,-,3,-,-,-]

    static const int32_t v1s[] = { 3, 1, 4, 1, 5 };
    c2.set(3, v1s);

    DataTable::Value_t v2s[] = { 2, afl::base::Nothing, 4 };
    c2.set(4, v2s);
    // --> c2 now [-,-,-,3,2,-,4,5]

    c1.setName("one");
    c2.setName("two");

    // Verify
    a.checkEqual("161. getNumColumns", c1.getNumColumns(), 5);
    a.checkEqual("162. getNumColumns", c2.getNumColumns(), 8);
    a.checkEqual("163. getNumColumns", ct.getNumColumns(), 8);

    a.checkEqual("171. get", c1.get(0).orElse(-1), 5);
    a.checkEqual("172. get", c1.get(7).orElse(-1), -1);
    a.checkEqual("173. get", c2.get(4).orElse(-1), 2);
    a.checkEqual("174. get", c2.get(5).orElse(-1), -1);

    a.checkEqual("181. getValueRange", c1.getValueRange().min(), 3);
    a.checkEqual("182. getValueRange", c1.getValueRange().max(), 5);
    a.checkEqual("183. getValueRange", c2.getValueRange().min(), 2);
    a.checkEqual("184. getValueRange", c2.getValueRange().max(), 5);
    a.checkEqual("185. getValueRange", ct.getValueRange().min(), 2);
    a.checkEqual("186. getValueRange", ct.getValueRange().max(), 5);

    a.checkEqual("191. getName", c1.getName(), "one");
    a.checkEqual("192. getName", c2.getName(), "two");
}

/** Test name operations. */
AFL_TEST("util.DataTable:setColumnName", a)
{
    DataTable ta;
    DataTable tb;
    a.checkEqual("01", ta.getColumnName(7), "");
    a.checkEqual("02", tb.getColumnName(7), "");

    ta.setColumnName(7, "seven");
    a.checkEqual("11", ta.getColumnName(7), "seven");

    tb.copyColumnNames(ta);
    a.checkEqual("21", tb.getColumnName(7), "seven");
}

/** Test iteration. */
AFL_TEST("util.DataTable:iteration", a)
{
    DataTable t;
    DataTable::Row& c1 = t.addRow(10);
    DataTable::Row& c2 = t.addRow(20);
    DataTable::Row& c3 = t.addRow(10);
    DataTable::Row& c4 = t.addRow(40);
    const DataTable::Row* nullc = 0;

    a.checkEqual("01", t.findRowById(10), &c1);
    a.checkEqual("02", t.findNextRowById(&c1), &c3);
    a.checkEqual("03", t.findNextRowById(&c3), nullc);

    a.checkEqual("11", t.findRowById(20), &c2);
    a.checkEqual("12", t.findNextRowById(&c2), nullc);

    a.checkEqual("21", t.findRowById(40), &c4);
    a.checkEqual("22", t.findNextRowById(&c4), nullc);

    a.checkEqual("31", t.findRowById(50),  nullc);
}

/** Test stack(). */
AFL_TEST("util.DataTable:stack", a)
{
    DataTable t;
    DataTable::Row& c1 = t.addRow(10);
    DataTable::Row& c2 = t.addRow(20);

    c1.set(0, 10);
    c1.set(1, 20);
    c1.set(4, 30);

    c2.set(0,  3);
    c2.set(2,  5);

    t.stack();

    a.checkEqual("01", c1.get(0).orElse(-1), 10);
    a.checkEqual("02", c1.get(1).orElse(-1), 20);
    a.checkEqual("03", c1.get(2).orElse(-1), -1);
    a.checkEqual("04", c1.get(3).orElse(-1), -1);
    a.checkEqual("05", c1.get(4).orElse(-1), 30);

    a.checkEqual("11", c2.get(0).orElse(-1), 13);
    a.checkEqual("12", c2.get(1).orElse(-1), 20);
    a.checkEqual("13", c2.get(2).orElse(-1),  5);
    a.checkEqual("14", c2.get(3).orElse(-1), -1);
    a.checkEqual("15", c2.get(4).orElse(-1), 30);
}

/** Test append() variants. */
AFL_TEST("util.DataTable:append", a)
{
    DataTable t1, t2, t3;
    t1.addRow(10).set(0, 10);
    t2.addRow(20).set(0, 20);
    t3.addRow(30).set(0, 30);
    t1.getRow(0)->setName("one");
    t2.getRow(0)->setName("two");
    t3.getRow(0)->setName("three");

    t1.appendCopy(t2);
    t1.appendMove(t3);

    a.checkEqual("01. getNumRows", t1.getNumRows(), 3U);
    a.checkEqual("02. getNumRows", t2.getNumRows(), 1U);
    a.checkEqual("03. getNumRows", t3.getNumRows(), 0U);

    a.checkEqual("11", t1.getRow(0)->getId(), 10);
    a.checkEqual("12", t1.getRow(1)->getId(), 20);
    a.checkEqual("13", t1.getRow(2)->getId(), 30);
    a.checkEqual("14", t1.getRow(0)->getIndex(), 0U);
    a.checkEqual("15", t1.getRow(1)->getIndex(), 1U);
    a.checkEqual("16", t1.getRow(2)->getIndex(), 2U);
    a.checkEqual("17", t1.getRow(0)->getName(), "one");
    a.checkEqual("18", t1.getRow(1)->getName(), "two");
    a.checkEqual("19", t1.getRow(2)->getName(), "three");
}

/** Test add(). */
AFL_TEST("util.DataTable:add", a)
{
    // Table 1
    DataTable t1;
    DataTable::Row& c11 = t1.addRow(10);
    c11.set(0, 10);
    c11.set(1, 20);
    DataTable::Row& c12 = t1.addRow(20);
    c12.set(0, 5);
    c12.set(1, 6);

    // Table 2
    DataTable t2;
    DataTable::Row& c21 = t2.addRow(10);
    c21.set(0, 3);
    c21.set(1, -7);

    // Action
    t1.add(3, t2);

    // Verify
    a.checkEqual("01", c11.get(0).orElse(-999), 19);
    a.checkEqual("02", c11.get(1).orElse(-999), -1);
    a.checkEqual("03", c12.get(0).orElse(-999), 5);
    a.checkEqual("04", c12.get(1).orElse(-999), 6);
}

/** Test sort(). */
AFL_TEST("util.DataTable:sort", a)
{
    util::DataTable t;
    DataTable::Row& r1 = t.addRow(1);
    DataTable::Row& r3 = t.addRow(3);
    DataTable::Row& r2 = t.addRow(2);
    a.checkEqual("01. getIndex", r1.getIndex(), 0U);
    a.checkEqual("02. getIndex", r3.getIndex(), 1U);
    a.checkEqual("03. getIndex", r2.getIndex(), 2U);

    struct Sorter : public afl::functional::BinaryFunction<const DataTable::Row&, const DataTable::Row&, bool> {
        virtual bool get(const DataTable::Row& a, const DataTable::Row& b) const
            { return a.getId() < b.getId(); }
    };
    t.sortRows(Sorter());

    a.checkEqual("11. getIndex", r1.getIndex(), 0U);
    a.checkEqual("12. getIndex", r2.getIndex(), 1U);
    a.checkEqual("13. getIndex", r3.getIndex(), 2U);
    a.checkEqual("14. getRow", t.getRow(0), &r1);
    a.checkEqual("15. getRow", t.getRow(1), &r2);
    a.checkEqual("16. getRow", t.getRow(2), &r3);
}
