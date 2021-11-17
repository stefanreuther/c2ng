/**
  *  \file util/doc/textimport.cpp
  *  \brief Text file import
  */

#include "util/doc/textimport.hpp"
#include "afl/io/textfile.hpp"
#include "util/string.hpp"

void
util::doc::importText(Index& idx, Index::Handle_t page, BlobStore& blobStore, afl::io::Stream& file, afl::charset::Charset& cs)
{
    afl::io::TextFile tf(file);
    tf.setCharsetNew(cs.clone());
    String_t blob = "<pre class=\"bare\">";
    String_t line;
    while (tf.readLine(line)) {
        blob += encodeHtml(afl::string::strRTrim(line), true);
        blob += "\n";
    }
    blob += "</pre>";

    idx.setNodeContentId(page, blobStore.addObject(afl::string::toBytes(blob)));
}
