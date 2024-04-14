/**
  *  \file ui/rich/document.cpp
  *  \brief Class ui::rich::Document
  */

#include <climits>
#include <memory>
#include "ui/rich/document.hpp"
#include "afl/base/staticassert.hpp"
#include "gfx/complex.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/visitor.hpp"
#include "util/skincolor.hpp"

namespace {
    void drawKeycap(gfx::BaseContext& ctx, int x, int y, int he, int wi)
    {
        /* Adjust to leave some inter-line space */
        he -= he/8;

        /* Main frame */
        // FIXME: original code distinguishes between bitness and references main palette
        // uint32_t oldColor = ctx.getRawColor();
        // if (ui_screen_bits > 8) {
        //     ctx.setAlpha(128);
        // } else {
        //     ctx.setRawColor(standard_colors[COLOR_DARK]);
        // }
        ctx.setAlpha(128);

        drawHLine(ctx, x+2,    y,      x+wi-3);
        drawHLine(ctx, x+2,    y+he-1, x+wi-3);
        drawVLine(ctx, x,      y+2,    y+he-3);
        drawVLine(ctx, x+wi-1, y+2,    y+he-3);
        drawPixel(ctx, gfx::Point(x+1, y+1));
        drawPixel(ctx, gfx::Point(x+1, y+he-2));
        drawPixel(ctx, gfx::Point(x+wi-2, y+1));
        drawPixel(ctx, gfx::Point(x+wi-2, y+he-2));
        // ctx.setRawColor(oldColor);
        ctx.setAlpha(gfx::OPAQUE_ALPHA);

        /* Shadow */
        drawHLine(ctx, x+3,  y+he, x+wi-2);
        drawVLine(ctx, x+wi, y+3,  y+he-2);
        drawPixel(ctx, gfx::Point(x+wi-2, y+he-1));
        drawPixel(ctx, gfx::Point(x+wi-1, y+he-1));
        drawPixel(ctx, gfx::Point(x+wi-1, y+he-2));
    }
}


ui::rich::Document::BlockItem::BlockItem(Kind k, ui::icons::Icon& obj)
    : kind(k),
      pos(gfx::Point(), obj.getSize()),
      obj(obj)
{ }


/** Splits up RichText into individual per-chunk attributes,
    and generates appropriate RichText::Item's. */
class ui::rich::Document::Splitter : public util::rich::Visitor {
 public:
    Splitter(afl::container::PtrVector<Item>& items, gfx::ResourceProvider& provider);
    bool handleText(String_t text);
    bool startAttribute(const util::rich::Attribute& att);
    bool endAttribute(const util::rich::Attribute& att);

 private:
    afl::container::PtrVector<Item>& items;
    gfx::ResourceProvider& m_provider;
    int nbold;
    int nbig;
    int nfixed;
    int nunder;
    int nlink;
    int nkey;
    std::vector<util::SkinColor::Color> colors;
    std::vector<size_t> m_alignmentSlots;
    std::vector<int> m_alignmentWidths;

    void process(const util::rich::Attribute& att, int delta);
};

ui::rich::Document::Splitter::Splitter(afl::container::PtrVector<Item>& items, gfx::ResourceProvider& provider)
    : items(items),
      m_provider(provider),
      nbold(0), nbig(0), nfixed(0), nunder(0), nlink(0), nkey(0)
{
    // ex RichTextSplitter::RichTextSplitter
}

bool
ui::rich::Document::Splitter::handleText(String_t text)
{
    // ex RichTextSplitter::handleText
    gfx::FontRequest req;
    req.setSize(int16_t(nbig));
    req.setWeight(int16_t(nbold));
    req.setStyle(nfixed > 0 ? 1 : 0);

    util::SkinColor::Color color = !colors.empty() ? colors.back() : util::SkinColor::Static;
    int width = m_provider.getFont(req)->getTextWidth(text);
    if (nkey != 0) {
        width = std::max(width + 7, m_provider.getFont(req)->getTextHeight(text));
    }
    bool addIt;
    bool breakable;
    if (!m_alignmentWidths.empty()) {
        int& remain = m_alignmentWidths.back();
        if (remain == 0) {
            // No room for this one
            addIt = false;
            breakable = false;
        } else {
            // There is some room; reduce room.
            remain -= std::min(remain, width);
            addIt = true;
            breakable = false;
        }
    } else {
        addIt = true;
        breakable = true;
    }

    if (addIt) {
        items.pushBackNew(new Item(nlink ? Link : Normal, 0, 0,
                                   width,
                                   req, color,
                                   nunder > 0,
                                   nkey > 0,
                                   text,
                                   breakable));
    }
    return true;
}

