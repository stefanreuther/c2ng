/**
  *  \file u/t_util_doc_verifier.cpp
  *  \brief Test for util::doc::Verifier
  */

#include "util/doc/verifier.hpp"

#include "t_util_doc.hpp"
#include "afl/string/nulltranslator.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"

using afl::string::NullTranslator;
using util::doc::BlobStore;
using util::doc::Index;
using util::doc::InternalBlobStore;
using util::doc::Verifier;

namespace {
    /* A simple Verifier implementation for testing */
    class Tester : public Verifier {
     public:
        struct Msg {
            Message msg;
            Index::Handle_t handle;
            String_t info;
            Msg(Message msg, Index::Handle_t handle, String_t info)
                : msg(msg), handle(handle), info(info)
                { }
        };

        virtual void reportMessage(Message msg, const Index& /*idx*/, Index::Handle_t refNode, String_t info)
            { msgs.push_back(Msg(msg, refNode, info)); }

        std::vector<Msg> msgs;
    };

    /* All objects for testing, in a convenient package */
    struct TestHarness {
        InternalBlobStore blobStore;
        Index idx;
        Tester tester;

        void verify()
            { tester.verify(idx, blobStore); }
        BlobStore::ObjectId_t addBlob(String_t data)
            { return blobStore.addObject(afl::string::toBytes(data)); }
    };
}

/** Test the static methods. */
void
TestUtilDocVerifier::testStatic()
{
    // getMessage
    NullTranslator tx;
    for (size_t i = 0; i < Verifier::MAX_MESSAGE; ++i) {
        TS_ASSERT_DIFFERS(Verifier::getMessage(static_cast<Verifier::Message>(i), tx), "");
    }

    // warningMessages + infoMessages = allMessages
    TS_ASSERT_EQUALS(Verifier::warningMessages() + Verifier::infoMessages(), Verifier::allMessages());
    TS_ASSERT_EQUALS(Verifier::warningMessages() & Verifier::infoMessages(), Verifier::Messages_t());

    // summaryMessages is a subset of allMessages
    TS_ASSERT_EQUALS(Verifier::summaryMessages() - Verifier::allMessages(), Verifier::Messages_t());
}

/** Test getNodeName. */
void
TestUtilDocVerifier::testGetNodeName()
{
    Tester t;

    // Normal case
    {
        Index idx;
        Index::Handle_t doc = idx.addDocument(idx.root(), "a,b,c", "", "");
        Index::Handle_t page = idx.addPage(doc, "d,e,f", "", "");
        TS_ASSERT_EQUALS(t.getNodeName(idx, page), "a/d");
    }

    // Nameless node
    {
        Index idx;
        Index::Handle_t doc = idx.addDocument(idx.root(), "", "", "");
        Index::Handle_t page = idx.addPage(doc, "", "", "");
        TS_ASSERT_EQUALS(t.getNodeName(idx, page), "(root)>#0>#0");
    }
}

/** Test Warn_NodeHasNoId. */
void
TestUtilDocVerifier::testWarnNodeHasNoId()
{
    // Normal case
    {
        TestHarness h;
        /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_NodeHasNoId);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
    }
}

/** Test Warn_NodeHasNoTitle. */
void
TestUtilDocVerifier::testWarnNodeHasNoTitle()
{
    // Normal case
    {
        TestHarness h;
        /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_NodeHasNoTitle);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
    }
}

/** Test Warn_NodeIsEmpty. */
void
TestUtilDocVerifier::testWarnNodeIsEmpty()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "");
        Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", "");
        /*Index::Handle_t p2 =*/ h.idx.addPage(p1, "y", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "");
        Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", "");
        Index::Handle_t p2 = h.idx.addPage(p1, "y", "Title", "");
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_NodeIsEmpty);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p2);
    }
}

/** Test Warn_UnresolvableContent. */
void
TestUtilDocVerifier::testWarnUnresolvableContent()
{
    // Normal case
    {
        TestHarness h;
        /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "123456");
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_UnresolvableContent);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
    }
}

/** Test Warn_UniqueSecondaryId. */
void
TestUtilDocVerifier::testWarnUniqueSecondaryId()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "x", "Title", h.addBlob("x"));
        Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "b", "Title", "");
        /*Index::Handle_t p2 =*/ h.idx.addPage(d2, "y,x", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "x", "Title", h.addBlob("x"));
        Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "b", "Title", "");
        Index::Handle_t p2 = h.idx.addPage(d2, "y,z", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_UniqueSecondaryId);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p2);
    }
}

/*
 *  Cannot test Warn_NestingError > this structure cannot be built using the public interface
 */

/** Test Warn_DuplicateAddress. */
void
TestUtilDocVerifier::testWarnDuplicateAddress()
{
    // Case 1: duplicate document
    {
        TestHarness h;
        /*Index::Handle_t d1 =*/ h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
        /*Index::Handle_t d2 =*/ h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "a");
    }

    // Case 2: duplicate page
    {
        TestHarness h;
        Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
        /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "b", "Title", h.addBlob("x"));
        /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "b", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "a/b");
    }

    // Case 3: duplicate document/page combination
    {
        TestHarness h;
        Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a/b", "Title", h.addBlob("x"));
        /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "c", "Title", h.addBlob("x"));
        Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
        /*Index::Handle_t p1 =*/ h.idx.addPage(d2, "b/c", "Title", h.addBlob("x"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "a/b/c");
    }
}

