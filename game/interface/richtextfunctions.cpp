/**
  *  \file game/interface/richtextfunctions.cpp
  */

#include <climits>
#include "game/interface/richtextfunctions.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/entityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/string/parse.hpp"
#include "interpreter/values.hpp"
#include "util/charsetfactory.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/skincolor.hpp"

using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::makeSizeValue;
using interpreter::makeStringValue;
using util::rich::StyleAttribute;
using util::SkinColor;
typedef game::interface::RichTextValue::Ptr_t Ptr_t;
typedef game::interface::RichTextValue::Ref_t Ref_t;

namespace {
    enum StyleKind {
        kNone,
        kStyle,
        kColor
    };
    struct Style {
        const char* name;
        StyleKind   kind : 8;
        uint8_t     value;
    };
    const Style styles[] = {
        { "",                  kNone,  0 },
        { "b",                 kStyle, StyleAttribute::Bold }, /* tag name */
        { "background-color",  kColor, SkinColor::Background },
        { "big",               kStyle, StyleAttribute::Big },
        { "blue",              kColor, SkinColor::Blue },
        { "bold",              kStyle, StyleAttribute::Bold }, /* real name */
        { "contrast-color",    kColor, SkinColor::Contrast },
        { "dim",               kColor, SkinColor::Faded },
        { "em",                kStyle, StyleAttribute::Bold }, /* tag name (should actually be italic) */
        { "fixed",             kStyle, StyleAttribute::Fixed }, /* real name */
        { "green",             kColor, SkinColor::Green },
        { "heading-color",     kColor, SkinColor::Heading },
        { "input-color",       kColor, SkinColor::Input },
        { "inverse-color",     kColor, SkinColor::InvStatic },
        // { "italic",            kStyle, StyleAttribute::Italic }, /* not supported yet */
        { "kbd",               kStyle, StyleAttribute::Key }, /* tag name */
        { "key",               kStyle, StyleAttribute::Key }, /* real name */
        { "link-color",        kColor, SkinColor::Link },
        { "link-focus-color",  kColor, SkinColor::LinkFocus },
        { "link-shade-color",  kColor, SkinColor::LinkShade },
        { "none",              kNone,  0 },
        { "red",               kColor, SkinColor::Red },
        { "selection-color",   kColor, SkinColor::Selection },
        { "small"    ,         kStyle, StyleAttribute::Small },
        { "static",            kColor, SkinColor::Static },
        { "tt",                kStyle, StyleAttribute::Fixed }, /* tag name */
        { "u",                 kStyle, StyleAttribute::Underline }, /* tag name */
        { "underline",         kStyle, StyleAttribute::Underline }, /* real name */
        { "white",             kColor, SkinColor::White },
        { "yellow",            kColor, SkinColor::Yellow },
    };

    Ptr_t processStyle(String_t style, Ptr_t text)
    {
        style = afl::string::strTrim(afl::string::strLCase(style));

        const Style* p = 0;
        afl::base::Memory<const Style> pp(styles);
        while (const Style* q = pp.eat()) {
            if (q->name == style) {
                p = q;
                break;
            }
        }

        if (!p) {
            throw interpreter::Error("Invalid style");
        }

        switch (p->kind) {
         case kNone:
            return text;
         case kColor:
         {
             Ptr_t tmp = new util::rich::Text(*text);
             tmp->withNewAttribute(new util::rich::ColorAttribute(SkinColor::Color(p->value)));
             return tmp;
         }
         case kStyle:
         {
             Ptr_t tmp = new util::rich::Text(*text);
             tmp->withNewAttribute(new StyleAttribute(StyleAttribute::Style(p->value)));
             return tmp;
         }
        }

        // Fallback, does not happen
        return text;
    }
}

bool
game::interface::checkRichArg(RichTextValue::Ptr_t& out, const afl::data::Value* value)
{
    // ex int/if/richif.h:checkRichArg
    if (value == 0) {
        return false;
    } else {
        if (const RichTextValue* rv = dynamic_cast<const RichTextValue*>(value)) {
            out = rv->get().asPtr();
        } else {
            out = new util::rich::Text(interpreter::toString(value, false));
        }
        return true;
    }
}

afl::data::Value*
game::interface::IFRAdd(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RAdd(args:RichText...):RichText (Function)
       Concatenate all arguments, which can be strings or rich text, to a new rich text string,
       and returns that.

       If any argument is EMPTY, returns EMPTY.
       If no arguments are given, returns an empty (=zero length) rich text string.

       In text mode, this function produces plain strings instead,
       as rich text attributes have no meaning to the text mode applications.

       @since PCC2 1.99.21, PCC2 2.40 */
    // ex int/if/richif.h:IFRAdd
    if (args.getNumArgs() == 1) {
        // Special case: act as cast-to-rich-text, avoiding a copy
        Ptr_t result;
        if (checkRichArg(result, args.getNext())) {
            return new RichTextValue(*result);
        } else {
            return 0;
        }
    } else {
        // General case
        Ref_t result = *new util::rich::Text();
        while (args.getNumArgs() > 0) {
            Ptr_t tmp;
            if (!checkRichArg(tmp, args.getNext())) {
                return 0;
            }
            *result += *tmp;
        }
        return new RichTextValue(result);
    }
}