bool
ui::rich::Document::Splitter::startAttribute(const util::rich::Attribute& att)
{
    // ex RichTextSplitter::startAttribute
    process(att, +1);
    return true;
}

bool
ui::rich::Document::Splitter::endAttribute(const util::rich::Attribute& att)
{
    // ex RichTextSplitter::endAttribute
    process(att, -1);
    return true;
}

void
ui::rich::Document::Splitter::process(const util::rich::Attribute& att, int delta)
{
    // ex RichTextSplitter::process
    if (const util::rich::StyleAttribute* sa = dynamic_cast<const util::rich::StyleAttribute*>(&att)) {
        switch (sa->getStyle()) {
         case util::rich::StyleAttribute::Bold:
            nbold += delta;
            break;
         case util::rich::StyleAttribute::Big:
            nbig += delta;
            break;
         case util::rich::StyleAttribute::Small:
            nbig -= delta;
            break;
         case util::rich::StyleAttribute::Fixed:
            nfixed += delta;
            break;
         case util::rich::StyleAttribute::Underline:
            nunder += delta;
            break;
         case util::rich::StyleAttribute::Key:
            nkey += delta;
            break;
         default:
            break;
        }
    } else if (const util::rich::LinkAttribute* la = dynamic_cast<const util::rich::LinkAttribute*>(&att)) {
        if (delta > 0) {
            items.pushBackNew(new Item(LinkTarget, 0, 0, 0, gfx::FontRequest(),
                                       util::SkinColor::Static, false, false, la->getTarget(), false));
        }
        nlink += delta;
    } else if (const util::rich::ColorAttribute* ca = dynamic_cast<const util::rich::ColorAttribute*>(&att)) {
        if (delta > 0)
            colors.push_back(ca->getColor());
        else
            colors.pop_back();
    } else if (const util::rich::AlignmentAttribute* aa = dynamic_cast<const util::rich::AlignmentAttribute*>(&att)) {
        if (delta > 0) {
            m_alignmentWidths.push_back(aa->getWidth());
            m_alignmentSlots.push_back(items.size());
        } else {
            if (!m_alignmentSlots.empty() && !m_alignmentWidths.empty()) {
                int remainingWidth = m_alignmentWidths.back();
                size_t startSlot = m_alignmentSlots.back();
                if (startSlot < items.size()) {
                    int leftWidth = remainingWidth * aa->getAlignment() / 2;
                    int rightWidth = remainingWidth - leftWidth;
                    items.insertNew(items.begin() + startSlot, new Item(Normal, 0, 0, leftWidth, gfx::FontRequest(), util::SkinColor::Static, false, false, String_t(), false));
                    items.pushBackNew(new Item(Normal, 0, 0, rightWidth, gfx::FontRequest(), util::SkinColor::Static, false, false, String_t(), false));
                } else {
                    items.pushBackNew(new Item(Normal, 0, 0, remainingWidth, gfx::FontRequest(), util::SkinColor::Static, false, false, String_t(), false));
                }
            }
            if (!m_alignmentSlots.empty()) {
                m_alignmentSlots.pop_back();
            }
            if (!m_alignmentWidths.empty()) {
                m_alignmentWidths.pop_back();
            }
        }
    } else {
        // ignore unknown attribute
    }
}


const ui::rich::Document::LinkId_t ui::rich::Document::nil;

// Default constructor.
ui::rich::Document::Document(gfx::ResourceProvider& provider)
    : m_provider(provider),
      m_deleter(),
      content(),
      last_chunk(),
      block_objs(),
      m_renderOptions(),
      x(0),
      y(0),
      first_this_line(0),
      page_width(100),
      left_margin(0),
      right_margin(0)
{
    // ex RichDocument::RichDocument
    bo_index[0] = bo_index[1] = 0;
    bo_width[0] = bo_width[1] = 0;
    bo_height[0] = bo_height[1] = 0;
}

ui::rich::Document::~Document()
{ }

gfx::ResourceProvider&
ui::rich::Document::provider()
{
    return m_provider;
}

afl::base::Deleter&
ui::rich::Document::deleter()
{
    return m_deleter;
}

