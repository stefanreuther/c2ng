/**
  *  \file ui/rich/documentparser.cpp
  *  \brief Class ui::rich::DocumentParser
  *
  *  FIXME: This is a straight port of the PCC2 code and it uses a
  *  token stream (afl::io::xml::BaseReader) as its interface. Since
  *  we're using DOM (afl::io::xml::Node) to load help pages, it would
  *  make sense porting this to use DOM, to allow random access to
  *  nodes and allow more reliable handling of unknown tags.
  *
  *  PCC2 Comment:
  *
  *  This module contains a function loadHelpPage() and associated
  *  utilities to render (part of) an XML file into a RichDocument.
  *  This is used for displaying help files.
  *
  *  Note that we assume valid, well-formed XML.
  *
  *  \todo When given an input such as
  *     <b>foo</b> <tt>bar</tt>
  *  (with arbitrary space between the tags) it will not render a
  *  space between "foo" and "bar".
  */

#include "ui/rich/documentparser.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/parse.hpp"
#include "ui/rich/document.hpp"
#include "util/numberformatter.hpp"
#include "util/syntax/factory.hpp"
#include "util/syntax/highlighter.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/segment.hpp"
#include "util/unicodechars.hpp"
#include "ui/icons/image.hpp"

using afl::io::xml::BaseReader;

namespace {
    const char*const bullets[] = {
        UTF_SQUARE_BULLET,
        UTF_BULLET,
        "+",
        "-",
        ".",
    };

    bool checkCompact(BaseReader::Token current, const BaseReader& rdr)
    {
        return current == BaseReader::TagAttribute
            && rdr.getName() == "class"
            && rdr.getValue() == "compact";
    }
}

ui::rich::DocumentParser::DocumentParser(Document& doc, afl::io::xml::BaseReader& reader)
    : m_document(doc),
      m_parser(reader),
      m_hadLoadingImages(false)
{ }

void
ui::rich::DocumentParser::parseDocument()
{
    // ex RichHelpRenderer::parseDocument
    /* A document is a list of block elements.
       It might be intermixed with
       - remaining attributes from the opening tag
       - spurious text elements (usually blank) */
    m_parser.readNext();
    while (m_parser.getCurrentToken() != BaseReader::TagEnd
           && m_parser.getCurrentToken() != BaseReader::Eof
           && m_parser.getCurrentToken() != BaseReader::Error)
    {
        if (m_parser.getCurrentToken() == BaseReader::TagStart) {
            parseBlock(0);
        } else {
            m_parser.readNext();
        }
    }
}

bool
ui::rich::DocumentParser::hadLoadingImages() const
{
    return m_hadLoadingImages;
}

// /** Output line to document if present. Clears it afterwards. */
void
ui::rich::DocumentParser::flushLine(util::rich::Text& line)
{
    // ex RichHelpRenderer::flushLine
    if (line.size()) {
        m_document.add(line);
        m_document.addNewline();
        line.clear();
    }
}

// /** Parse a block-level element.
//     - h1/h2/h3 headings
//     - p paragraphs
//     - ul/ol/kl/dl lists

//     Upon call, we're looking at the opening tag.
//     Upon exit, we're looking at the next element after the block element.

