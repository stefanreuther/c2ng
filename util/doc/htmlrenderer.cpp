/**
  *  \file util/doc/htmlrenderer.cpp
  *  \brief HTML renderer
  */

#include "util/doc/htmlrenderer.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/visitor.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/doc/renderoptions.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

using afl::io::xml::PINode;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::io::xml::Visitor;
using afl::string::Format;
using util::doc::RenderOptions;
using util::strStartsWith;

namespace {
    class HtmlRenderer : public Visitor {
     public:
        HtmlRenderer(String_t& result, const RenderOptions& opts)
            : m_result(result),
              m_options(opts)
            { }
        virtual void visitPI(const PINode& p);
        virtual void visitText(const TextNode& t);
        virtual void visitTag(const TagNode& t);

     private:
        String_t& m_result;
        const RenderOptions& m_options;

        void copyTag(const String_t& tagName, const TagNode& t);
        void addAttribute(const String_t& attName, const String_t& attValue);
        void renderImage(const TagNode& t);
        void renderInfobox(const TagNode& t);
        void renderLink(const TagNode& t);
        void renderFont(const TagNode& t);
        void renderDefinitionItem(const TagNode& t);
        void renderKeyItem(const TagNode& t);
        void renderKeyTag(const TagNode& t);
        void renderKeys(const String_t& name);
        void renderTable(const TagNode& t);
        void renderTableCell(const String_t& tagName, const String_t& defaultAlign, const TagNode& t);
        void renderPreformatted(const TagNode& t);
    };

    String_t getLinkClass(const TagNode& t)
    {
        const String_t& klass = t.getAttributeByName("class");
        if (klass == "bare") {
            return String_t();
        } else if (!klass.empty()) {
            return klass;
        } else {
            const String_t& s = t.getAttributeByName("href");
            if (strStartsWith(s, "http:") || strStartsWith(s, "https:") || strStartsWith(s, "mailto:") || strStartsWith(s, "ftp:")
                || strStartsWith(s, "news:") || strStartsWith(s, "nntp:") || strStartsWith(s, "data:"))
            {
                return "external-link";
            } else if (strStartsWith(s, "site:")) {
                return "site-link";
            } else {
                return String_t();
            }
        }
    }
}

void
HtmlRenderer::visitPI(const PINode& /*p*/)
{ }

void
HtmlRenderer::visitText(const TextNode& t)
{
    m_result += util::encodeHtml(t.get(), true);
}

void
HtmlRenderer::visitTag(const TagNode& t)
{
    const String_t& tagName = t.getName();
    if (tagName == "a") {
        // Link
        renderLink(t);
    } else if (tagName == "b") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "big") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "br") {
        // Standalone newline
        m_result += "<br />";
    } else if (tagName == "cfg") {
        // Flow-text markup
        // TODO: link to the definition of the configuration item
        copyTag("tt", t);
    } else if (tagName == "dd") {
        // Definition item, long form.
        copyTag(tagName, t);
    } else if (tagName == "di") {
        // Definition item.
        renderDefinitionItem(t);
    } else if (tagName == "dl") {
        // Definition list. Children are <di> which do the formatting, or <dt>/<dd>
        copyTag(tagName, t);
    } else if (tagName == "dt") {
        // Definition item, long form.
        copyTag(tagName, t);
    } else if (tagName == "em") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "font") {
        // Flow-text markup: transform attributes
        renderFont(t);
    } else if (tagName == "h1") {
        // Block markup: copy, including @id
        copyTag("h2", t);
    } else if (tagName == "h2") {
        // Block markup: copy, including @id
        copyTag("h3", t);
    } else if (tagName == "h3") {
        // Block markup: copy, including @id
        copyTag("h4", t);
    } else if (tagName == "img") {
        // Image
        renderImage(t);
    } else if (tagName == "infobox") {
        // Infobox
        renderInfobox(t);
    } else if (tagName == "key" || tagName == "kbd") {
        // Key in flow-text markup
        renderKeyTag(t);
    } else if (tagName == "ki") {
        // Key item
        renderKeyItem(t);
    } else if (tagName == "kl") {
        // Key list, rendered as <li> for now. Children are <ki> which do the formatting.
        copyTag("ul", t);
    } else if (tagName == "li") {
        // List item: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "ol") {
        // List: copy, including @id. Attribute @class=compact not handled for now.
        copyTag(tagName, t);
    } else if (tagName == "p") {
        // Paragraph: copy, including @id
        copyTag(tagName, t);
    } else if (tagName == "pre") {
        // Block markup
        renderPreformatted(t);
    } else if (tagName == "small") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "table") {
        // Block markup: table, with special handling
        renderTable(t);
    } else if (tagName == "td") {
        // Table cell
        renderTableCell(tagName, "", t);
    } else if (tagName == "th") {
        // Table cell
        renderTableCell(tagName, "", t);
    } else if (tagName == "tn") {
        // Table cell
        renderTableCell("td", "right", t);
    } else if (tagName == "tr") {
        // Table
        copyTag(tagName, t);
    } else if (tagName == "tt") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "u") {
        // Flow-text markup: copy [handling of @id not required]
        copyTag(tagName, t);
    } else if (tagName == "ul") {
        // List: copy, including @id. Attributes @class=compact, @bullet not handled for now.
        copyTag(tagName, t);
    } else {
        // Fallback, should not happen, but helps in debugging maybe; can be found by verifier.
        m_result += "<!-- ";
        m_result += tagName;
        m_result += " -->";
    }
}

