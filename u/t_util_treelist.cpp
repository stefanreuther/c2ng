/**
  *  \file u/t_util_treelist.cpp
  *  \brief Test for util::TreeList
  */

#include "util/treelist.hpp"

#include "t_util.hpp"

using util::TreeList;

/** Test behaviour on empty list. */
void
TestUtilTreeList::testEmpty()
{
    TreeList testee;
    TS_ASSERT(!testee.hasChildren(TreeList::root));
    TS_ASSERT_EQUALS(testee.getFirstChild(TreeList::root), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(TreeList::root), TreeList::nil);
    TS_ASSERT_EQUALS(testee.hasChildren(TreeList::root), false);
    TS_ASSERT_EQUALS(testee.findChildByLabel("", TreeList::root), TreeList::nil);
}

/** Test a normal scenario. */
void
TestUtilTreeList::testNormal()
{
    TreeList testee;
    size_t a = testee.add(10, "a", TreeList::root);
    size_t b = testee.add(20, "b", TreeList::root);
    size_t aa = testee.add(11, "a", a);
    size_t ac = testee.add(12, "c", a);
    size_t c = testee.add(30, "c", TreeList::root);
    size_t ab = testee.add(13, "b", a);
    size_t ca = testee.add(31, "a", c);

    // Verify structure
    TS_ASSERT_EQUALS(testee.getFirstChild(TreeList::root), a);
    TS_ASSERT_EQUALS(testee.getNextSibling(TreeList::root), TreeList::nil);
    TS_ASSERT_EQUALS(testee.hasChildren(TreeList::root), true);

    TS_ASSERT_EQUALS(testee.getFirstChild(a), aa);
    TS_ASSERT_EQUALS(testee.getNextSibling(a), b);
    TS_ASSERT_EQUALS(testee.hasChildren(a), true);

    TS_ASSERT_EQUALS(testee.getFirstChild(b), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(b), c);
    TS_ASSERT_EQUALS(testee.hasChildren(b), false);

    TS_ASSERT_EQUALS(testee.getFirstChild(c), ca);
    TS_ASSERT_EQUALS(testee.getNextSibling(c), TreeList::nil);
    TS_ASSERT_EQUALS(testee.hasChildren(c), true);

    TS_ASSERT_EQUALS(testee.getFirstChild(aa), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(aa), ac);
    TS_ASSERT_EQUALS(testee.hasChildren(aa), false);

    TS_ASSERT_EQUALS(testee.getFirstChild(ac), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(ac), ab);
    TS_ASSERT_EQUALS(testee.hasChildren(ac), false);

    TS_ASSERT_EQUALS(testee.getFirstChild(ab), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(ab), TreeList::nil);
    TS_ASSERT_EQUALS(testee.hasChildren(ab), false);

    TS_ASSERT_EQUALS(testee.getFirstChild(ca), TreeList::nil);
    TS_ASSERT_EQUALS(testee.getNextSibling(ca), TreeList::nil);
    TS_ASSERT_EQUALS(testee.hasChildren(ca), false);

    // Verify find
    TS_ASSERT_EQUALS(testee.findChildByLabel("a", TreeList::root), a);
    TS_ASSERT_EQUALS(testee.findChildByLabel("c", TreeList::root), c);
    TS_ASSERT_EQUALS(testee.findChildByLabel("a", a), aa);
    TS_ASSERT_EQUALS(testee.findChildByLabel("c", a), ac);

    // Verify get
    String_t s;
    int32_t key;
    TS_ASSERT_EQUALS(testee.get(ac, key, s), true);
    TS_ASSERT_EQUALS(s, "c");
    TS_ASSERT_EQUALS(key, 12);

    TS_ASSERT_EQUALS(testee.get(TreeList::nil, key, s), false);
}

/** Test addPath(). */
void
TestUtilTreeList::testAddPath()
{
    String_t p1[] = {"a","b","c"};
    String_t p2[] = {"a","b"};
    String_t p3[] = {"a","b","d"};

    TreeList testee;
    size_t a1 = testee.addPath(10, p1, TreeList::root);
    size_t a2 = testee.addPath(20, p2, TreeList::root);
    size_t a3 = testee.addPath(30, p3, TreeList::root);

    // Node a was never mentioned as a result
    size_t a = testee.getFirstChild(TreeList::root);
    TS_ASSERT_DIFFERS(a, TreeList::nil);

    // First and only child of a is a2/p2
    TS_ASSERT_EQUALS(testee.getFirstChild(a), a2);
    TS_ASSERT_EQUALS(testee.getNextSibling(a2), TreeList::nil);

    // First child of a2 is a1/p1; next sibling is a3/p3
    TS_ASSERT_EQUALS(testee.getFirstChild(a2), a1);
    TS_ASSERT_EQUALS(testee.getNextSibling(a1), a3);

    // Updating with p1 again will return same result
    size_t a4 = testee.addPath(40, p1, TreeList::root);
    TS_ASSERT_EQUALS(a4, a1);

    // Verify content
    String_t s;
    int32_t key;
    TS_ASSERT_EQUALS(testee.get(a1, key, s), true);
    TS_ASSERT_EQUALS(s, "c");
    TS_ASSERT_EQUALS(key, 40);

    TS_ASSERT_EQUALS(testee.get(a2, key, s), true);
    TS_ASSERT_EQUALS(s, "b");
    TS_ASSERT_EQUALS(key, 20);

    TS_ASSERT_EQUALS(testee.get(a3, key, s), true);
    TS_ASSERT_EQUALS(s, "d");
    TS_ASSERT_EQUALS(key, 30);

    TS_ASSERT_EQUALS(testee.get(a, key, s), true);
    TS_ASSERT_EQUALS(s, "a");
    TS_ASSERT_EQUALS(key, 0);
}

/** Test swap(). */
void
TestUtilTreeList::testSwap()
{
    TreeList t1;
    size_t a = t1.add(10, "a", TreeList::root);
    TS_ASSERT_EQUALS(t1.getFirstChild(TreeList::root), a);

    TreeList t2;
    TS_ASSERT_EQUALS(t2.getFirstChild(TreeList::root), TreeList::nil);

    // swap
    t1.swap(t2);
    TS_ASSERT_EQUALS(t1.getFirstChild(TreeList::root), TreeList::nil);
    TS_ASSERT_EQUALS(t2.getFirstChild(TreeList::root), a);
}

