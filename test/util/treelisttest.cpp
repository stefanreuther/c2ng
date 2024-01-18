/**
  *  \file test/util/treelisttest.cpp
  *  \brief Test for util::TreeList
  */

#include "util/treelist.hpp"
#include "afl/test/testrunner.hpp"

using util::TreeList;

/** Test behaviour on empty list. */
AFL_TEST("util.TreeList:empty", a)
{
    TreeList testee;
    a.check     ("01. hasChildren",     !testee.hasChildren(TreeList::root));
    a.checkEqual("02. getFirstChild",    testee.getFirstChild(TreeList::root), TreeList::nil);
    a.checkEqual("03. getNextSibling",   testee.getNextSibling(TreeList::root), TreeList::nil);
    a.checkEqual("04. hasChildren",      testee.hasChildren(TreeList::root), false);
    a.checkEqual("05. findChildByLabel", testee.findChildByLabel("", TreeList::root), TreeList::nil);
}

/** Test a normal scenario. */
AFL_TEST("util.TreeList:normal", a)
{
    TreeList testee;
    size_t xa = testee.add(10, "a", TreeList::root);
    size_t xb = testee.add(20, "b", TreeList::root);
    size_t xaa = testee.add(11, "a", xa);
    size_t xac = testee.add(12, "c", xa);
    size_t xc = testee.add(30, "c", TreeList::root);
    size_t xab = testee.add(13, "b", xa);
    size_t xca = testee.add(31, "a", xc);

    // Verify structure
    a.checkEqual("01. getFirstChild",  testee.getFirstChild(TreeList::root), xa);
    a.checkEqual("02. getNextSibling", testee.getNextSibling(TreeList::root), TreeList::nil);
    a.checkEqual("03. hasChildren",    testee.hasChildren(TreeList::root), true);

    a.checkEqual("11. getFirstChild",  testee.getFirstChild(xa), xaa);
    a.checkEqual("12. getNextSibling", testee.getNextSibling(xa), xb);
    a.checkEqual("13. hasChildren",    testee.hasChildren(xa), true);

    a.checkEqual("21. getFirstChild",  testee.getFirstChild(xb), TreeList::nil);
    a.checkEqual("22. getNextSibling", testee.getNextSibling(xb), xc);
    a.checkEqual("23. hasChildren",    testee.hasChildren(xb), false);

    a.checkEqual("31. getFirstChild",  testee.getFirstChild(xc), xca);
    a.checkEqual("32. getNextSibling", testee.getNextSibling(xc), TreeList::nil);
    a.checkEqual("33. hasChildren",    testee.hasChildren(xc), true);

    a.checkEqual("41. getFirstChild",  testee.getFirstChild(xaa), TreeList::nil);
    a.checkEqual("42. getNextSibling", testee.getNextSibling(xaa), xac);
    a.checkEqual("43. hasChildren",    testee.hasChildren(xaa), false);

    a.checkEqual("51. getFirstChild",  testee.getFirstChild(xac), TreeList::nil);
    a.checkEqual("52. getNextSibling", testee.getNextSibling(xac), xab);
    a.checkEqual("53. hasChildren",    testee.hasChildren(xac), false);

    a.checkEqual("61. getFirstChild",  testee.getFirstChild(xab), TreeList::nil);
    a.checkEqual("62. getNextSibling", testee.getNextSibling(xab), TreeList::nil);
    a.checkEqual("63. hasChildren",    testee.hasChildren(xab), false);

    a.checkEqual("71. getFirstChild",  testee.getFirstChild(xca), TreeList::nil);
    a.checkEqual("72. getNextSibling", testee.getNextSibling(xca), TreeList::nil);
    a.checkEqual("73. hasChildren",    testee.hasChildren(xca), false);

    // Verify find
    a.checkEqual("81. findChildByLabel", testee.findChildByLabel("a", TreeList::root), xa);
    a.checkEqual("82. findChildByLabel", testee.findChildByLabel("c", TreeList::root), xc);
    a.checkEqual("83. findChildByLabel", testee.findChildByLabel("a", xa), xaa);
    a.checkEqual("84. findChildByLabel", testee.findChildByLabel("c", xa), xac);

    // Verify get
    String_t s;
    int32_t key;
    a.checkEqual("91. get", testee.get(xac, key, s), true);
    a.checkEqual("92. str", s, "c");
    a.checkEqual("93. key", key, 12);

    a.checkEqual("101. get", testee.get(TreeList::nil, key, s), false);
}

/** Test addPath(). */
AFL_TEST("util.TreeList:addPath", a)
{
    String_t p1[] = {"a","b","c"};
    String_t p2[] = {"a","b"};
    String_t p3[] = {"a","b","d"};

    TreeList testee;
    size_t a1 = testee.addPath(10, p1, TreeList::root);
    size_t a2 = testee.addPath(20, p2, TreeList::root);
    size_t a3 = testee.addPath(30, p3, TreeList::root);

    // Node a was never mentioned as a result
    size_t xa = testee.getFirstChild(TreeList::root);
    a.checkDifferent("01. getFirstChild", xa, TreeList::nil);

    // First and only child of a is a2/p2
    a.checkEqual("11. getFirstChild",  testee.getFirstChild(xa), a2);
    a.checkEqual("12. getNextSibling", testee.getNextSibling(a2), TreeList::nil);

    // First child of a2 is a1/p1; next sibling is a3/p3
    a.checkEqual("21. getFirstChild",  testee.getFirstChild(a2), a1);
    a.checkEqual("22. getNextSibling", testee.getNextSibling(a1), a3);

    // Updating with p1 again will return same result
    size_t a4 = testee.addPath(40, p1, TreeList::root);
    a.checkEqual("31. addPath", a4, a1);

    // Verify content
    String_t s;
    int32_t key;
    a.checkEqual("41. get", testee.get(a1, key, s), true);
    a.checkEqual("42. str", s, "c");
    a.checkEqual("43. key", key, 40);

    a.checkEqual("51. get", testee.get(a2, key, s), true);
    a.checkEqual("52. str", s, "b");
    a.checkEqual("53. key", key, 20);

    a.checkEqual("61. get", testee.get(a3, key, s), true);
    a.checkEqual("62. str", s, "d");
    a.checkEqual("63. key", key, 30);

    a.checkEqual("71. get", testee.get(xa, key, s), true);
    a.checkEqual("72. str", s, "a");
    a.checkEqual("73. key", key, 0);
}

/** Test swap(). */
AFL_TEST("util.TreeList:swap", a)
{
    TreeList t1;
    size_t xa = t1.add(10, "a", TreeList::root);
    a.checkEqual("01", t1.getFirstChild(TreeList::root), xa);

    TreeList t2;
    a.checkEqual("11", t2.getFirstChild(TreeList::root), TreeList::nil);

    // swap
    t1.swap(t2);
    a.checkEqual("21", t1.getFirstChild(TreeList::root), TreeList::nil);
    a.checkEqual("22", t2.getFirstChild(TreeList::root), xa);
}