void
HtmlRenderer::copyTag(const String_t& tagName, const TagNode& t)
{
    m_result += "<";
    m_result += tagName;
    addAttribute("id", t.getAttributeByName("id"));
    m_result += ">";
    visit(t.getChildren());
    m_result += "</";
    m_result += tagName;
    m_result += ">";
}

void
HtmlRenderer::addAttribute(const String_t& attName, const String_t& attValue)
{
    if (!attValue.empty()) {
        m_result += " ";
        m_result += attName;
        m_result += "=\"";
        m_result += util::encodeHtml(attValue, true);
        m_result += "\"";
    }
}

void
HtmlRenderer::renderImage(const TagNode& t)
{
    String_t link = t.getAttributeByName("src");
    if (!link.empty()) {
        int width, height;
        if (afl::string::strToInteger(t.getAttributeByName("width"), width)
            && afl::string::strToInteger(t.getAttributeByName("height"), height))
        {
            int left, top;
            if (afl::string::strToInteger(t.getAttributeByName("left"), left)
                && afl::string::strToInteger(t.getAttributeByName("top"), top))
            {
                // Rendering a slice of an image
                m_result += "<div";
                addAttribute("title", t.getAttributeByName("alt"));
                addAttribute("style",
                             Format("width:%dpx;height:%dpx;background:url(%s);background-position:-%dpx -%dpx")
                             << width << height
                             << m_options.transformLink(link)
                             << left << top);
                m_result += ">&nbsp;</div>";
            } else {
                // Image with given width and height (this will scale the image)
                m_result += "<img";
                addAttribute("src", m_options.transformLink(link));
                addAttribute("width", Format("%d", width));
                addAttribute("height", Format("%d", height));
                addAttribute("alt", t.getAttributeByName("alt"));
                m_result += ">";
            }
        } else {
            // Just an image
            m_result += "<img";
            addAttribute("src", m_options.transformLink(link));
            addAttribute("alt", t.getAttributeByName("alt"));
            m_result += ">";
        }
    }
}

void
HtmlRenderer::renderInfobox(const TagNode& t)
{
    String_t type = t.getAttributeByName("type");
    m_result += "<p";
    addAttribute("id", t.getAttributeByName("id"));
    addAttribute("class", type.empty() ? "infobox" : "infobox-" + type);
    m_result += ">";
    visit(t.getChildren());
    m_result += "</p>";
}

void
HtmlRenderer::renderLink(const TagNode& t)
{
    String_t link = t.getAttributeByName("href");
    if (link.empty()) {
        // Should not happen: link without target
        visit(t.getChildren());
    } else {
        m_result += "<a";
        addAttribute("href", m_options.transformLink(link));
        addAttribute("class", getLinkClass(t));
        m_result += ">";
        visit(t.getChildren());
        m_result += "</a>";
    }
}

void
HtmlRenderer::renderFont(const TagNode& t)
{
    String_t color = t.getAttributeByName("color");
    if (!color.empty()) {
        m_result += "<span class=\"color-";
        m_result += util::encodeHtml(color, true);
        m_result += "\">";
    }
    visit(t.getChildren());
    if (!color.empty()) {
        m_result += "</span>";
    }
}

void
HtmlRenderer::renderDefinitionItem(const TagNode& t)
{
    String_t term = t.getAttributeByName("term");
    if (!term.empty()) {
        m_result += "<dt";
        addAttribute("id", t.getAttributeByName("id"));
        m_result += ">";
        m_result += util::encodeHtml(term, true);
        m_result += "</dt><dd>";
    } else {
        m_result += "<dd";
        addAttribute("id", t.getAttributeByName("id"));
        m_result += ">";
    }
    visit(t.getChildren());
    m_result += "</dd>";
}