afl::data::Value*
game::interface::IFRAlign(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RAlign(str:RichText, width:Int, Optional align:Int):RichText (Function)
       Place rich text in an alignment block.

       Creates a block of the given width, and aligns the text in it.
       For word-wrapping purposes, the block is treated as a single unit.

       This can be used to make simple tables. For example,
       <pre class="ccscript">
         RAdd(RAlign("one", 100, 2), "\n", RAlign("another", 100, 2))
       </pre>
       produces the words "one" and "another", each aligned to the right at 100 pixels, in successive lines.

       <b>Note:</b> This function's interface is still preliminary.

       @since PCC2 2.40.1 */
    args.checkArgumentCount(2, 3);

    // Parse args
    Ptr_t text;
    int32_t width;
    int32_t align = 0;
    if (!checkRichArg(text, args.getNext()) || !checkIntegerArg(width, args.getNext(), 0, 10000)) {
        return 0;
    }
    checkIntegerArg(align, args.getNext(), 0, 2);   // FIXME: preliminary!!!

    // Create result
    Ref_t clone = *new util::rich::Text(*text);
    clone->withNewAttribute(new util::rich::AlignmentAttribute(width, align));
    return new RichTextValue(clone);
}

afl::data::Value*
game::interface::IFRMid(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RMid(str:RichText, first:Int, Optional length:Int):RichText (Function)
       Returns a substring of a rich text string.

       %first specifies the first character position to extract, where 1 means the first.
       %length specifies the number of characters to extract.
       If %length is omitted or EMPTY, the remaining string is extracted.

       If %str or %first are EMPTY, returns EMPTY.

       In text mode, this function deals with plain strings instead,
       as rich text attributes have no meaning to the text mode applications.

       @since PCC2 1.99.21, PCC2 2.40 */
    // ex int/if/richif.h:IFRMid
    args.checkArgumentCount(2, 3);

    // Parse args
    Ptr_t str;
    int32_t iStart, iLength;
    if (!checkRichArg(str, args.getNext()) || !checkIntegerArg(iStart, args.getNext(), 0, INT_MAX)) {
        return 0;
    }
    if (!checkIntegerArg(iLength, args.getNext(), 0, INT_MAX)) {
        iLength = INT_MAX;
    }

    // Convert BASIC indexes to C++ indexes
    // FIXME: this needs some optimisation
    const String_t& text = str->getText();
    afl::charset::Utf8 u8(0);
    size_t nStart  = u8.charToBytePos(text, iStart == 0 ? 0 : static_cast<size_t>(iStart) - 1);
    size_t nLength = u8.charToBytePos(text.substr(nStart), iLength);
    if (nStart > str->size()) {
        return new RichTextValue(*new util::rich::Text());
    } else {
        return new RichTextValue(*new util::rich::Text(str->substr(nStart, nLength)));
    }
}

afl::data::Value*
game::interface::IFRString(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RString(str:RichText):Str (Function)
       Returns the text content of a rich text string,
       i.e. the string with all attributes removed.

       If %str is EMPTY, returns EMPTY.

       In text mode, this function deals with plain strings instead,
       as rich text attributes have no meaning to the text mode applications.

       @since PCC2 1.99.21, PCC2 2.40 */
    // ex int/if/richif.h:IFRString
    args.checkArgumentCount(1);
    Ptr_t str;
    if (checkRichArg(str, args.getNext())) {
        return makeStringValue(str->getText());
    } else {
        return 0;
    }
}

afl::data::Value*
game::interface::IFRLen(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RLen(str:RichText):Int (Function)
       Returns the number of characters in a rich text string.

       If %str is EMPTY, returns EMPTY.

       In text mode, this function deals with plain strings instead,
       as rich text attributes have no meaning to the text mode applications.

       @since PCC2 1.99.21, PCC2 2.40 */
    // ex int/if/richif.h:IFRLen
    args.checkArgumentCount(1);
    Ptr_t str;
    if (checkRichArg(str, args.getNext())) {
        return makeSizeValue(afl::charset::Utf8(0).length(str->getText()));
    } else {
        return 0;
    }
}