//     \param listLevel list nesting level */
void
ui::rich::DocumentParser::parseBlock(int listLevel)
{
    if (m_parser.isOpeningTag("h1")) {
        if (m_document.getDocumentHeight() > 0) {
            m_document.addNewline();
        }
        m_document.add(m_parser.parseText(false)
                       .withStyle(util::rich::StyleAttribute::Big)
                       .withStyle(util::rich::StyleAttribute::Bold));
        m_document.addParagraph();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("h2")) {
        if (m_document.getDocumentHeight() > 0) {
            m_document.addNewline();
        }
        m_document.add(util::rich::Text(util::SkinColor::Link, ">> ").withStyle(util::rich::StyleAttribute::Bold));
        m_document.add(m_parser.parseText(false).withStyle(util::rich::StyleAttribute::Bold));
        m_document.addParagraph();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("h3")) {
        if (m_document.getDocumentHeight() > 0) {
            m_document.addNewline();
        }
        m_document.add(util::rich::Text(util::SkinColor::Blue, ">> ").withStyle(util::rich::StyleAttribute::Bold));
        m_document.add(m_parser.parseText(false).withStyle(util::rich::StyleAttribute::Underline));
        m_document.addParagraph();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("pre")) {
        parsePre();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("p")) {
        /* This is almost a copy of parseText() */
        util::rich::Text line;
        bool haveSpace = true;
        while (1) {
            if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
                // EOF or error stop parsing, or surrounding tag ends
                break;
            } else if (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
                // Spurious attribute from surrounding tag
                m_parser.readNext();
            } else if (m_parser.getCurrentToken() == BaseReader::Text) {
                // Text
                util::rich::Parser::appendText(line, haveSpace, m_parser.reader().getValue());
                m_parser.readNext();
            } else if (m_parser.isOpeningTag("img")) {
                // Image
                m_document.add(line);
                line.clear();
                parseImage();
                m_parser.readNext();
            } else if (m_parser.isOpeningTag("br")) {
                // Newline
                flushLine(line);
                while (m_parser.getCurrentToken() != BaseReader::Eof && m_parser.getCurrentToken() != BaseReader::Error && m_parser.getCurrentToken() != BaseReader::TagEnd) {
                    m_parser.readNext();
                }
                m_parser.readNext();
                haveSpace = true;
            } else {
                line.append(m_parser.parseTextItem(false));
                m_parser.readNext();
                haveSpace = false;
            }
        }
        m_document.add(line);
        m_document.addParagraph();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("ul")) {
        parseBulletList(listLevel);
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("ol")) {
        parseCountedList(listLevel);
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("dl")) {
        parseDefinitionList(listLevel);
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("table")) {
        parseTable();
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("kl")) {
        parseKeyList(listLevel);
        m_parser.readNext();
    } else if (m_parser.isOpeningTag("img")) {
        parseImage();
        m_parser.readNext();
    } else {
        m_parser.skipTag();
    }
}

// /** Parse bullet list.
//     Upon call, we're looking at the first token of the content.
//     Upon exit, we're looking at the closing tag. */
void
ui::rich::DocumentParser::parseBulletList(int listLevel)
{
    // ex RichHelpRenderer::parseBulletList
    bool compact = false;
    while (1) {
        if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
            // EOF or error stop parsing, or surrounding tag ends
            break;
        } else if (checkCompact(m_parser.getCurrentToken(), m_parser.reader())) {
            // "<ul class="compact">"?
            compact = true;
            m_parser.readNext();
        } else if (m_parser.isOpeningTag("li")) {
            // List item
            String_t bullet = bullets[listLevel % countof(bullets)];

            // Parse "<li bullet="...">"
            while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
                if (m_parser.reader().getName() == "bullet") {
                    bullet = m_parser.reader().getValue();
                }
                m_parser.readNext();
            }

            int margin = m_document.getLeftMargin();
            m_document.addAt(margin, bullet);
            m_document.add(" "); /* needed to make sure setLeftMargin does not indent the bullet! */
            m_document.setLeftMargin(margin +
                                     m_document.provider().getFont(gfx::FontRequest())->getTextWidth(String_t(bullet) + " "));
            parseListItem(listLevel+1);
            m_document.setLeftMargin(margin);
            if (listLevel == 0 && !compact) {
                m_document.addNewline();
            }
            m_parser.readNext();
        } else {
            // Unknown, skip it
            m_parser.skipTag();
        }
    }
    if (listLevel == 0 && compact) {
        m_document.addNewline();
    }
}

// /** Parse counted list. */
void
ui::rich::DocumentParser::parseCountedList(int listLevel)
{
    // ex RichHelpRenderer::parseCountedList
    const int em = m_document.provider().getFont(gfx::FontRequest())->getEmWidth() * 3/2;
    int counter = 0;
    bool compact = false;
    while (1) {
        if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (checkCompact(m_parser.getCurrentToken(), m_parser.reader())) {
            /* "<ul class="compact">"? */
            compact = true;
            m_parser.readNext();
        } else if (m_parser.isOpeningTag("li")) {
            /* List item */
            ++counter;
            int margin = m_document.getLeftMargin();
            m_document.addAt(margin, util::NumberFormatter(false, false).formatNumber(counter));
            m_document.add(". ");
            m_document.setLeftMargin(margin + em);
            parseListItem(listLevel+1);
            m_document.setLeftMargin(margin);
            if (listLevel == 0 && !compact) {
                m_document.addNewline();
            }
            m_parser.readNext();
        } else {
            /* Unknown, skip it */
            m_parser.skipTag();
        }
    }
    if (listLevel == 0 && compact) {
        m_document.addNewline();
    }
}

