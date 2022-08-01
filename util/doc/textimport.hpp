/**
  *  \file util/doc/textimport.hpp
  *  \brief Text file import
  */
#ifndef C2NG_UTIL_DOC_TEXTIMPORT_HPP
#define C2NG_UTIL_DOC_TEXTIMPORT_HPP

#include "util/doc/index.hpp"
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"

namespace util { namespace doc {

    class BlobStore;

    /** Import a single text file.
        The file is imported as a single blob into the given page.
        The text is not modified or formatted.

        @param [in,out] idx        Help index
        @param [in]     page       Page
        @param [in,out] blobStore  Blob store to store transformed pages
        @param [in]     file       File
        @param [in]     cs         Character set */
    void importText(Index& idx, Index::Handle_t page, BlobStore& blobStore, afl::io::Stream& file, afl::charset::Charset& cs);

} }

#endif
