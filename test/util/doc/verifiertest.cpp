/**
  *  \file test/util/doc/verifiertest.cpp
  *  \brief Test for util::doc::Verifier
  */

#include "util/doc/verifier.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("util.doc.Verifier:static", a)
{
    // getMessage
    NullTranslator tx;
    for (size_t i = 0; i < Verifier::MAX_MESSAGE; ++i) {
        a.checkDifferent("01. getMessage", Verifier::getMessage(static_cast<Verifier::Message>(i), tx), "");
    }

    // warningMessages + infoMessages = allMessages
    a.checkEqual("11. message set", Verifier::warningMessages() + Verifier::infoMessages(), Verifier::allMessages());
    a.checkEqual("12. message set", Verifier::warningMessages() & Verifier::infoMessages(), Verifier::Messages_t());

    // summaryMessages is a subset of allMessages
    a.checkEqual("21. message set", Verifier::summaryMessages() - Verifier::allMessages(), Verifier::Messages_t());
}

/*
 *  Test getNodeName.
 */


// Normal case
AFL_TEST("util.doc.Verifier:getNodeName:normal", a)
{
    Tester t;
    Index idx;
    Index::Handle_t doc = idx.addDocument(idx.root(), "a,b,c", "", "");
    Index::Handle_t page = idx.addPage(doc, "d,e,f", "", "");
    a.checkEqual("", t.getNodeName(idx, page), "a/d");
}

// Nameless node
AFL_TEST("util.doc.Verifier:getNodeName:nameless", a)
{
    Tester t;
    Index idx;
    Index::Handle_t doc = idx.addDocument(idx.root(), "", "", "");
    Index::Handle_t page = idx.addPage(doc, "", "", "");
    a.checkEqual("", t.getNodeName(idx, page), "(root)>#0>#0");
}

/*
 *  Test Warn_NodeHasNoId.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_NodeHasNoId:normal", a)
{
    TestHarness h;
    /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_NodeHasNoId:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_NodeHasNoId);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
}

/*
 *  Test Warn_NodeHasNoTitle.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_NodeHasNoTitle:normal", a)
{
    TestHarness h;
    /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_NodeHasNoTitle:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_NodeHasNoTitle);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
}

/*
 *  Test Warn_NodeIsEmpty.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_NodeIsEmpty:normal", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "");
    Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", "");
    /*Index::Handle_t p2 =*/ h.idx.addPage(p1, "y", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_NodeIsEmpty:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "");
    Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", "");
    Index::Handle_t p2 = h.idx.addPage(p1, "y", "Title", "");
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_NodeIsEmpty);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p2);
}

/*
 *  Test Warn_UnresolvableContent.
 */

    // Normal case
AFL_TEST("util.doc.Verifier:Warn_UnresolvableContent:normal", a)
{
    TestHarness h;
    /*Index::Handle_t doc =*/ h.idx.addDocument(h.idx.root(), "a,b,c", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_UnresolvableContent:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a,b,c", "Title", "123456");
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_UnresolvableContent);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
}

/*
 *  Test Warn_UniqueSecondaryId.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_UniqueSecondaryId:normal", a)
{
    TestHarness h;
    Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "x", "Title", h.addBlob("x"));
    Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "b", "Title", "");
    /*Index::Handle_t p2 =*/ h.idx.addPage(d2, "y,x", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_UniqueSecondaryId:error", a)
{
    TestHarness h;
    Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "x", "Title", h.addBlob("x"));
    Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "b", "Title", "");
    Index::Handle_t p2 = h.idx.addPage(d2, "y,z", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_UniqueSecondaryId);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p2);
}


/*
 *  Cannot test Warn_NestingError > this structure cannot be built using the public interface
 */

/*
 *  Test Warn_DuplicateAddress.
 */

// Case 1: duplicate document
AFL_TEST("util.doc.Verifier:Warn_DuplicateAddress:duplicate-doc", a)
{
    TestHarness h;
    /*Index::Handle_t d1 =*/ h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    /*Index::Handle_t d2 =*/ h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
    a.checkEqual("03", h.tester.msgs[0].info, "a");
}

