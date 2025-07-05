/**
  *  \file util/doc/helpimport.hpp
  *  \brief Import PCC2 Help Files
  */
#ifndef C2NG_UTIL_DOC_HELPIMPORT_HPP
#define C2NG_UTIL_DOC_HELPIMPORT_HPP

#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/doc/index.hpp"

namespace util { namespace doc {

    class BlobStore;

    /** Flag for importHelp: remove source notes.
        PCC2 includes source notices in the script manual: "(from foo/bar.cc:123)".
        These defeat duplicate elimination and are not required for the web interface. */
    const int ImportHelp_RemoveSource = 1;

    /** Import PCC2 Help Files.
        Loads a help file and imports it into pages below a given root.

        - every \<page id="X"> creates a page "X"
        - the page's \<h1> produces the page name
        - the page's content is normalized as far as possible, so, when importing multiple versions, unchanged sections can be recognized.
          Page names (and thus, links) are transformed by replacing ":" to "/".
          Otherwise, doc server's XML is a superset of PCC2 help XML, so it is taken over verbatim.

        Normalisation tries hard to remove superfluous spaces (and keep valid ones).
        The main aim of normalisation is to give some independence from input layout,
        i.e. it doesn't matter when a block is re-indented.
        In addition, it gives a marginal space saving (and effort saving when rendering).

        @param [in,out] idx        Help index
        @param [in]     root       Root page
        @param [in,out] blobStore  Blob store to store transformed pages
        @param [in]     file       XML file
        @param [in]     imagePath  Image path
        @param [in]     flags      Flags
        @param [in,out] log        Log listener (warning messages)
        @param [in]     tx         Translator (warning messages) */
    void importHelp(Index& idx, Index::Handle_t root, BlobStore& blobStore, afl::io::Stream& file, afl::io::Directory& imagePath, int flags, afl::sys::LogListener& log, afl::string::Translator& tx);

    /** Import downloads.
        Reads an XML file describing the downloads, and imports those.

        - &lt;dir&gt; describes a directory; represented as a document.
          It can contain text in the same syntax as for help.
        - &lt;file&gt; represents a file; represented as a page blob.

        @param [in,out] idx        Help index
        @param [in]     root       Root page
        @param [in,out] blobStore  Blob store to store transformed pages
        @param [in]     file       XML file
        @param [in]     imagePath  Image path (for &lt;img&gt; tags in directories)
        @param [in]     filePath   File path (for importing files referenced by &lt;file&gt;)
        @param [in,out] log        Log listener (warning messages)
        @param [in]     tx         Translator (warning messages) */
    void importDownloads(Index& idx, Index::Handle_t root, BlobStore& blobStore, afl::io::Stream& file, afl::io::Directory& imagePath, afl::io::Directory& filePath, afl::sys::LogListener& log, afl::string::Translator& tx);

} }

#endif
