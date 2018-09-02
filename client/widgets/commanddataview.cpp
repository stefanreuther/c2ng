/**
  *  \file client/widgets/commanddataview.cpp
  */

#include <algorithm>
#include "client/widgets/commanddataview.hpp"
#include "gfx/context.hpp"
#include "ui/rich/draw.hpp"
#include "ui/simplewidget.hpp"
#include "gfx/complex.hpp"
#include "gfx/clipfilter.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"
#include "client/widgets/keymapwidget.hpp"

// FIXME: move elsewhere?
namespace client { namespace widgets { namespace {

    class StaticTextPair : public ui::SimpleWidget {
     public:
        StaticTextPair(gfx::ResourceProvider& provider)
            : m_provider(provider),
              m_left(),
              m_right()
            { }
        // SimpleWidget:
        virtual void draw(gfx::Canvas& can)
            {
                gfx::Rectangle r = getExtent();
                {
                    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                    drawBackground(ctx, r);
                }

                int rightWidth = ui::rich::getTextWidth(m_right, m_provider);
                int leftWidth = std::max(0, getExtent().getWidth() - rightWidth);
                
                gfx::Rectangle leftArea = r.splitX(leftWidth);

                int centerY = leftArea.getCenter().getY();

                {
                    gfx::ClipFilter filter(can, leftArea);
                    gfx::Context<util::SkinColor::Color> ctx(filter, getColorScheme());
                    ctx.setTextAlign(0, 1);
                    ui::rich::outText(ctx, gfx::Point(leftArea.getLeftX(), centerY), m_left, m_provider);
                }
                {
                    gfx::ClipFilter filter(can, r);
                    gfx::Context<util::SkinColor::Color> ctx(filter, getColorScheme());
                    ctx.setTextAlign(0, 1);
                    ui::rich::outText(ctx, gfx::Point(r.getLeftX(), centerY), m_right, m_provider);
                }
            }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { requestRedraw(); }
        virtual ui::layout::Info getLayoutInfo() const
            {
                gfx::Point size = m_provider.getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 1);
                return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
            }
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }

        void setText(bool left, const util::rich::Text& t)
            {
                if (left) {
                    m_left = t;
                } else {
                    m_right = t;
                }
                requestRedraw();
            }

     private:
        gfx::ResourceProvider& m_provider;
        util::rich::Text m_left;
        util::rich::Text m_right;
    };

} } }

struct client::widgets::CommandDataView::Line {
    ui::widgets::FrameGroup frame;
    ui::widgets::Button button;
    StaticTextPair textPair;

    Line(String_t title, util::Key_t key, ui::Root& root)
        : frame(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::NoFrame),
          button(title, key, root),
          textPair(root.provider())
        {
            frame.setFrameWidth(2);
            frame.add(button);
        }
};

client::widgets::CommandDataView::CommandDataView(ui::Root& root, KeymapWidget& widget, Mode mode)
    : CollapsibleDataView(root),
      m_keys(widget),
      m_lines(),
      m_mode(mode)
{ }

client::widgets::CommandDataView::~CommandDataView()
{ }

void
client::widgets::CommandDataView::setChildPositions()
{
    gfx::Point buttonSize = findButtonSize();
    gfx::Rectangle r = getExtent();
    r.moveTo(getAnchorPoint(LeftAligned | DataAligned));

    int buttonWidth = std::min(r.getWidth(), buttonSize.getX());
    int textWidth   = r.getWidth() - buttonWidth;

    for (size_t i = 0, n = m_lines.size(); i < n; ++i) {
        Line& p = *m_lines[i];
        gfx::Rectangle row(r.splitY(buttonSize.getY()));
        switch (m_mode) {
         case ButtonsLeft:
            p.frame.setExtent(row.splitX(buttonWidth));
            row.consumeX(5);
            p.textPair.setExtent(row);
            break;

         case ButtonsRight:
            p.textPair.setExtent(row.splitX(textWidth));
            p.frame.setExtent(row);
            break;
        }
    }
}

gfx::Point
client::widgets::CommandDataView::getPreferredChildSize() const
{
    gfx::Point buttonSize = findButtonSize();
    return gfx::Point(buttonSize.getX() + root().provider().getFont(gfx::FontRequest())->getEmWidth() * 20, int(buttonSize.getY() * m_lines.size()));
}

void
client::widgets::CommandDataView::addButton(String_t title, util::Key_t key)
{
    Line* p = m_lines.pushBackNew(new Line(title, key, root()));
    addChild(p->frame, 0);
    addChild(p->textPair, 0);
    p->button.dispatchKeyTo(m_keys);
}

bool
client::widgets::CommandDataView::setText(util::Key_t key, bool left, const util::rich::Text& text)
{
    if (Line* p = findLine(key)) {
        p->textPair.setText(left, text);
        return true;
    } else {
        return false;
    }
}

bool
client::widgets::CommandDataView::setFrame(util::Key_t key, ui::widgets::FrameGroup::Type type)
{
    if (Line* p = findLine(key)) {
        p->frame.setType(type);
        return true;
    } else {
        return false;
    }
}

client::widgets::CommandDataView::Line*
client::widgets::CommandDataView::findLine(util::Key_t key)
{
    for (size_t i = 0, n = m_lines.size(); i < n; ++i) {
        if (m_lines[i]->button.getKey() == key) {
            return m_lines[i];
        }
    }
    return 0;
}

gfx::Point
client::widgets::CommandDataView::findButtonSize() const
{
    // We use the button's metrics but fit button+frame into that, making the buttons a little smaller.
    int maxX = 0;
    int maxY = 0;
    for (size_t i = 0, n = m_lines.size(); i < n; ++i) {
        ui::layout::Info info = m_lines[i]->button.getLayoutInfo();
        if (!info.isIgnored()) {
            gfx::Point pt = info.getPreferredSize();
            maxX = std::max(maxX, pt.getX());
            maxY = std::max(maxY, pt.getY());
        }
    }
    return gfx::Point(maxX, maxY);
}