// Case 2: duplicate page
AFL_TEST("util.doc.Verifier:Warn_DuplicateAddress:duplicate-page", a)
{
    TestHarness h;
    Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "b", "Title", h.addBlob("x"));
    /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "b", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
    a.checkEqual("03", h.tester.msgs[0].info, "a/b");
}

// Case 3: duplicate document/page combination
AFL_TEST("util.doc.Verifier:Warn_DuplicateAddress:duplicate-combo", a)
{
    TestHarness h;
    Index::Handle_t d1 = h.idx.addDocument(h.idx.root(), "a/b", "Title", h.addBlob("x"));
    /*Index::Handle_t p1 =*/ h.idx.addPage(d1, "c", "Title", h.addBlob("x"));
    Index::Handle_t d2 = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    /*Index::Handle_t p1 =*/ h.idx.addPage(d2, "b/c", "Title", h.addBlob("x"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DuplicateAddress);
    a.checkEqual("03", h.tester.msgs[0].info, "a/b/c");
}

/*
 *  Cannot test Warn_ContentError > XML parser does not throw for now
 */

/*
 *  Test Warn_InvalidComment.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_InvalidComment:normal", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p>foo</p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_InvalidComment:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<qqq>foo</qqq>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_InvalidComment);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, page);
    a.checkEqual("04. info", h.tester.msgs[0].info, "qqq");
}

/*
 *  Test Warn_AssetLink.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_AssetLink:normal", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_AssetLink:error", a)
{
    TestHarness h;
    String_t link = h.addBlob("image...");
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"asset:" + link + "/image.jpg\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_AssetLink);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, page);
    a.checkEqual("04", h.tester.msgs[0].info, link);
}

/*
 *  Test Warn_DocumentImage.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_DocumentImage:normal", a)
{
    TestHarness h;
    String_t link = h.addBlob("image...");
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:" + link + "/image.jpg\"></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_DocumentImage:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"/a/x\"></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DocumentImage);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, page);
    a.checkEqual("04. info", h.tester.msgs[0].info, "/a/x");
}

/*
 *  Test Warn_InvalidAsset.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_InvalidAsset:normal", a)
{
    TestHarness h;
    String_t link = h.addBlob("image...");
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t page =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:" + link + "/image.jpg\"></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error case
AFL_TEST("util.doc.Verifier:Warn_InvalidAsset:error", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t page = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><img src=\"asset:123456789/image.jpg\"></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_InvalidAsset);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, page);
    a.checkEqual("04. info", h.tester.msgs[0].info, "123456789");
}

/*
 *  Test Warn_DeadLink.
 */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_DeadLink:normal", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"y\">link</a></p>"));
    /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error 1
AFL_TEST("util.doc.Verifier:Warn_DeadLink:local", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p><a href=\"y\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DeadLink);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p1);
    a.checkEqual("04. info", h.tester.msgs[0].info, "a/y");
}

// Error 2
AFL_TEST("util.doc.Verifier:Warn_DeadLink:global", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", h.addBlob("<p><a href=\"/a/x\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_DeadLink);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p2);
    a.checkEqual("04. info", h.tester.msgs[0].info, "a/x");
}

/** Test Warn_BadAnchor. */

// Normal case
AFL_TEST("util.doc.Verifier:Warn_BadAnchor:normal", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1\">link</a></p>"));
    /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2\">link</a></p>"));
    /*Index::Handle_t p3 =*/ h.idx.addPage(doc, "z", "Title", h.addBlob("<p id=\"3\"><a href=\"#3\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 0U);
}

// Error 1 - absolute link with bad anchor
AFL_TEST("util.doc.Verifier:Warn_BadAnchor:absolute", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    /*Index::Handle_t p1 =*/ h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1\">link</a></p>"));
    Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2a\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p2);
    a.checkEqual("04. info", h.tester.msgs[0].info, "/a/x#2a");
}