// Clear this document.
void
ui::rich::Document::clear()
{
    // ex RichDocument::clear
    content.clear();
    last_chunk.clear();
    block_objs.clear();
    x = y = 0;
    first_this_line = 0,
    left_margin = right_margin = 0;
    bo_index[0] = bo_index[1] = 0;
    bo_width[0] = bo_width[1] = 0;
    bo_height[0] = bo_height[1] = 0;
    m_deleter.clear();
}

// Set this document's page width.
void
ui::rich::Document::setPageWidth(int width)
{
    // ex RichDocument::setPageWidth
    page_width = width;
}

// Set this document's left margin.
void
ui::rich::Document::setLeftMargin(int lm)
{
    // ex RichDocument::setLeftMargin
    if (x == left_margin + bo_width[BlockItem::Left])
        x = lm + bo_width[BlockItem::Left];
    left_margin = lm;
}

// Set this document's right margin.
void
ui::rich::Document::setRightMargin(int rm)
{
    // ex RichDocument::setRightMargin
    right_margin = rm;
}

// Get page width.
int
ui::rich::Document::getPageWidth() const
{
    // ex RichDocument::getPageWidth
    return page_width;
}

// Get this document's current left margin.
int
ui::rich::Document::getLeftMargin() const
{
    // ex RichDocument::getLeftMargin
    return left_margin;
}

// Get this document's current right margin.
int
ui::rich::Document::getRightMargin() const
{
    // ex RichDocument::getRightMargin
    return right_margin;
}

// Add rich text.
void
ui::rich::Document::add(const util::rich::Text& text)
{
    // ex RichDocument::add
    String_t::size_type start = 0;
    String_t::size_type end;
    while ((end = text.find('\n', start)) != String_t::npos) {
        Splitter(last_chunk, m_provider).visit(util::rich::Text(text, start, end-start));
        process();
        addNewline();
        start = end+1;
    }

    if (start) {
        Splitter(last_chunk, m_provider).visit(util::rich::Text(text, start));
    } else {
        Splitter(last_chunk, m_provider).visit(text);
    }
    process();
}

// Add plain text.
void
ui::rich::Document::add(const String_t& text)
{
    // ex RichDocument::add
    // This can be done more efficient, but this way it's simple:
    add(util::rich::Text(text));
}

// Add plain text.
void
ui::rich::Document::add(const char* text)
{
    // ex RichDocument::add
    // This can be done more efficient, but this way it's simple:
    add(util::rich::Text(text));
}

// Add zero-width word separator.
void
ui::rich::Document::addWordSeparator()
{
    // ex RichDocument::addWordSeparator
    flushWord();
}

// Add newline.
void
ui::rich::Document::addNewline()
{
    // ex RichDocument::addNewline
    flushWord();
    if (first_this_line == content.size()) {
        /* This line is empty, so just add some space */
        afl::base::Ref<gfx::Font> font = m_provider.getFont(gfx::FontRequest());
        int lineHeight = font->getLineHeight();
        if (m_renderOptions.contains(FullLinesBetweenParagraphs)) {
            addY(lineHeight);
        } else {
            addY(lineHeight/2);
        }
    } else {
        /* Regular newline */
        flushLine();
    }
}

// Add new paragraph.
void
ui::rich::Document::addParagraph()
{
    // ex RichDocument::addParagraph
    addNewline();
    addNewline();
}

// Add text at horizontal position.
void
ui::rich::Document::addAt(int x, const util::rich::Text& text)
{
    // ex RichDocument::addAt
    tabTo(x);
    add(text);
}

// Add right-justified column text.
void
ui::rich::Document::addRight(int x, const util::rich::Text& text)
{
    // ex RichDocument::addRight
    /* Write pending previous output */
    flushWord();

    /* Set right margin to our target position */
    int oldRightMargin = right_margin;
    right_margin = page_width - x - bo_width[BlockItem::Right];

    /* Write text, and remember where it ended up */
    size_t start = content.size();
    add(text);
    flushWord();
    size_t end = content.size();

    /* Restore old margin */
    right_margin = oldRightMargin;

    /* If we have produced some output, and that's left of our X, move it right */
    if (end > start && content[end-1]->x + content[end-1]->w < x) {
        int delta = content[end-1]->x + content[end-1]->w - x;
        while (end > start) {
            --end;
            content[end]->x -= delta;
        }
        this->x = x;
    }
}