// /** Parse key list. */
void
ui::rich::DocumentParser::parseKeyList(int listLevel)
{
    // ex RichHelpRenderer::parseKeyList
    const int em = m_document.provider().getFont(gfx::FontRequest())->getEmWidth() * 5;
    while (1) {
        if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (m_parser.isOpeningTag("ki")) {
            /* List item */
            String_t name;
            while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
                if (m_parser.reader().getName() == "key") {
                    name = m_parser.reader().getValue();
                }
                m_parser.readNext();
            }
            int margin = m_document.getLeftMargin();
            m_document.addAt(margin, bullets[listLevel % countof(bullets)]);
            m_document.add(" ");
            m_document.add(util::rich::Parser::renderKeys(name));
            m_document.tabTo(margin + em);
            m_document.setLeftMargin(margin + em);
            parseListItem(listLevel+1);
            m_document.setLeftMargin(margin);
            m_parser.readNext();
        } else {
            /* Unknown, skip it */
            m_parser.skipTag();
        }
    }
    if (listLevel == 0) {
        m_document.addNewline();
    }
}

// /** Parse definition list. */
void
ui::rich::DocumentParser::parseDefinitionList(int listLevel)
{
    // RichHelpRenderer::parseDefinitionList
    const int em = m_document.provider().getFont(gfx::FontRequest())->getEmWidth() * 3/2;
    while (1) {
        if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (m_parser.isOpeningTag("di")) {
            /* List item */
            String_t name;
            while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
                if (m_parser.reader().getName() == "term") {
                    name = m_parser.reader().getValue();
                }
                m_parser.readNext();
            }
            int margin = m_document.getLeftMargin();
            m_document.addAt(margin, util::rich::Text(name).withStyle(util::rich::StyleAttribute::Bold));
            m_document.add(" ");
            m_document.setLeftMargin(margin + em);
            parseListItem(listLevel+1);
            m_document.setLeftMargin(margin);
            if (listLevel == 0) {
                m_document.addNewline();
            }
            m_parser.readNext();
        } else {
            /* Unknown, skip it */
            m_parser.skipTag();
        }
    }
}

// /** Parse list item.
//     Upon call, we're looking at the first token of the content.
//     Upon exit, we're looking at the closing tag of the list item. */
void
ui::rich::DocumentParser::parseListItem(int listLevel)
{
    // ex RichHelpRenderer::parseListItem
    util::rich::Text line;
    bool haveSpace = true;
    bool hadImage = false;
    while (1) {
        if (m_parser.getCurrentToken() == BaseReader::Eof || m_parser.getCurrentToken() == BaseReader::Error || m_parser.getCurrentToken() == BaseReader::TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (m_parser.getCurrentToken() == BaseReader::Text) {
            /* Text */
            util::rich::Parser::appendText(line, haveSpace, m_parser.reader().getValue());
            m_parser.readNext();
        } else if (m_parser.isOpeningTag("ul")) {
            /* Sublist */
            flushLine(line);
            parseBulletList(listLevel);
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("ol")) {
            /* Sublist */
            flushLine(line);
            parseCountedList(listLevel);
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("dl")) {
            /* Sublist */
            flushLine(line);
            parseDefinitionList(listLevel);
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("kl")) {
            /* Sublist */
            flushLine(line);
            parseKeyList(listLevel);
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("table")) {
            /* Table */
            flushLine(line);
            parseTable();
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("pre")) {
            /* Preformatted */
            flushLine(line);
            parsePre();
            m_parser.readNext();
            haveSpace = true;
        } else if (m_parser.isOpeningTag("img")) {
            /* Image */
            m_document.add(line);
            line.clear();
            parseImage();
            m_parser.readNext();
            hadImage = true;
        } else if (m_parser.isOpeningTag("br")) {
            /* Newline */
            flushLine(line);
            while (m_parser.getCurrentToken() != BaseReader::Eof && m_parser.getCurrentToken() != BaseReader::Error && m_parser.getCurrentToken() != BaseReader::TagEnd) {
                m_parser.readNext();
            }
            m_parser.readNext();
            haveSpace = true;
        } else {
            /* Formatted text */
            line.append(m_parser.parseTextItem(false));
            m_parser.readNext();
            haveSpace = false;
        }
    }

    if (line.size()) {
        m_document.add(line);
        m_document.addNewline();
    } else if (hadImage) {
        m_document.addNewline();
    }
}

// /** Parse image. Upon entry, the "img" tag has been read. Upon exit,
//     we're looking at the closing tag. */
void
ui::rich::DocumentParser::parseImage()
{
    // ex RichHelpRenderer::parseImage
    /* Parse the tag */
    String_t img;
    String_t align;
    while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
        if (m_parser.reader().getName() == "src") {
            img = m_parser.reader().getValue();
        } else if (m_parser.reader().getName() == "align") {
            align = m_parser.reader().getValue();
        }
        m_parser.readNext();
    }
    while (m_parser.getCurrentToken() != BaseReader::TagEnd && m_parser.getCurrentToken() != BaseReader::Eof) {
        m_parser.readNext();
    }

    // Access the image.
    // @change In c2ng, we need to deal with the image not being available yet
    bool isFinal = false;
    afl::base::Ptr<gfx::Canvas> pix = m_document.provider().getImage(img, &isFinal);
    if (pix.get() != 0) {
        ui::icons::Icon& obj = m_document.deleter().addNew(new ui::icons::Image(*pix));
        if (align == "left") {
            m_document.addFloatObject(obj, true);
        } else if (align == "right") {
            m_document.addFloatObject(obj, false);
        } else {
            m_document.addCenterObject(obj);
        }
    } else {
        if (!isFinal) {
            m_hadLoadingImages = true;
        }
    }
}