// Error 2 - relative link with bad anchor
AFL_TEST("util.doc.Verifier:Warn_BadAnchor:relative", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1a\">link</a></p>"));
    /*Index::Handle_t p2 =*/ h.idx.addPage(doc, "y", "Title", h.addBlob("<p id=\"1\"><a href=\"/a/x#2\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p1);
    a.checkEqual("04. info", h.tester.msgs[0].info, "y#1a");
}

// Error 2a - link into document with no content
AFL_TEST("util.doc.Verifier:Warn_BadAnchor:no-anchor", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t p1 = h.idx.addPage(doc, "x", "Title", h.addBlob("<p id=\"2\"><a href=\"y#1a\">link</a></p>"));
    Index::Handle_t p2 = h.idx.addPage(doc, "y", "Title", "");
    h.idx.addPage(p2, "z", "Title", h.addBlob("y"));                // avoid "Warn_NodeIsEmpty" for p2
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p1);
    a.checkEqual("04. info", h.tester.msgs[0].info, "y#1a");
}

// Error 3 - anchor-only link
AFL_TEST("util.doc.Verifier:Warn_BadAnchor:anchor-only", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", "");
    Index::Handle_t p3 = h.idx.addPage(doc, "z", "Title", h.addBlob("<p id=\"3\"><a href=\"#3a\">link</a></p>"));
    h.verify();
    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Warn_BadAnchor);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, p3);
    a.checkEqual("04. info", h.tester.msgs[0].info, "#3a");
}

/*
 *  Test Info_UsedTags.
 */

AFL_TEST("util.doc.Verifier:Info_UsedTags", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("x"));
    Index::Handle_t page = h.idx.addPage(doc, "a", "Title", h.addBlob("x"));
    h.idx.addNodeTags(doc, "lang=en");
    h.idx.addNodeTags(page, "toc");
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 2U);
    a.checkEqual("02. msg",      h.tester.msgs[0].msg, Verifier::Info_UsedTags);
    a.checkEqual("03. doc",      h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info",     h.tester.msgs[0].info, "lang=en");
    a.checkEqual("05. msg",      h.tester.msgs[1].msg, Verifier::Info_UsedTags);
    a.checkEqual("06. handle",   h.tester.msgs[1].handle, page);
    a.checkEqual("07. info",     h.tester.msgs[1].info, "toc");
}

/*
 *  Test Info_UsedClasses.
 */

AFL_TEST("util.doc.Verifier:Info_UsedClasses", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><font color=\"green\">g!</color></p>"));
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg",      h.tester.msgs[0].msg, Verifier::Info_UsedClasses);
    a.checkEqual("03. doc",      h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info",     h.tester.msgs[0].info, "span.color-green");
}

/*
 *  Test Info_ExternalLinks.
 */

// Link
AFL_TEST("util.doc.Verifier:Info_ExternalLinks:link", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><a class=\"bare\" href=\"http://phost.de/\">link</a></p>"));
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Info_ExternalLinks);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info", h.tester.msgs[0].info, "http://phost.de/");
}

// Image
AFL_TEST("util.doc.Verifier:Info_ExternalLinks:image", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><img src=\"http://phost.de/favicon.ico\"></p>"));
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Info_ExternalLinks);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info", h.tester.msgs[0].info, "http://phost.de/favicon.ico");
}

/*
 *  Test Info_SiteLinks.
 */

// Link
AFL_TEST("util.doc.Verifier:Info_SiteLinks:link", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><a class=\"bare\" href=\"site:login.cgi\">link</a></p>"));
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Info_SiteLinks);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info", h.tester.msgs[0].info, "login.cgi");
}

// Image
AFL_TEST("util.doc.Verifier:Info_SiteLinks:image", a)
{
    TestHarness h;
    Index::Handle_t doc = h.idx.addDocument(h.idx.root(), "a", "Title", h.addBlob("<p><img src=\"site:res/upload.png\"></p>"));
    h.verify();

    a.checkEqual("01. num msgs", h.tester.msgs.size(), 1U);
    a.checkEqual("02. msg", h.tester.msgs[0].msg, Verifier::Info_SiteLinks);
    a.checkEqual("03. doc", h.tester.msgs[0].handle, doc);
    a.checkEqual("04. info", h.tester.msgs[0].info, "res/upload.png");
}