// Add centered text.
void
ui::rich::Document::addCentered(int x, const util::rich::Text& text)
{
    // ex RichDocument::addCentered
    /* Write pending previous output */
    flushWord();

    /* Write text, and remember where it ended up */
    size_t start = content.size();
    int startX = this->x;
    add(text);
    flushWord();
    size_t end = content.size();
    int endX = this->x;

    /* If we have produced some output, adjust its position */
    if (end > start && endX > startX && endX < 2*x - startX) {
        int width = endX - startX;
        int delta = x - startX - (width / 2);
        while (end > start) {
            --end;
            content[end]->x += delta;
        }
        this->x += delta;
    }
}

// Add preformatted text.
void
ui::rich::Document::addPreformatted(const util::rich::Text& text)
{
    // ex RichDocument::addPreformatted
    /* Write pending previous output */
    tabTo(left_margin);

    /* Set page width to infinity */
    int oldWidth = page_width;
    int oldRightMargin = right_margin;
    page_width = INT_MAX;
    right_margin = 0;

    /* Add output */
    add(text);

    /* Restore margins */
    page_width = oldWidth;
    right_margin = oldRightMargin;
    tabTo(left_margin);
}

// Add floating object.
void
ui::rich::Document::addFloatObject(ui::icons::Icon& obj, bool left)
{
    // ex RichDocument::addFloatObject
    static_assert(int(true)  == int(BlockItem::Left), "left");
    static_assert(int(false) == int(BlockItem::Right), "right");

    /* Store the object */
    block_objs.pushBackNew(new BlockItem(left ? BlockItem::Left : BlockItem::Right, obj));

    /* When we're at the beginning of the line, try starting the object immediately */
    if (bo_height[left] == 0 && last_chunk.size() == 0 && x == left_margin + bo_width[BlockItem::Left]) {
        bo_index[left] = block_objs.size()-1;
        startNextObject(left);
        x = left_margin + bo_width[BlockItem::Left];
    }
}

// Add centered object.
void
ui::rich::Document::addCenterObject(ui::icons::Icon& obj)
{
    // ex RichDocument::addCenterObject
    /* Finish current line */
    flushWord();
    flushLine();

    /* Place the object */
    std::auto_ptr<BlockItem> p(new BlockItem(BlockItem::Center, obj));
    p->pos = gfx::Rectangle((page_width - p->pos.getWidth()) / 2,
                            this->y,
                            p->pos.getWidth(),
                            p->pos.getHeight());
    addY(p->pos.getHeight());
    block_objs.pushBackNew(p.release());
}

// Move to horizontal position.
void
ui::rich::Document::tabTo(int x)
{
    // ex RichDocument::tabTo
    x += bo_width[BlockItem::Left];
    flushWord();
    if (this->x > x && first_this_line != content.size()) {
        /* This line already has content, and that is beyond x */
        flushLine();
    }
    this->x = x;
}

// Finish this document.
void
ui::rich::Document::finish()
{
    // ex RichDocument::finish
    /* Finish text */
    flushWord();
    flushLine();

    /* Finish all floats */
    while (1) {
        if (bo_height[0] > 0) {
            addY(bo_height[0]);
        } else if (bo_height[1] > 0) {
            addY(bo_height[1]);
        } else {
            break;
        }
    }
}

// Get height of document.
int
ui::rich::Document::getDocumentHeight() const
{
    // ex RichDocument::getDocumentHeight
    return y;
}

// Get document width.
int
ui::rich::Document::getDocumentWidth() const
{
    // ex RichDocument::getDocumentWidth
    int width = 0;
    for (std::size_t i = 0; i < content.size(); ++i) {
        width = std::max(width, content[i]->w + content[i]->x);
    }
    for (std::size_t i = 0; i < block_objs.size(); ++i) {
        width = std::max(width, block_objs[i]->pos.getLeftX() + block_objs[i]->pos.getWidth());
    }
    return width;
}

