/**
  *  \file u/t_util_doc_renderoptions.cpp
  *  \brief Test for util::doc::RenderOptions
  */

#include "util/doc/renderoptions.hpp"

#include "t_util_doc.hpp"

using util::doc::RenderOptions;

void
TestUtilDocRenderOptions::testSet()
{
    RenderOptions testee;
    testee.setDocumentRoot("doc");
    testee.setSiteRoot("site");
    testee.setDocumentId("id");
    testee.setAssetRoot("asset");
    testee.setDocumentLinkSuffix("?x");

    TS_ASSERT_EQUALS(testee.getDocumentRoot(), "doc");
    TS_ASSERT_EQUALS(testee.getSiteRoot(), "site");
    TS_ASSERT_EQUALS(testee.getDocumentId(), "id");
    TS_ASSERT_EQUALS(testee.getAssetRoot(), "asset");
    TS_ASSERT_EQUALS(testee.getDocumentLinkSuffix(), "?x");
}

void
TestUtilDocRenderOptions::testLink()
{
    RenderOptions testee;
    testee.setSiteRoot("site/");
    testee.setAssetRoot("asset/");
    testee.setDocumentRoot("doc/");
    testee.setDocumentId("id");
    testee.setDocumentLinkSuffix("?z");

    // Preserve global links
    TS_ASSERT_EQUALS(testee.transformLink("http://1.2.3"), "http://1.2.3");
    TS_ASSERT_EQUALS(testee.transformLink("https://x"), "https://x");

    // Asset
    TS_ASSERT_EQUALS(testee.transformLink("asset:abcde/efg.jpg"), "asset/abcde/efg.jpg");

    // Site
    TS_ASSERT_EQUALS(testee.transformLink("site:root.cgi"), "site/root.cgi");

    // Special case: link to root
    TS_ASSERT_EQUALS(testee.transformLink("site:"), "site/");

    // Link to other document
    TS_ASSERT_EQUALS(testee.transformLink("/foo/bar"), "doc/foo/bar?z");

    // Link to current document
    TS_ASSERT_EQUALS(testee.transformLink("foo/bar"), "doc/id/foo/bar?z");

    // Fragments
    TS_ASSERT_EQUALS(testee.transformLink("/a#b"), "doc/a?z#b");

    // Not confused by other special characters
    TS_ASSERT_EQUALS(testee.transformLink("/a#b:c"), "doc/a?z#b:c");
    TS_ASSERT_EQUALS(testee.transformLink("a#b:c"), "doc/id/a?z#b:c");

    // Link to fragment
    TS_ASSERT_EQUALS(testee.transformLink("#f"), "#f");
}

