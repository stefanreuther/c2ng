/**
  *  \file u/t_util_datatable.cpp
  *  \brief Test for util::DataTable
  */

#include "util/datatable.hpp"

#include "t_util.hpp"

using util::DataTable;

/** Test basics: adding and querying data. */
void
TestUtilDataTable::testBasics()
{
    // Initial state
    DataTable testee;
    const DataTable& ct = testee;

    TS_ASSERT_EQUALS(testee.getNumRows(), 0U);
    TS_ASSERT_EQUALS(ct.getNumRows(), 0U);

    TS_ASSERT_EQUALS(testee.getNumColumns(), 0);
    TS_ASSERT_EQUALS(ct.getNumColumns(), 0);

    TS_ASSERT(testee.getRow(0) == 0);
    TS_ASSERT(ct.getRow(0) == 0);

    TS_ASSERT(ct.getValueRange().empty());

    TS_ASSERT(testee.findRowById(10) == 0);
    TS_ASSERT(ct.findRowById(10) == 0);

    TS_ASSERT(testee.findNextRowById(0) == 0);
    TS_ASSERT(ct.findNextRowById(0) == 0);

    // Add two rows
    DataTable::Row& c1 = testee.addRow(10);
    DataTable::Row& c2 = testee.addRow(20);

    TS_ASSERT_EQUALS(testee.getNumRows(), 2U);
    TS_ASSERT_EQUALS(c1.getName(), "");
    TS_ASSERT_EQUALS(c2.getName(), "");

    TS_ASSERT_EQUALS(testee.getRow(0), &c1);
    TS_ASSERT_EQUALS(ct.getRow(0), &c1);

    TS_ASSERT_EQUALS(testee.getRow(1), &c2);
    TS_ASSERT_EQUALS(ct.getRow(1), &c2);

    TS_ASSERT_EQUALS(c1.getIndex(), 0U);
    TS_ASSERT_EQUALS(c1.getId(), 10);

    TS_ASSERT_EQUALS(c2.getIndex(), 1U);
    TS_ASSERT_EQUALS(c2.getId(), 20);

    TS_ASSERT_EQUALS(testee.findRowById(10), &c1);
    TS_ASSERT_EQUALS(ct.findRowById(10), &c1);

    TS_ASSERT_EQUALS(testee.findRowById(20), &c2);
    TS_ASSERT_EQUALS(ct.findRowById(20), &c2);

    TS_ASSERT(c1.getValueRange().empty());
    TS_ASSERT(c2.getValueRange().empty());

    TS_ASSERT_EQUALS(c1.getNumColumns(), 0);
    TS_ASSERT_EQUALS(c2.getNumColumns(), 0);

    TS_ASSERT(!c1.get(0).isValid());

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
    TS_ASSERT_EQUALS(c1.getNumColumns(), 5);
    TS_ASSERT_EQUALS(c2.getNumColumns(), 8);
    TS_ASSERT_EQUALS(ct.getNumColumns(), 8);

    TS_ASSERT_EQUALS(c1.get(0).orElse(-1), 5);
    TS_ASSERT_EQUALS(c1.get(7).orElse(-1), -1);
    TS_ASSERT_EQUALS(c2.get(4).orElse(-1), 2);
    TS_ASSERT_EQUALS(c2.get(5).orElse(-1), -1);

    TS_ASSERT_EQUALS(c1.getValueRange().min(), 3);
    TS_ASSERT_EQUALS(c1.getValueRange().max(), 5);
    TS_ASSERT_EQUALS(c2.getValueRange().min(), 2);
    TS_ASSERT_EQUALS(c2.getValueRange().max(), 5);
    TS_ASSERT_EQUALS(ct.getValueRange().min(), 2);
    TS_ASSERT_EQUALS(ct.getValueRange().max(), 5);

    TS_ASSERT_EQUALS(c1.getName(), "one");
    TS_ASSERT_EQUALS(c2.getName(), "two");
}

/** Test name operations. */
void
TestUtilDataTable::testNames()
{
    DataTable a;
    DataTable b;
    TS_ASSERT_EQUALS(a.getColumnName(7), "");
    TS_ASSERT_EQUALS(b.getColumnName(7), "");

    a.setColumnName(7, "seven");
    TS_ASSERT_EQUALS(a.getColumnName(7), "seven");

    b.copyColumnNames(a);
    TS_ASSERT_EQUALS(b.getColumnName(7), "seven");
}