// Draw document.
void
ui::rich::Document::draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, int skipY)
{
    // ex RichDocument::draw
    for (size_t i = 0; i < content.size(); ++i) {
        Item& it = *content[i];
        if (it.kind != LinkTarget) {
            afl::base::Ref<gfx::Font> font = m_provider.getFont(it.font);
            gfx::Rectangle pos(it.x + area.getLeftX(),
                               it.y + area.getTopY() - skipY,
                               it.w,
                               font->getLineHeight());
            if (pos.isIntersecting(area)) {
                ctx.useFont(*font);
                if (it.kind & Link) {
                    pos.setHeight(pos.getHeight() - 1);
                    if (it.kind == LinkFocus) {
                        drawSolidBar(ctx, pos, util::SkinColor::LinkFocus);
                    } else if (it.kind == LinkHover) {
                        drawSolidBar(ctx, pos, util::SkinColor::LinkShade);
                    } else {
                        // No shade
                    }
                    ctx.setColor(util::SkinColor::Link);
                    ctx.setLinePattern(uint8_t(0x55 << (pos.getLeftX()&1)));
                    drawHLine(ctx, pos.getLeftX(), pos.getTopY() + font->getLineHeight()*17/20, pos.getRightX()-1);
                    ctx.setLinePattern(gfx::SOLID_LINE);
                    if (it.color != util::SkinColor::Static) {
                        ctx.setColor(it.color);
                    }
                    outText(ctx, pos.getTopLeft(), it.text);
                } else {
                    ctx.setColor(it.color);
                    if (it.key) {
                        drawKeycap(ctx, pos.getLeftX()+1, pos.getTopY(), font->getLineHeight(), pos.getWidth()-3);
                        outText(ctx, gfx::Point(pos.getLeftX() + (pos.getWidth() - font->getTextWidth(it.text))/2, pos.getTopY()), it.text);
                    } else {
                        if (it.underline) {
                            drawHLine(ctx, pos.getLeftX(), pos.getTopY() + font->getLineHeight()*17/20, pos.getRightX()-1);
                        }
                        outText(ctx, pos.getTopLeft(), it.text);
                    }
                }
            }
        }
    }
    for (std::size_t i = 0; i < block_objs.size(); ++i) {
        BlockItem& obj = *block_objs[i];
        gfx::Rectangle pos(obj.pos);
        pos.moveBy(gfx::Point(area.getLeftX(), area.getTopY() - skipY));
        if (pos.isIntersecting(area)) {
            obj.obj.draw(ctx, pos, ButtonFlags_t());
        }
    }
}

// Get link, given a position.
ui::rich::Document::LinkId_t
ui::rich::Document::getLinkFromPos(gfx::Point pt) const
{
    // ex RichDocument::getLinkFromPos
    for (size_t i = 0; i < content.size(); ++i) {
        afl::base::Ref<gfx::Font> font = m_provider.getFont(content[i]->font);
        if ((content[i]->kind & Link) != 0
            && gfx::Rectangle(content[i]->x, content[i]->y, content[i]->w, font->getLineHeight()).contains(pt))
        {
            while (i > 0 && content[i]->kind != LinkTarget) {
                --i;
            }
            return i;
        }
    }
    return nil;
}

// Get link target for a link.
String_t
ui::rich::Document::getLinkTarget(LinkId_t link) const
{
    // ex RichDocument::getLinkTarget
    return (link < content.size()
            ? content[link]->text
            : String_t());
}

// Change kind (=status) of a link.
void
ui::rich::Document::setLinkKind(LinkId_t link, ItemKind kind)
{
    // ex RichDocument::setLinkKind
    while (++link < content.size() && (content[link]->kind & Link) != 0) {
        content[link]->kind = kind;
    }
}

// Get next link.
ui::rich::Document::LinkId_t
ui::rich::Document::getNextLink(LinkId_t id) const
{
    // ex RichDocument::getNextLink
    return getNextLink(id, gfx::Rectangle(0, 0, INT_MAX, INT_MAX));
}

// Get next link within range.
ui::rich::Document::LinkId_t
ui::rich::Document::getNextLink(LinkId_t id, gfx::Rectangle limit) const
{
    // ex RichDocument::getNextLink
    LinkId_t pos = (id == nil ? 0 : id+1);
    while (pos < content.size()) {
        if (content[pos]->kind == LinkTarget && isLinkVisible(pos, limit)) {
            return pos;
        }
        ++pos;
    }
    return nil;
}

// Get previous link.
ui::rich::Document::LinkId_t
ui::rich::Document::getPreviousLink(LinkId_t id) const
{
    // ex RichDocument::getPreviousLink
    return getPreviousLink(id, gfx::Rectangle(0, 0, INT_MAX, INT_MAX));
}