/*
 *  Cannot test Warn_ContentError > XML parser does not throw for now
 */

/** Test Warn_InvalidComment. */
void
TestUtilDocVerifier::testWarnInvalidComment()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p>foo</p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<qqq>foo</qqq>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_InvalidComment);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, page);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "qqq");
    }
}

/** Test Warn_AssetLink. */
void
TestUtilDocVerifier::testWarnAssetLink()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        String_t link = h.addBlob("image...");
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"asset:" + link + "/image.jpg\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_AssetLink);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, page);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, link);
    }
}

/** Test Warn_DocumentImage. */
void
TestUtilDocVerifier::testWarnDocumentImage()
{
    // Normal case
    {
        TestHarness h;
        String_t link = h.addBlob("image...");
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:" + link + "/image.jpg\"></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"/a/x\"></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DocumentImage);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, page);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "/a/x");
    }
}

/** Test Warn_InvalidAsset. */
void
TestUtilDocVerifier::testWarnInvalidAsset()
{
    // Normal case
    {
        TestHarness h;
        String_t link = h.addBlob("image...");
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:" + link + "/image.jpg\"></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:123456789/image.jpg\"></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_InvalidAsset);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, page);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "123456789");
    }
}

/** Test Warn_DeadLink. */
void
TestUtilDocVerifier::testWarnDeadLink()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"y\">link</a></p>"));
        /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error 1
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"y\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DeadLink);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p1);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "a/y");
    }

    // Error 2
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_DeadLink);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p2);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "a/x");
    }
}

/** Test Warn_BadAnchor. */
void
TestUtilDocVerifier::testWarnBadAnchor()
{
    // Normal case
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1\">link</a></p>"));
        /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2\">link</a></p>"));
        /*Index::Handle_t p3 =*/ h.idx.addPage(doc, "z", "Title", h.addBlob("<p id=\"3\"><a href=\"#3\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 0U);
    }

    // Error 1 - absolute link with bad anchor
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1\">link</a></p>"));
        Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2a\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p2);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "/a/x#2a");
    }

    // Error 2 - relative link with bad anchor
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1a\">link</a></p>"));
        /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p1);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "y#1a");
    }

    // Error 2a - link into document with no content
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1a\">link</a></p>"));
        Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", "");
        h.idx.addPage(p2, "z", "Title", h.addBlob("y"));                // avoid "Warn_NodeIsEmpty" for p2
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p1);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "y#1a");
    }

    // Error 3 - anchor-only link
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
        Index::Handle_t p3 = h.idx.addPage(doc, "z", "Title", h.addBlob("<p id=\"3\"><a href=\"#3a\">link</a></p>"));
        h.verify();
        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, p3);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "#3a");
    }
}

/** Test Info_UsedTags. */
void
TestUtilDocVerifier::testInfoUsedTags()
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    Index::Handle_t page = h.idx.addPage(doc, "a", "Title", h.addBlob("x"));
    h.idx.addNodeTags(doc, "lang=en");
    h.idx.addNodeTags(page, "toc");
    h.verify();

    TS_ASSERT_EQUALS(h.tester.msgs.size(), 2U);
    TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_UsedTags);
    TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
    TS_ASSERT_EQUALS(h.tester.msgs[0].info, "lang=en");
    TS_ASSERT_EQUALS(h.tester.msgs[1].msg, Verifier::Info_UsedTags);
    TS_ASSERT_EQUALS(h.tester.msgs[1].handle, page);
    TS_ASSERT_EQUALS(h.tester.msgs[1].info, "toc");
}

/** Test Info_UsedClasses. */
void
TestUtilDocVerifier::testInfoUsedClasses()
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><font color=\"green\">g!</color></p>"));
    h.verify();

    TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
    TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_UsedClasses);
    TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
    TS_ASSERT_EQUALS(h.tester.msgs[0].info, "span.color-green");
}

/** Test Info_ExternalLinks. */
void
TestUtilDocVerifier::testInfoExternalLinks()
{
    // Link
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><a class=\"bare\" href=\"http://phost.de/\">link</a></p>"));
        h.verify();

        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_ExternalLinks);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "http://phost.de/");
    }

    // Image
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><img src=\"http://phost.de/favicon.ico\"></p>"));
        h.verify();

        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_ExternalLinks);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "http://phost.de/favicon.ico");
    }
}

/** Test Info_SiteLinks. */
void
TestUtilDocVerifier::testInfoSiteLinks()
{
    // Link
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><a class=\"bare\" href=\"site:login.cgi\">link</a></p>"));
        h.verify();

        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_SiteLinks);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "login.cgi");
    }

    // Image
    {
        TestHarness h;
        Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><img src=\"site:res/upload.png\"></p>"));
        h.verify();

        TS_ASSERT_EQUALS(h.tester.msgs.size(), 1U);
        TS_ASSERT_EQUALS(h.tester.msgs[0].msg, Verifier::Info_SiteLinks);
        TS_ASSERT_EQUALS(h.tester.msgs[0].handle, doc);
        TS_ASSERT_EQUALS(h.tester.msgs[0].info, "res/upload.png");
    }
}

