/**
  *  \file u/t_util_doc_helpimport.cpp
  *  \brief Test for util::doc::HelpImport
  */

#include "util/doc/helpimport.hpp"

#include "t_util_doc.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"

using afl::io::ConstMemoryStream;
using afl::string::NullTranslator;
using afl::sys::Log;
using util::doc::Index;
using util::doc::InternalBlobStore;

/** Generic free-form import test.
    Tests fragments taken from actual help page. */
void
TestUtilDocHelpImport::testIt()
{
    // Environment
    // (fragments taken from actual help page)
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"toc\">\n"
                                              "  <h1>PCC2 Help Table of Content</h1>\n"
                                              "  <p>Invoking <a href=\"pcc2:invoke\">PCC2</a></p>\n"
                                              " </page>\n"
                                              " <page id=\"group:invoking\">\n"
                                              "  <h1>Invocation</h1>\n"
                                              "\n"
                                              "  <page id=\"pcc2:invoke\">\n"
                                              "   <h1>Invoking PCC2</h1>\n"
                                              "   <p>PCC2 is a graphical application.</p>\n"
                                              "  </page>\n"
                                              " </page>\n"
                                              " <page id=\"int:expr:grammar\">"
                                              "  <h1>Expressions: Formal Grammar</h1>\n"
                                              "  <p>This formal grammar describes expressions.</p>\n"
                                              "  <pre>\n"
                                              "<u>sequence</u>:\n"
                                              "    <u>assignment</u></pre>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 3U);

    // "Invoking PCC2" page
    Index::Handle_t invPage;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/pcc2/invoke", invPage, tmp));
    TS_ASSERT(idx.isNodePage(invPage));
    TS_ASSERT_EQUALS(idx.getNodeTitle(invPage), "Invoking PCC2");

    String_t invContent = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(invPage))->get());
    TS_ASSERT_EQUALS(invContent, "<p>PCC2 is a graphical application.</p>");

    // "TOC" page
    Index::Handle_t tocPage;
    TS_ASSERT(idx.findNodeByAddress("doc-url/toc", tocPage, tmp));
    TS_ASSERT(idx.isNodePage(tocPage));
    TS_ASSERT_EQUALS(idx.getNodeTitle(tocPage), "PCC2 Help Table of Content");

    String_t tocContent = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(tocPage))->get());
    TS_ASSERT_EQUALS(tocContent, "<p>Invoking <a href=\"pcc2/invoke\">PCC2</a></p>");

    // Grammar page
    Index::Handle_t exprPage;
    TS_ASSERT(idx.findNodeByAddress("doc-url/int/expr/grammar", exprPage, tmp));
    TS_ASSERT(idx.isNodePage(exprPage));
    TS_ASSERT_EQUALS(idx.getNodeTitle(exprPage), "Expressions: Formal Grammar");

    String_t exprContent = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(exprPage))->get());
    TS_ASSERT_EQUALS(exprContent,
                     "<p>This formal grammar describes expressions.</p>"
                     "<pre><u>sequence</u>:\n"
                     "    <u>assignment</u></pre>");
}

/** Import test.
    Exercises whitespace handling: a whitespace node between two free-form tags must not be deleted. */
void
TestUtilDocHelpImport::testIt2()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"a\">\n"
                                              "  <h1>Heading</h1>\n"
                                              "  <p> <b>Warning:</b> <em>hot!</em></p>\n"
                                              //    ^               ^ this space is kept
                                              //    ^ this space is removed
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);

    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/a", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "Heading");

    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    TS_ASSERT_EQUALS(content, "<p><b>Warning:</b> <em>hot!</em></p>");
}

/** Import test.
    Exercises link handling. */
void
TestUtilDocHelpImport::testIt3()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"a:b\">\n"
                                              "  <h1>Heading</h1>\n"
                                              "  <p>\n"
                                              "   <a href=\"http://web.link/\">web</a>\n"
                                              "   <a href=\"site:index.cgi\">site</a>\n"
                                              "   <a href=\"/other/doc\">other</a>\n"
                                              "   <a href=\"a:b\">same</a>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);

    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/a/b", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "Heading");

    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    TS_ASSERT_EQUALS(content,
                     "<p>"
                     "<a href=\"http://web.link/\">web</a> "
                     "<a href=\"site:index.cgi\">site</a> "
                     "<a href=\"/other/doc\">other</a> "
                     "<a href=\"a/b\">same</a>"
                     "</p>");
}

/** Import test.
    Exercises trimming of space between blocks. */
void
TestUtilDocHelpImport::testIt4()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"a:b\">\n"
                                              "  <h1>Heading</h1>\n"
                                              // Space between these blocks is removed per "no containing tag" rule:
                                              "  <p>a</p>\n"
                                              "  <p>b</p>\n"
                                              // Space between list items is removed per "isBlockContext tag" rule:
                                              "  <ul>\n"
                                              "   <li>x</li>\n"
                                              "   <li>y</li>\n"
                                              "  </ul>\n"
                                              // Space between flow-text markup tags is kept:
                                              "  <p><b>m</b> <b>n</b></p>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);

    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/a/b", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "Heading");

    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    TS_ASSERT_EQUALS(content,
                     "<p>a</p><p>b</p>"
                     "<ul><li>x</li><li>y</li></ul>"
                     "<p><b>m</b> <b>n</b></p>");
}