// /** Parse table. Upon entry, the "table" tag has been read. Upon exit,
//     we're looking at the closing tag. */
void
ui::rich::DocumentParser::parseTable()
{
    // ex RichHelpRenderer::parseTable
    /* Parse table attributes */
    int align = 1;
    while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
        if (m_parser.reader().getName() == "align") {
            if (m_parser.reader().getValue() == "left") {
                align = 0;
            } else if (m_parser.reader().getValue() == "right") {
                align = 2;
            } else {
                align = 1;
            }
        }
        m_parser.readNext();
    }

    /* Parse table content */
    std::vector<int> cellWidths;
    while (m_parser.getCurrentToken() != BaseReader::TagEnd && m_parser.getCurrentToken() != BaseReader::Eof && m_parser.getCurrentToken() != BaseReader::Error) {
        if (m_parser.isOpeningTag("tr")) {
            parseTableLine(cellWidths, align);
            m_parser.readNext();
        } else {
            m_parser.skipTag();
        }
    }
    m_document.addNewline();
}

void
ui::rich::DocumentParser::parseTableLine(std::vector<int>& cellWidths, int align)
{
    // ex RichHelpRenderer::parseTableLine
    /* Parse */
    afl::container::PtrVector<util::rich::Text> cellText;
    std::vector<int> cellAlign;
    while (m_parser.getCurrentToken() != BaseReader::TagEnd && m_parser.getCurrentToken() != BaseReader::Eof && m_parser.getCurrentToken() != BaseReader::Error) {
        if (m_parser.isOpeningTag("th")) {
            parseTableCell(cellWidths, cellText, cellAlign, 1);
            m_parser.readNext();
        } else if (m_parser.isOpeningTag("td")) {
            parseTableCell(cellWidths, cellText, cellAlign, 0);
            m_parser.readNext();
        } else if (m_parser.isOpeningTag("tn")) {
            parseTableCell(cellWidths, cellText, cellAlign, 2);
            m_parser.readNext();
        } else {
            m_parser.skipTag();
        }
    }

    /* Found anything? */
    if (cellText.empty()) {
        return;
    }

    /* Compute layout */
    const int PADDING = 5;
    int totalWidth = PADDING*(int(cellText.size()) - 1);
    for (size_t i = 0; i < cellText.size(); ++i) {
        totalWidth += cellWidths[i];
    }

    int lm = m_document.getLeftMargin();
    int rm = m_document.getRightMargin();
    int wi = m_document.getPageWidth();
    int room = wi - lm - rm;
    int x;
    if (room < totalWidth) {
        x = lm;
    } else {
        x = lm + align*(room - totalWidth)/2;
    }

    /* Render table */
    for (size_t i = 0; i < cellText.size(); ++i) {
        m_document.setLeftMargin(x);
        m_document.setRightMargin(std::max(0, wi - x - cellWidths[i]));
        if (cellAlign[i] == 0) {
            /* Left */
            m_document.addAt(x, *cellText[i]);
            m_document.addWordSeparator();
        } else if (cellAlign[i] == 1) {
            /* Center */
            m_document.addCentered(x + cellWidths[i]/2, *cellText[i]);
        } else {
            /* Right */
            m_document.addRight(x + cellWidths[i], *cellText[i]);
        }
        x += cellWidths[i];
        x += PADDING;
    }
    m_document.setLeftMargin(lm);
    m_document.setRightMargin(rm);
    m_document.addNewline();
}