/** Test iteration. */
void
TestUtilDataTable::testIteration()
{
    DataTable t;
    DataTable::Row& c1 = t.addRow(10);
    DataTable::Row& c2 = t.addRow(20);
    DataTable::Row& c3 = t.addRow(10);
    DataTable::Row& c4 = t.addRow(40);
    const DataTable::Row* nullc = 0;

    TS_ASSERT_EQUALS(t.findRowById(10), &c1);
    TS_ASSERT_EQUALS(t.findNextRowById(&c1), &c3);
    TS_ASSERT_EQUALS(t.findNextRowById(&c3), nullc);

    TS_ASSERT_EQUALS(t.findRowById(20), &c2);
    TS_ASSERT_EQUALS(t.findNextRowById(&c2), nullc);

    TS_ASSERT_EQUALS(t.findRowById(40), &c4);
    TS_ASSERT_EQUALS(t.findNextRowById(&c4), nullc);

    TS_ASSERT_EQUALS(t.findRowById(50),  nullc);
}

/** Test stack(). */
void
TestUtilDataTable::testStack()
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

    TS_ASSERT_EQUALS(c1.get(0).orElse(-1), 10);
    TS_ASSERT_EQUALS(c1.get(1).orElse(-1), 20);
    TS_ASSERT_EQUALS(c1.get(2).orElse(-1), -1);
    TS_ASSERT_EQUALS(c1.get(3).orElse(-1), -1);
    TS_ASSERT_EQUALS(c1.get(4).orElse(-1), 30);

    TS_ASSERT_EQUALS(c2.get(0).orElse(-1), 13);
    TS_ASSERT_EQUALS(c2.get(1).orElse(-1), 20);
    TS_ASSERT_EQUALS(c2.get(2).orElse(-1),  5);
    TS_ASSERT_EQUALS(c2.get(3).orElse(-1), -1);
    TS_ASSERT_EQUALS(c2.get(4).orElse(-1), 30);
}

/** Test append() variants. */
void
TestUtilDataTable::testAppend()
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

    TS_ASSERT_EQUALS(t1.getNumRows(), 3U);
    TS_ASSERT_EQUALS(t2.getNumRows(), 1U);
    TS_ASSERT_EQUALS(t3.getNumRows(), 0U);

    TS_ASSERT_EQUALS(t1.getRow(0)->getId(), 10);
    TS_ASSERT_EQUALS(t1.getRow(1)->getId(), 20);
    TS_ASSERT_EQUALS(t1.getRow(2)->getId(), 30);
    TS_ASSERT_EQUALS(t1.getRow(0)->getIndex(), 0U);
    TS_ASSERT_EQUALS(t1.getRow(1)->getIndex(), 1U);
    TS_ASSERT_EQUALS(t1.getRow(2)->getIndex(), 2U);
    TS_ASSERT_EQUALS(t1.getRow(0)->getName(), "one");
    TS_ASSERT_EQUALS(t1.getRow(1)->getName(), "two");
    TS_ASSERT_EQUALS(t1.getRow(2)->getName(), "three");
}

/** Test add(). */
void
TestUtilDataTable::testAdd()
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
    TS_ASSERT_EQUALS(c11.get(0).orElse(-999), 19);
    TS_ASSERT_EQUALS(c11.get(1).orElse(-999), -1);
    TS_ASSERT_EQUALS(c12.get(0).orElse(-999), 5);
    TS_ASSERT_EQUALS(c12.get(1).orElse(-999), 6);
}

/** Test sort(). */
void
TestUtilDataTable::testSort()
{
    util::DataTable t;
    DataTable::Row& r1 = t.addRow(1);
    DataTable::Row& r3 = t.addRow(3);
    DataTable::Row& r2 = t.addRow(2);
    TS_ASSERT_EQUALS(r1.getIndex(), 0U);
    TS_ASSERT_EQUALS(r3.getIndex(), 1U);
    TS_ASSERT_EQUALS(r2.getIndex(), 2U);

    struct Sorter : public afl::functional::BinaryFunction<const DataTable::Row&, const DataTable::Row&, bool> {
        virtual bool get(const DataTable::Row& a, const DataTable::Row& b) const
            { return a.getId() < b.getId(); }
    };
    t.sortRows(Sorter());

    TS_ASSERT_EQUALS(r1.getIndex(), 0U);
    TS_ASSERT_EQUALS(r2.getIndex(), 1U);
    TS_ASSERT_EQUALS(r3.getIndex(), 2U);
    TS_ASSERT_EQUALS(t.getRow(0), &r1);
    TS_ASSERT_EQUALS(t.getRow(1), &r2);
    TS_ASSERT_EQUALS(t.getRow(2), &r3);
}

