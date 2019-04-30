/**
  *  \file util/rich/parser.cpp
  *  \brief Class util::rich::Parser
  */

#include "util/rich/parser.hpp"
#include "util/rich/text.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/skincolor.hpp"
#include "util/unicodechars.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/base/countof.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "afl/string/parse.hpp"
#include "afl/base/optional.hpp"
#include "util/charsetfactory.hpp"

namespace {
    struct ColorMap {
        const char* name;
        util::SkinColor::Color color;
    };
    
    /** Parse a color name. */
    util::SkinColor::Color parseColorName(const String_t& name)
    {
        static const ColorMap colors[] = {
            { "static", util::SkinColor::Static },
            { "green",  util::SkinColor::Green },
            { "yellow", util::SkinColor::Yellow },
            { "red",    util::SkinColor::Red },
            { "white",  util::SkinColor::White },
            { "blue",   util::SkinColor::Blue },
            { "dim",    util::SkinColor::Faded },
        };
        for (std::size_t i = 0; i < countof(colors); ++i) {
            if (name == colors[i].name) {
                return colors[i].color;
            }
        }
        return util::SkinColor::Static;
    }
}


// Constructor.
util::rich::Parser::Parser(afl::io::xml::BaseReader& rdr)
    : m_reader(rdr),
      m_currentToken(rdr.Eof)
{
    // ex RichParser::RichParser
}

// Advance to next tag.
void
util::rich::Parser::readNext()
{
    // ex RichParser::next
    m_currentToken = m_reader.readNext();
}

// Check for opening tag.
bool
util::rich::Parser::isOpeningTag(const char* what)
{
    // ex RichParser::isOpen
    if (m_currentToken == m_reader.TagStart && m_reader.getTag() == what) {
        readNext();
        return true;
    } else {
        return false;
    }
}

// Skip a tag.
void
util::rich::Parser::skipTag()
{
    // ex RichParser::parseUnknown
    int nesting = 0;
    do {
        if (m_currentToken == m_reader.TagStart) {
            ++nesting;
        }
        if (m_currentToken == m_reader.TagEnd) {
            --nesting;
        }
        readNext();
    } while (m_currentToken != m_reader.Eof && nesting != 0);
}

