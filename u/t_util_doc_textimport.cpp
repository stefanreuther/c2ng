/**
  *  \file u/t_util_doc_textimport.cpp
  *  \brief Test for util::doc::TextImport
  */

#include "util/doc/textimport.hpp"

#include "t_util_doc.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"

using afl::charset::CodepageCharset;
using afl::io::ConstMemoryStream;
using util::doc::Index;
using util::doc::InternalBlobStore;

/** Simple test: import some text. */
void
TestUtilDocTextImport::testIt()
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
    TS_ASSERT(!blobId.empty());

    String_t content = afl::string::fromBytes(blobStore.getObject(blobId)->get());
    TS_ASSERT_EQUALS(content, "<pre class=\"bare\">Mot\xC3\xB6r\n&lt;head&gt;\n</pre>");
}

