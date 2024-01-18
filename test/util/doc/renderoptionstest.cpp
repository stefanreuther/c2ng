/**
  *  \file test/util/doc/renderoptionstest.cpp
  *  \brief Test for util::doc::RenderOptions
  */

#include "util/doc/renderoptions.hpp"
#include "afl/test/testrunner.hpp"

using util::doc::RenderOptions;

AFL_TEST("util.doc.RenderOptions:basics", a)
{
    RenderOptions testee;
    testee.setDocumentRoot("doc");
    testee.setSiteRoot("site");
    testee.setDocumentId("id");
    testee.setAssetRoot("asset");
    testee.setDocumentLinkSuffix("?x");

    a.checkEqual("01. getDocumentRoot",       testee.getDocumentRoot(), "doc");
    a.checkEqual("02. getSiteRoot",           testee.getSiteRoot(), "site");
    a.checkEqual("03. getDocumentId",         testee.getDocumentId(), "id");
    a.checkEqual("04. getAssetRoot",          testee.getAssetRoot(), "asset");
    a.checkEqual("05. getDocumentLinkSuffix", testee.getDocumentLinkSuffix(), "?x");
}

AFL_TEST("util.doc.RenderOptions:transformLink", a)
{
    RenderOptions testee;
    testee.setSiteRoot("site/");
    testee.setAssetRoot("asset/");
    testee.setDocumentRoot("doc/");
    testee.setDocumentId("id");
    testee.setDocumentLinkSuffix("?z");

    // Preserve global links
    a.checkEqual("01", testee.transformLink("http://1.2.3"), "http://1.2.3");
    a.checkEqual("02", testee.transformLink("https://x"), "https://x");

    // Asset
    a.checkEqual("11", testee.transformLink("asset:abcde/efg.jpg"), "asset/abcde/efg.jpg");

    // Site
    a.checkEqual("21", testee.transformLink("site:root.cgi"), "site/root.cgi");

    // Special case: link to root
    a.checkEqual("31", testee.transformLink("site:"), "site/");

    // Link to other document
    a.checkEqual("41", testee.transformLink("/foo/bar"), "doc/foo/bar?z");

    // Link to current document
    a.checkEqual("51", testee.transformLink("foo/bar"), "doc/id/foo/bar?z");

    // Fragments
    a.checkEqual("61", testee.transformLink("/a#b"), "doc/a?z#b");

    // Not confused by other special characters
    a.checkEqual("71", testee.transformLink("/a#b:c"), "doc/a?z#b:c");
    a.checkEqual("72", testee.transformLink("a#b:c"), "doc/id/a?z#b:c");

    // Link to fragment
    a.checkEqual("81", testee.transformLink("#f"), "#f");
}