void
HtmlRenderer::renderKeyItem(const TagNode& t)
{
    m_result += "<li>";
    String_t keyName = t.getAttributeByName("key");
    if (!keyName.empty()) {
        renderKeys(keyName);
        m_result += ": ";
    }
    visit(t.getChildren());
    m_result += "</li>";
}

void
HtmlRenderer::renderKeyTag(const TagNode& t)
{
    String_t keyName = t.getTextContent();
    if (!keyName.empty()) {
        renderKeys(keyName);
    }
}

void
HtmlRenderer::renderKeys(const String_t& name)
{
    // xref util::rich::Parser::renderKeys
    String_t::size_type p = 0;
    while (p < name.size()) {
        /* Delimiters:
           "-" and "+" for key combinations "Alt-K"
           "/" for alternatives "Up/Down"
           ",.;: " for punctuation "Up, Down" */
        String_t::size_type e = name.find_first_of("-+/,.;: ", p+1);
        if (e == String_t::npos) {
            /* String ends */
            m_result += "<kbd>";
            m_result += util::encodeHtml(String_t(name, p), true);
            m_result += "</kbd>";
            break;
        }

        /* String does not end after key, but may have punctuation */
        m_result += "<kbd>";
        m_result += util::encodeHtml(String_t(name, p, e-p), true);
        m_result += "</kbd>";
        p = e;
        if (name[p] == '.') {
            e = name.find_first_not_of(". ", p+1);
        } else {
            e = name.find_first_not_of(" ", p+1);
        }
        if (e == String_t::npos) {
            /* String ends with punctuation */
            m_result += util::encodeHtml(String_t(name, p), true);
            break;
        }

        /* String does not end after punctuation. As a special case, if it's a
           normal dash, turn that into a (shorter) hyphen. */
        if (e == p+1 && name[p] == '-') {
            m_result += UTF_HYPHEN;
        } else {
            m_result += util::encodeHtml(String_t(name, p, e-p), true);
        }
        p = e;
    }
}

void
HtmlRenderer::renderTable(const TagNode& t)
{
    // Same logic as in pcc2help.xsl for now
    String_t align = t.getAttributeByName("align");
    String_t klass = t.getAttributeByName("class");
    m_result += "<table";
    addAttribute("align", align.empty() ? "center" : align);
    if (klass != "bare") {
        addAttribute("class", klass.empty() ? "normaltable" : klass);
    }
    addAttribute("id", t.getAttributeByName("id"));
    m_result += ">";

    visit(t.getChildren());

    m_result += "</table>";
    if (!align.empty()) {
        m_result += "<div style=\"clear:both;\"></div>";
    }
}

void
HtmlRenderer::renderTableCell(const String_t& tagName, const String_t& defaultAlign, const TagNode& t)
{
    // Same logic as in pcc2help.xsl for now
    String_t align = t.getAttributeByName("align");
    m_result += "<";
    m_result += tagName;
    addAttribute("valign", "top");
    addAttribute("align", align.empty() ? defaultAlign : align);
    addAttribute("colspan", t.getAttributeByName("colspan"));
    addAttribute("rowspan", t.getAttributeByName("rowspan"));
    addAttribute("id", t.getAttributeByName("id"));

    int width;
    if (afl::string::strToInteger(t.getAttributeByName("width"), width)) {
        addAttribute("width", Format("%d", 16*width));
    }
    m_result += ">";

    visit(t.getChildren());

    m_result += "</";
    m_result += tagName;
    m_result += ">";
}

void
HtmlRenderer::renderPreformatted(const TagNode& t)
{
    m_result += "<pre";
    addAttribute("id", t.getAttributeByName("id"));

    const String_t klass = t.getAttributeByName("class");
    if (klass == "bare") {
        // No formatting as code > no CSS for it
    } else if (klass == "formula") {
        // Special handling
        addAttribute("class", "formula");
    } else {
        // TODO: handle @class=ccscript somehow: syntax-highlight if possible
        addAttribute("class", "code");
    }

    m_result += ">";
    visit(t.getChildren());
    m_result += "</pre>";
}

String_t
util::doc::renderHTML(const afl::io::xml::Nodes_t& nodes, const RenderOptions& opts)
{
    String_t result;
    HtmlRenderer(result, opts).visit(nodes);
    return result;
}