// Parse text sequence.
util::rich::Text
util::rich::Parser::parseText(bool keepFormat)
{
    // ex RichParser::parseText
    /* Text consists of a, b, em, tt, key, font, where a and font have parameters. */
    bool haveSpace = true;
    Text result;
    while (1) {
        if (m_currentToken == m_reader.Eof || m_currentToken == m_reader.Error || m_currentToken == m_reader.TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (m_currentToken == m_reader.TagAttribute) {
            /* Spurious attribute from surrounding tag */
            readNext();
        } else if (m_currentToken == m_reader.Text) {
            /* Text */
            if (keepFormat) {
                result.append(m_reader.getValue());
            } else {
                appendText(result, haveSpace, m_reader.getValue());
            }
            readNext();
        } else {
            result.append(parseTextItem(keepFormat));
            readNext();
            haveSpace = false;
        }
    }
    return result;
}

// Parse text element.
util::rich::Text
util::rich::Parser::parseTextItem(bool keepFormat)
{
    // ex RichParser::parseTextItem
    if (isOpeningTag("a")) {
        afl::base::Optional<String_t> target;
        while (m_currentToken == m_reader.TagAttribute) {
            if (m_reader.getName() == "href") {
                target = m_reader.getValue();
            }
            readNext();
        }
        if (const String_t* s = target.get()) {
            return parseText(keepFormat).withNewAttribute(new LinkAttribute(*s));
        } else {
            return parseText(keepFormat);
        }
    } else if (isOpeningTag("b")) {
        return parseText(keepFormat).withStyle(StyleAttribute::Bold);
    } else if (isOpeningTag("em")) {
        // FIXME: availability of fonts shouldn't be an issue here
        // Should be italic, but we don't have an italic font yet.
        // return parseText(keepFormat).withAttribute(RichTextStyleAttribute::Italic);
        return parseText(keepFormat).withStyle(StyleAttribute::Bold);
    } else if (isOpeningTag("u")) {
        return parseText(keepFormat).withStyle(StyleAttribute::Underline);
    } else if (isOpeningTag("tt")) {
        return parseText(keepFormat).withStyle(StyleAttribute::Fixed);
    } else if (isOpeningTag("kbd") || isOpeningTag("key")) {
        return renderKeys(parseText(keepFormat).getText());
    } else if (isOpeningTag("big")) {
        return parseText(keepFormat).withStyle(StyleAttribute::Big);
    } else if (isOpeningTag("small")) {
        return parseText(keepFormat).withStyle(StyleAttribute::Small);
    } else if (isOpeningTag("font")) {
        afl::base::Optional<SkinColor::Color> c;
        while (m_currentToken == m_reader.TagAttribute) {
            if (m_reader.getName() == "color") {
                c = parseColorName(m_reader.getValue());
            }
            readNext();
        }
        if (const SkinColor::Color* pc = c.get()) {
            return parseText(keepFormat).withColor(*pc);
        } else {
            return parseText(keepFormat);
        }
    } else if (isOpeningTag("align")) {
        int width = 0;
        int align = 0;
        while (m_currentToken == m_reader.TagAttribute) {
            if (m_reader.getName() == "width") {
                int n;
                if (afl::string::strToInteger(m_reader.getValue(), n) && n > 0) {
                    width = n;
                }
            } else if (m_reader.getName() == "align") {
                if (m_reader.getValue() == "left") {
                    align = 0;
                } else if (m_reader.getValue() == "center") {
                    align = 1;
                } else if (m_reader.getValue() == "right") {
                    align = 2;
                } else {
                    // invalid
                }
            } else {
                // unknown attribute
            }
            readNext();
        }
        return parseText(keepFormat).withNewAttribute(new AlignmentAttribute(width, align));
    } else if (m_currentToken == m_reader.TagStart) {
        /* An opening tag we don't understand; parse its content */
        readNext();
        return parseText(keepFormat);
    } else {
        /* Completely unknown, skip it */
        readNext();
        return Text();
    }
}

// Parse text.
util::rich::Text
util::rich::Parser::parse()
{
    // ex RichParser::parse
    // FIXME: like the <p> handling, this is almost a copy of parseText.
    bool haveSpace = true;
    Text result;
    while (1) {
        if (m_currentToken == m_reader.Eof || m_currentToken == m_reader.Error || m_currentToken == m_reader.TagEnd) {
            /* EOF or error stop parsing, or surrounding tag ends */
            break;
        } else if (m_currentToken == m_reader.TagAttribute) {
            /* Spurious attribute from surrounding tag */
            readNext();
        } else if (m_currentToken == m_reader.Text) {
            /* Text */
            appendText(result, haveSpace, m_reader.getValue());
            readNext();
        } else if (isOpeningTag("br")) {
            /* Newline */
            result.append("\n\n");
            while (m_currentToken != m_reader.Eof && m_currentToken != m_reader.Error && m_currentToken != m_reader.TagEnd) {
                readNext();
            }
            readNext();
            haveSpace = true;
        } else {
            result.append(parseTextItem(false));
            readNext();
            haveSpace = false;
        }
    }
    return result;
}

// Access reader.
afl::io::xml::BaseReader&
util::rich::Parser::reader()
{
    return m_reader;
}

// Get current token.
afl::io::xml::BaseReader::Token
util::rich::Parser::getCurrentToken() const
{
    return m_currentToken;
}

// Append string to Text.
void
util::rich::Parser::appendText(Text& out, bool& haveSpace, const String_t& in)
{
    // ex RichParser::appendText
    static const char SPACE[] = " \r\n\t";
    String_t::size_type a = 0;
    while (a < in.size()) {
        if (haveSpace) {
            /* out ends with a space. Skip all space in input. */
            a = in.find_first_not_of(SPACE, a);
            haveSpace = false;
        } else {
            /* out does not end with a space, so look for spaces in /in/. */
            String_t::size_type e = in.find_first_of(SPACE, a);
            if (e == in.npos) {
                out.append(String_t(in, a));
            } else {
                out.append(String_t(in, a, e-a));
                out.append(" ");
                haveSpace = true;
            }
            a = e;
        }
    }
}

// Render keys.
util::rich::Text
util::rich::Parser::renderKeys(const String_t& name)
{
    // ex RichParser::renderKeys
    Text result;
    String_t::size_type p = 0;
    while (p < name.size()) {
        /* Delimiters:
           "-" and "+" for key combinations "Alt-K"
           "/" for alternatives "Up/Down"
           ",.;: " for punctuation "Up, Down" */
        String_t::size_type e = name.find_first_of("-+/,.;: ", p+1);
        if (e == String_t::npos) {
            /* String ends */
            result.append(Text(String_t(name, p)).withStyle(StyleAttribute::Key));
            break;
        }

        /* String does not end after key, but may have punctuation */
        result.append(Text(String_t(name, p, e-p)).withStyle(StyleAttribute::Key));
        p = e;
        if (name[p] == '.') {
            e = name.find_first_not_of(". ", p+1);
        } else {
            e = name.find_first_not_of(" ", p+1);
        }
        if (e == String_t::npos) {
            /* String ends with punctuation */
            result.append(String_t(name, p));
            break;
        }

        /* String does not end after punctuation. As a special case, if it's a
           normal dash, turn that into a (shorter) hyphen. */
        if (e == p+1 && name[p] == '-') {
            result.append(UTF_HYPHEN);
        } else {
            result.append(String_t(name, p, e-p));
        }
        p = e;
    }

    return result;
}

// Parse string.
util::rich::Text
util::rich::Parser::parseXml(String_t source)
{
    // ex RichParser::textFromXml
    // ex parseRichText(?) - difference is that that guy does p.parse(), which allows <br>
    CharsetFactory csf;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(source));
    afl::io::xml::Reader rdr(ms, afl::io::xml::DefaultEntityHandler::getInstance(), csf);
    Parser p(rdr);
    p.readNext();
    return p.parseText(true);
}