void
ui::rich::DocumentParser::parseTableCell(std::vector<int>& cellWidths, afl::container::PtrVector<util::rich::Text>& cellText,
                                         std::vector<int>& cellAlign, int align)
{
    // ex RichHelpRenderer::parseTableCell
    /* Tag attributes */
    int userAlign = align;
    int userWidth = 0;
    while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
        if (m_parser.reader().getName() == "align") {
            if (m_parser.reader().getValue() == "left") {
                userAlign = 0;
            } else if (m_parser.reader().getValue() == "right") {
                userAlign = 2;
            } else {
                userAlign = 1;
            }
        }
        if (m_parser.reader().getName() == "width") {
            if (!afl::string::strToInteger(m_parser.reader().getValue(), userWidth)) {
                userWidth = 0;
            }
        }
        m_parser.readNext();
    }

    /* Text content */
    util::rich::Text thisText(m_parser.parseText(false));

    /* Set cell width */
    if (cellText.size() >= cellWidths.size()) {
        /* We do not yet have a size for this cell. If user gave one, convert
           to ems; otherwise estimate.*/
        afl::base::Ref<gfx::Font> font = m_document.provider().getFont(gfx::FontRequest().addWeight(1));
        if (userWidth != 0) {
            userWidth *= font->getEmWidth();
        } else {
            userWidth = font->getTextWidth(thisText.getText());
        }
        cellWidths.push_back(userWidth);
    }

    /* Remember it. align=1 is <th>, which is implicitly bold. */
    if (align == 1) {
        cellText.pushBackNew(new util::rich::Text(thisText.withStyle(util::rich::StyleAttribute::Bold)));
    } else {
        cellText.pushBackNew(new util::rich::Text(thisText));
    }
    cellAlign.push_back(userAlign);
}

void
ui::rich::DocumentParser::parsePre()
{
    // ex RichHelpRenderer::parsePre
    /* Configure for whitespace preservation */
    BaseReader::WhitespaceMode wsMode = m_parser.reader().getWhitespaceMode();
    m_parser.reader().setWhitespaceMode(BaseReader::AllWS);

    /* Tag attributes */
    String_t kind;
    while (m_parser.getCurrentToken() == BaseReader::TagAttribute) {
        if (m_parser.reader().getName() == "class") {
            kind = m_parser.reader().getValue();
        }
        m_parser.readNext();
    }

    /* Read it */
    util::rich::Text content(m_parser.parseText(true));

    /* Restore whitespace mode */
    m_parser.reader().setWhitespaceMode(wsMode);

    /* Parse it */
    // @change Use util::syntax engine, not ad-hoc parser
    if (!kind.empty()) {
        util::syntax::KeywordTable tab;
        afl::base::Deleter del;
        util::syntax::Highlighter& hi = util::syntax::Factory(tab).create(kind, del);
        String_t rawText = content.getText();
        hi.init(afl::string::toMemory(rawText));

        util::syntax::Segment seg;
        util::rich::Text out;
        while (hi.scan(seg)) {
            // This implements the PCC2 syntax style: comments red, strings green.
            // FIXME: we could implement more styles, e.g. bold keywords.
            switch (seg.getFormat()) {
             case util::syntax::StringFormat:
                out.append(util::SkinColor::Green, afl::string::fromMemory(seg.getText()));
                break;

             case util::syntax::CommentFormat:
             case util::syntax::Comment2Format:
                out.append(util::SkinColor::Red, afl::string::fromMemory(seg.getText()));
                break;

             case util::syntax::DefaultFormat:
             case util::syntax::KeywordFormat:
             case util::syntax::NameFormat:
             case util::syntax::SectionFormat:
             case util::syntax::QuoteFormat:
             case util::syntax::ErrorFormat:
                out.append(afl::string::fromMemory(seg.getText()));
                break;
            }
        }
        out.swap(content);
    }

    const int em = m_document.provider().getFont(gfx::FontRequest())->getEmWidth();
    m_document.setLeftMargin(m_document.getLeftMargin() + em*2);
    m_document.addPreformatted(content.withStyle(util::rich::StyleAttribute::Fixed));
    m_document.addNewline();
    m_document.setLeftMargin(m_document.getLeftMargin() - em*2);
}