// Get previous link within range.
ui::rich::Document::LinkId_t
ui::rich::Document::getPreviousLink(LinkId_t id, gfx::Rectangle limit) const
{
    // ex RichDocument::getPreviousLink
    LinkId_t pos = (id == nil ? content.size() : id);
    while (pos > 0) {
        --pos;
        if (content[pos]->kind == LinkTarget && isLinkVisible(pos, limit)) {
            return pos;
        }
    }
    return nil;
}

// Check whether a link is visible.
bool
ui::rich::Document::isLinkVisible(LinkId_t id, gfx::Rectangle limit) const
{
    // ex RichDocument::isLinkVisible
    while (++id < content.size() && (content[id]->kind & Link) != 0) {
        afl::base::Ref<gfx::Font> font = m_provider.getFont(content[id]->font);
        if (gfx::Rectangle(content[id]->x, content[id]->y,
                           content[id]->w, font->getLineHeight()).isIntersecting(limit))
        {
            return true;
        }
    }
    return false;
}

// Set rendering options.
void
ui::rich::Document::setRenderOptions(Flags_t opts)
{
    // ex RichDocument::setRenderOptions
    m_renderOptions = opts;
}

// Get rendering options.
ui::rich::Document::Flags_t
ui::rich::Document::getRenderOptions() const
{
    // ex RichDocument::getRenderOptions
    return m_renderOptions;
}

/** Process pending input. */
void
ui::rich::Document::process()
{
    // ex RichDocument::process
    /* Word wrap. Can this be written more elegantly? */
    size_t i = 0;
    size_t lastWithSpace = 0;
    int addX = x;
    while (i < last_chunk.size()) {
        int maxX = page_width - right_margin - bo_width[BlockItem::Right];
        addX += last_chunk[i++]->w;
        if (addX > maxX) {
            /* Overflow! Try to find a break point in any chunk here. */
            size_t breakItem, breakChar;
            int remX = addX;
            for (breakItem = i; breakItem > 0; --breakItem) {
                Item& it = *last_chunk[breakItem-1];
                remX -= it.w;
                afl::base::Ref<gfx::Font> font = m_provider.getFont(it.font);
                if (it.breakable && it.w != 0) {
                    for (breakChar = it.text.size(); breakChar > 0; --breakChar) {
                        if (it.text[breakChar-1] == ' ' && remX + font->getTextWidth(it.text.substr(0, breakChar-1)) <= maxX) {
                            /* Found a break point */
                            goto out;
                        }
                    }
                }
            }
         out:
            if (breakItem != 0) {
                /* Means: we found a possible break point.
                   Items [0, breakItem-2] can be copied as is.
                   Item breakItem-1 must be split at breakChar-2. */
                flushItems(breakItem-1);
                last_chunk.erase(last_chunk.begin(), last_chunk.begin() + breakItem-1);

                /* last_chunk[0] now is what originally was breakItem-1 */
                Item& it = *last_chunk[0];
                afl::base::Ref<gfx::Font> font = m_provider.getFont(it.font);
                content.pushBackNew(new Item(it.kind, x, 0, 42, it.font, it.color, it.underline, it.key, it.text.substr(0, breakChar-1), true));
                content.back()->w = font->getTextWidth(content.back()->text);
                flushLine();

                it.text.erase(0, breakChar);
                it.w = font->getTextWidth(it.text);
                i = 0;
                addX = x;
                lastWithSpace = 0;
            } else {
                /* No break point found. If we are at the beginning of a line,
                   give up: copy the line (or its initial part) even though it does not fit.
                   Otherwise, move to a new line and retry. */
                if (x == left_margin + bo_width[BlockItem::Left]) {
                    Item& it = *last_chunk[i-1];
                    String_t::size_type pos = it.text.find(' ');
                    if (pos != String_t::npos) {
                        /* ok, we can split that chunk. Do so. */
                        afl::base::Ref<gfx::Font> font = m_provider.getFont(it.font);
                        content.pushBackNew(new Item(it.kind, x, 0, 42, it.font, it.color, it.underline, it.key, it.text.substr(0, pos), true));
                        content.back()->w = font->getTextWidth(content.back()->text);
                        flushLine();
                        it.text.erase(0, pos+1);
                        it.w = font->getTextWidth(it.text);
                        x += content.back()->w;
                    } else {
                        /* Nope, give up totally. */
                        flushItems(i);
                        last_chunk.erase(last_chunk.begin(), last_chunk.begin() + i);
                    }
                }
                flushLine();
                i = 0;
                addX = x;
                lastWithSpace = 0;
            }
        } else {
            /* No overflow, keep going */
            if (last_chunk[i-1]->text.find(' ') != String_t::npos) {
                lastWithSpace = i-1;
            }
        }
    }

    if (lastWithSpace) {
        flushItems(i);
        last_chunk.erase(last_chunk.begin(), last_chunk.begin() + i);
    }
}

