/**
  *  \file u/t_util_doc_helpimport.cpp
  *  \brief Test for util::doc::HelpImport
  */

#include "util/doc/helpimport.hpp"

#include "t_util_doc.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"
#include "afl/test/loglistener.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;
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
    Ref<InternalDirectory> dir = InternalDirectory::create("testIt");

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

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
    Ref<InternalDirectory> dir = InternalDirectory::create("testIt2");

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

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
    Ref<InternalDirectory> dir = InternalDirectory::create("testIt3");

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

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
TestUtilDocHelpImport::testIt5()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"a:b\">\n"
                                              "  <h1>Heading</h1>\n"
                                              "  bogus text\n"
                                              "  <p>good text</p>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    afl::test::LogListener log;
    NullTranslator tx;
    Index idx;
    Ref<InternalDirectory> dir = InternalDirectory::create("testIt5");

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);

    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/a/b", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "Heading");

    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    TS_ASSERT_EQUALS(content, "bogus text<p>good text</p>");

    // Import must have created a warning
    TS_ASSERT_LESS_THAN_EQUALS(1U, log.getNumWarnings());
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
    Ref<InternalDirectory> dir = InternalDirectory::create("testIt4");

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");

    // Do it
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

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

void
TestUtilDocHelpImport::testRemoveSource()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"p\">\n"
                                              "  <h1>H</h1>\n"
                                              "  <p>text...</p>\n"
                                              "  <p><b>See also: </b><a href=\"q\">Hooks</a></p>\n"
                                              "  <p><font color=\"dim\"><small>(from doc/interpreter_manual.txt:2083)</small></font></p>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;
    Ref<InternalDirectory> dir = InternalDirectory::create("testRemoveSource");

    // Do it
    ms.setPos(0);
    importHelp(idx, idx.addDocument(idx.root(), "off", "Doc", ""), blobStore, ms, *dir, 0, log, tx);
    ms.setPos(0);
    importHelp(idx, idx.addDocument(idx.root(), "on", "Doc", ""), blobStore, ms, *dir, util::doc::ImportHelp_RemoveSource, log, tx);

    // Verify
    {
        Index::Handle_t page;
        String_t tmp;
        TS_ASSERT(idx.findNodeByAddress("off/p", page, tmp));
        TS_ASSERT(idx.isNodePage(page));
        TS_ASSERT_EQUALS(idx.getNodeTitle(page), "H");

        String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
        TS_ASSERT_EQUALS(content,
                         "<p>text...</p>"
                         "<p><b>See also: </b><a href=\"q\">Hooks</a></p>"
                         "<p><font color=\"dim\"><small>(from doc/interpreter_manual.txt:2083)</small></font></p>");
    }
    {
        Index::Handle_t page;
        String_t tmp;
        TS_ASSERT(idx.findNodeByAddress("on/p", page, tmp));
        TS_ASSERT(idx.isNodePage(page));
        TS_ASSERT_EQUALS(idx.getNodeTitle(page), "H");

        String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
        TS_ASSERT_EQUALS(content,
                         "<p>text...</p>"
                         "<p><b>See also: </b><a href=\"q\">Hooks</a></p>");
    }
}

/** Test importing images (<img src> with relative URL). */
void
TestUtilDocHelpImport::testImportImage()
{
    // Environment
    static const uint8_t PIXEL[] = {
        0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x01, 0x00, 0x01, 0x00, 0xf0, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x4c, 0x01, 0x00, 0x3b
    };
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"p\">\n"
                                              "  <h1>H</h1>\n"
                                              "  <p>text...<img src=\"pixel.gif\" /></p>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    Log log;
    NullTranslator tx;
    Index idx;
    Ref<InternalDirectory> dir = InternalDirectory::create("testImportImage");
    dir->openFile("pixel.gif", afl::io::FileSystem::Create)->fullWrite(PIXEL);

    // Do it
    ms.setPos(0);
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);
    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/p", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "H");

    // Verify content [do not rely on the exact name of the picture]
    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    String_t pixelId = blobStore.addObject(PIXEL);
    TS_ASSERT_DIFFERS(pixelId, "");
    TS_ASSERT_EQUALS(content, "<p>text...<img src=\"asset:" + pixelId + "/pixel.gif\"/></p>");
}

/** Test failure to import image. */
void
TestUtilDocHelpImport::testImportImageFail()
{
    // Environment
    ConstMemoryStream ms(afl::string::toBytes("<?xml version=\"1.0\"?>\n"
                                              "<!DOCTYPE help SYSTEM \"pcc2help.dtd\">\n"
                                              "<help priority=\"99\">\n"
                                              " <page id=\"p\">\n"
                                              "  <h1>H</h1>\n"
                                              "  <p>text...<img src=\"pixel.gif\" /></p>\n"
                                              " </page>\n"
                                              "</help>\n"));
    InternalBlobStore blobStore;
    afl::test::LogListener log;
    NullTranslator tx;
    Index idx;
    Ref<InternalDirectory> dir = InternalDirectory::create("testImportImageFail");

    // Do it
    ms.setPos(0);
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");
    importHelp(idx, doc, blobStore, ms, *dir, 0, log, tx);

    // Verify
    TS_ASSERT_EQUALS(idx.getNumNodeChildren(doc), 1U);
    Index::Handle_t page;
    String_t tmp;
    TS_ASSERT(idx.findNodeByAddress("doc-url/p", page, tmp));
    TS_ASSERT(idx.isNodePage(page));
    TS_ASSERT_EQUALS(idx.getNodeTitle(page), "H");

    // Verify content: image tag does not receive a src attribute because we cannot translate it
    String_t content = afl::string::fromBytes(blobStore.getObject(idx.getNodeContentId(page))->get());
    TS_ASSERT_EQUALS(content, "<p>text...<img/></p>");

    // Import must have created a warning
    TS_ASSERT_LESS_THAN_EQUALS(1U, log.getNumWarnings());
}

