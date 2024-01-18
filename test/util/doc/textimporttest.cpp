/**
  *  \file test/util/doc/textimporttest.cpp
  *  \brief Test for util::doc::TextImport
  */

#include "util/doc/textimport.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/test/testrunner.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"

using afl::charset::CodepageCharset;
using afl::io::ConstMemoryStream;
using util::doc::Index;
using util::doc::InternalBlobStore;

/** Simple test: import some text. */
AFL_TEST("util.doc.TextImport", a)
{
    // Exercise character recoding and HTML escapes
    ConstMemoryStream ms(afl::string::toBytes("Mot\xF6r\n<head>"));

    InternalBlobStore blobStore;
    Index idx;
    CodepageCharset cs(afl::charset::g_codepageLatin1);

    // Import into a document
    Index::Handle_t doc = idx.addDocument(idx.root(), "doc-url", "Doc", "");
    importText(idx, doc, blobStore, ms, cs);

    // Verify
    String_t blobId = idx.getNodeContentId(doc);
    a.check("01. getNodeContentId", !blobId.empty());

    String_t content = afl::string::fromBytes(blobStore.getObject(blobId)->get());
    a.checkEqual("11. content", content, "<pre class=\"bare\">Mot\xC3\xB6r\n&lt;head&gt;\n</pre>");
}