/** Add vertical space. This will consume float objects. */
void
ui::rich::Document::addY(int dy)
{
    // ex RichDocument::addY
    while (dy > 0) {
        /* Advance to the end of the next float object, if possible. */
        int now = dy;
        for (int side = 0; side < 2; ++side) {
            if (bo_height[side] > 0 && now > bo_height[side]) {
                now = bo_height[side];
            }
        }
        this->y += now;
        dy -= now;

        /* Account for float objects */
        for (int side = 0; side < 2; ++side) {
            if (bo_height[side] > 0) {
                /* Account delta */
                bo_height[side] -= now;

                /* When at end of the object, AND there is another object
                   which is the same width or narrower than the existing one,
                   start it immediately. */
                if (bo_height[side] == 0) {
                    findNextObject(side);
                    if (bo_index[side] < block_objs.size() && block_objs[bo_index[side]]->pos.getWidth() <= bo_width[side]) {
                        startNextObject(side);
                    } else {
                        bo_width[side] = 0;
                    }
                }
            }
        }
    }

    /* We can now start new objects at will */
    for (int side = 0; side < 2; ++side) {
        if (bo_height[side] == 0) {
            findNextObject(side);
            if (bo_index[side] < block_objs.size()) {
                startNextObject(side);
            }
        }
    }
}

/** Advance index of specified side to next possible object for that side.
    If there is no such object, the pointer is placed at the end of the
    block_objs list. */
void
ui::rich::Document::findNextObject(int side)
{
    // ex RichDocument::findNextObject
    while (bo_index[side] < block_objs.size() && block_objs[bo_index[side]]->kind != side) {
        ++bo_index[side];
    }
}

/** Place the current object of the given side.
    @pre bo_index[side] points to an object */
void
ui::rich::Document::startNextObject(int side)
{
    // ex RichDocument::startNextObject
    BlockItem& bo = *block_objs[bo_index[side]];
    bo.pos.setLeftX(side == BlockItem::Left ? 0 : page_width - bo.pos.getWidth());
    bo.pos.setTopY(y);
    bo_height[side] = bo.pos.getHeight();
    bo_width[side] = bo.pos.getWidth();
    ++bo_index[side];
}

/** Finish a line. Makes sure that content contains only fully-rendered text. */
void
ui::rich::Document::flushLine()
{
    // ex RichDocument::flushLine
    /* Figure out maximum height of last line */
    int maxHeight = 0;
    for (size_t i = first_this_line; i < content.size(); ++i) {
        Item& it = *content[i];
        if (it.kind != LinkTarget) {
            int thisHeight = m_provider.getFont(it.font)->getLineHeight();
            if (thisHeight > maxHeight)
                maxHeight = thisHeight;
        }
    }

    /* Adjust everything. */
    /* Brute-force approach of baseline handling. Our default fonts have:
          height      10  16  22
          baseline     2   4   5  (=pixels for descenders; "room below a")
       This yields an approximation of height/4 for the baseline height.
       So let's assume a default of 4, and shift the others around a little. */
    for (size_t i = first_this_line; i < content.size(); ++i) {
        Item& it = *content[i];
        int fontHeight = m_provider.getFont(it.font)->getLineHeight();
        it.y = y + maxHeight - fontHeight - 4 + fontHeight/4;
    }

    /* Advance cursors */
    first_this_line = content.size();
    addY(maxHeight);
    x = left_margin + bo_width[BlockItem::Left];
}

void
ui::rich::Document::flushWord()
{
    // ex RichDocument::flushWord
    flushItems(last_chunk.size());
    last_chunk.clear();
}

void
ui::rich::Document::flushItems(std::size_t n)
{
    // ex RichDocument::flushItems
    for (std::size_t i = 0; i < n; ++i) {
        Item* it = last_chunk.extractElement(i);
        it->x = x;
        content.pushBackNew(it);
        x += it->w;
    }
}