afl::data::Value*
game::interface::IFRStyle(game::Session& session, interpreter::Arguments& args)
{
    /* @q RStyle(style:Str, content:RichText...):RichText (Function)
       Attaches a new style to a rich text string.
       Concatenates all %content parameters, and returns a new rich text string with the specified attribute added.

       <pre class="ccscript">
         RStyle("red", "This is ", RStyle("bold", "great"))
       </pre>
       produces "<font color="red">This is <b>great</b></font>".

       If any argument is EMPTY, returns EMPTY.

       In text mode, this function just returns the concatenation of the %content,
       as rich text attributes have no meaning to the text mode applications.

       @todo document the styles
       @since PCC2 1.99.21, PCC2 2.40
       @see RLink, RXml */
    // ex int/if/richif.h:IFRStyle
    // Read style argument
    args.checkArgumentCountAtLeast(1);
    String_t style;
    if (!checkStringArg(style, args.getNext())) {
        return 0;
    }

    // Read remaining arguments, converting them to Rich Text. This is just what IFRAdd does.
    std::auto_ptr<afl::data::Value> tmp(IFRAdd(session, args));
    Ptr_t result;
    if (!checkRichArg(result, tmp.get())) {
        return 0;
    }

    // Process the style
    size_t pos = 0, i;
    while ((i = style.find(',', pos)) != String_t::npos) {
        result = processStyle(style.substr(pos, i-pos), result);
        pos = i+1;
    }
    return new RichTextValue(*processStyle(style.substr(pos), result));
}

afl::data::Value*
game::interface::IFRLink(game::Session& session, interpreter::Arguments& args)
{
    /* @q RLink(target:Str, content:RichText...):RichText (Function)
       Attaches a link to a rich text string.
       Produces a rich text string that contains a link to the specified target,
       and the concatenation of all %content parameters as text.

       If any argument is EMPTY, returns EMPTY.

       In text mode, this function just returns the concatenation of the %content,
       as rich text attributes have no meaning to the text mode applications.

       @since PCC2 1.99.21, PCC2 2.40
       @see RStyle, RXml */

    // Read link argument
    args.checkArgumentCountAtLeast(1);
    String_t link;
    if (!checkStringArg(link, args.getNext())) {
        return 0;
    }

    // Read remaining arguments, converting them to Rich Text. This is just what IFRAdd does.
    std::auto_ptr<afl::data::Value> tmp(IFRAdd(session, args));
    Ptr_t result;
    if (!checkRichArg(result, tmp.get())) {
        return 0;
    }

    // Build a link
    Ref_t clone = *new util::rich::Text(*result);
    clone->withNewAttribute(new util::rich::LinkAttribute(link));
    return new RichTextValue(clone);
}

afl::data::Value*
game::interface::IFRXml(game::Session& /*session*/, interpreter::Arguments& args)
{
    /* @q RXml(xml:Str, args:Str...):RichText (Function)
       Create rich text string from XML.
       Parses the %xml string.
       Tags are converted into rich text attributes.
       Entity references of the form &amp;&lt;digits&gt;; are replaced by the respective element from %args,
       where the first element is &amp;0;.

       For example,
       <pre class="ccscript">
         RXml("&lt;font color='&0;'>This is &lt;b>&1;&lt;/b>&lt;/font>", "red", "great")
       </pre>
       produces <font color="red">This is <b>great</b></font>.

       In text mode, this function uses a simpler XML parser, and returns a plain string,
       as rich text attributes have no meaning to the text mode applications.

       @todo document the styles
       @since PCC2 1.99.21, PCC2 2.40
       @see RStyle, RLink */

    // ex int/if/richif:IFRXml
    class MyEntityHandler : public afl::io::xml::EntityHandler {
     public:
        MyEntityHandler()
            : m_args()
            { }
        void addArgument(afl::data::Value* value)
            { m_args.push_back(value); }
        virtual String_t expandEntityReference(String_t name)
            {
                // ex IntXmlReader::expandEntity
                size_t n;
                if (afl::string::strToInteger(name, n)) {
                    if (n < m_args.size()) {
                        return interpreter::toString(m_args[n], false);
                    } else {
                        return String_t();
                    }
                } else {
                    return afl::io::xml::DefaultEntityHandler().expandEntityReference(name);
                }
            }
     private:
        std::vector<afl::data::Value*> m_args;
    };

    // Read XML text
    args.checkArgumentCountAtLeast(1);
    String_t xml;
    if (!checkStringArg(xml, args.getNext())) {
        return 0;
    }

    // Construct XML reader
    afl::io::ConstMemoryStream ms(afl::string::toBytes(xml));
    MyEntityHandler eh;
    while (args.getNumArgs() > 0) {
        eh.addArgument(args.getNext());
    }
    util::CharsetFactory csf;
    afl::io::xml::Reader rdr(ms, eh, csf);
    util::rich::Parser p(rdr);
    p.readNext();

    // Read
    Ref_t result = *new util::rich::Text(p.parse());
    return new RichTextValue(result);
}
