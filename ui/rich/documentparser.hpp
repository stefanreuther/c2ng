/**
  *  \file ui/rich/documentparser.hpp
  *  \brief Class ui::rich::DocumentParser
  */
#ifndef C2NG_UI_RICH_DOCUMENTPARSER_HPP
#define C2NG_UI_RICH_DOCUMENTPARSER_HPP

#include <vector>
#include "afl/container/ptrvector.hpp"
#include "afl/io/xml/basereader.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/text.hpp"

namespace ui { namespace rich {

    class Document;

    /** Document parser.
        Parses a token stream from a afl::io::xml::BaseReader and renders it on a Document.
        This will use the Document's gfx::ResourceProvider,
        and format the output based on the Document's width and current font and image sizes.

        Rendering may be incomplete if images are not yet available.
        Use hadLoadingImages() to check for that state.
        In this case, caller needs to wait for ResourceProvider::sig_imageChange,
        and render the document again. */
    class DocumentParser {
     public:
        /** Constructor.
            \param doc [out] Document to produce
            \param reader [in/out] Token stream. First token has not yet been read. */
        DocumentParser(Document& doc, afl::io::xml::BaseReader& reader);

        /** Parse document.
            Call this once.
            Reads the entire token stream if possible. */
        void parseDocument();

        /** Check for loading images.
            \retval true There were images that are still loading. Call again later after ResourceProvider::sig_imageChange.
            \retval false Rendering is complete */
        bool hadLoadingImages() const;

     private:
        void flushLine(util::rich::Text& line);
        void parseBlock(int listLevel);
        void parseBulletList(int listLevel);
        void parseCountedList(int listLevel);
        void parseKeyList(int listLevel);
        void parseDefinitionList(int listLevel);
        void parseListItem(int listLevel);
        void parseImage();
        void parseTable();
        void parseTableLine(std::vector<int>& cellWidths, int align);
        void parseTableCell(std::vector<int>& cellWidths, afl::container::PtrVector<util::rich::Text>& cellText,
                            std::vector<int>& cellAlign, int align);
        void parsePre();

        Document& m_document;
        util::rich::Parser m_parser;
        bool m_hadLoadingImages;
    };

} }

#endif
