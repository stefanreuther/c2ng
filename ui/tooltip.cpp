/**
  *  \file ui/tooltip.cpp
  *  \brief Class ui::Tooltip
  */

#include "ui/tooltip.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/skincolorscheme.hpp"

namespace {
    const uint32_t TOOLTIP_INTERVAL_MS = 500;
    const int32_t MAX_MOVEMENT_DIST2 = 150;

    enum {
        FRAME = 4,
        OUT   = 10
    };

    class TooltipWidget : public ui::SimpleWidget {
     public:
        TooltipWidget(ui::Root& root, ui::icons::Icon& icon, gfx::Point anchor)
            : m_root(root),
              m_icon(icon),
              m_anchor(anchor),
              m_loop(root),
              m_top(false)
            {
                // ex UITooltip::UITooltip
                setState(ModalState, true);
                computePosition();
            }

        void run()
            {
                m_root.add(*this);
                m_loop.run();
            }

        virtual void draw(gfx::Canvas& can)
            {
                // ex UITooltip::drawContent
                // Colors
                enum {
                    BODY  = ui::Color_Fire + 29,
                    SHADE = ui::Color_DarkYellow,
                    LIGHT = ui::Color_Fire + 30
                };


                gfx::Rectangle r = getExtent();
                gfx::Context<uint8_t> ctx(can, m_root.colorScheme());

                // Context for inside
                ui::SkinColorScheme skinColorScheme(ui::GRAY_COLOR_SET, m_root.colorScheme());
                gfx::Context<util::SkinColor::Color> skinContext(can, skinColorScheme);

                if (m_top) {
                    /*
                      ,---------------------. ---
                      |                     |
                      |                     |
                      `--------------.  .---' ---
                                      \ |     OUT
                                       \|     ---
                     */
                    // Body that will contain icon
                    drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY(), r.getWidth(), r.getHeight()-OUT), BODY);

                    // Frame
                    ctx.setColor(LIGHT);
                    drawHLine(ctx, r.getLeftX(), r.getTopY(),   r.getRightX()-1);
                    drawVLine(ctx, r.getLeftX(), r.getTopY()+1, r.getBottomY() - OUT);

                    ctx.setColor(SHADE);
                    drawVLine(ctx, r.getRightX()-1, r.getTopY(),          r.getBottomY() - OUT);
                    drawHLine(ctx, r.getLeftX(),    r.getBottomY() - OUT, m_anchor.getX() - OUT);
                    drawHLine(ctx, m_anchor.getX(), r.getBottomY() - OUT, r.getRightX()-1);
                    drawVLine(ctx, m_anchor.getX(), r.getBottomY() - OUT, r.getBottomY()-1);

                    // Pointer
                    for (int i = 0; i < OUT; ++i) {
                        ctx.setColor(SHADE);
                        drawPixel(ctx, gfx::Point(m_anchor.getX() - OUT + i, r.getBottomY() - OUT + i));
                        if (i < OUT-1) {
                            ctx.setColor(BODY);
                            drawHLine(ctx, m_anchor.getX() - OUT + i + 1, r.getBottomY() - OUT + i, m_anchor.getX() - 1);
                        }
                    }

                    // Content
                    m_icon.draw(skinContext, gfx::Rectangle(r.getLeftX() + FRAME/2, r.getTopY() + FRAME/2, r.getWidth() - FRAME, r.getHeight() - OUT - FRAME), ui::ButtonFlags_t());
                } else {
                    /*
                                      /|      ---
                                     / |      OUT
                      ,-------------'  '----. ---
                      |                     |
                      |                     |
                      `---------------------' ---
                     */
                    // Body that will contain icon
                    drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY()+OUT, r.getWidth(), r.getHeight()-OUT), BODY);

                    // Frame
                    ctx.setColor(LIGHT);
                    drawHLine(ctx, r.getLeftX(),    r.getTopY()+OUT, m_anchor.getX()-OUT);
                    drawHLine(ctx, m_anchor.getX(), r.getTopY()+OUT, r.getRightX()-1);
                    drawVLine(ctx, r.getLeftX(),    r.getTopY()+OUT, r.getBottomY()-1);

                    ctx.setColor(SHADE);
                    drawVLine(ctx, r.getRightX()-1, r.getTopY() + OUT+1, r.getBottomY()-1);
                    drawHLine(ctx, r.getLeftX(),    r.getBottomY()-1,    r.getRightX()-1);
                    drawVLine(ctx, m_anchor.getX(), r.getTopY(),         r.getTopY() + OUT);

                    // Pointer
                    for (int i = 0; i < OUT; ++i) {
                        ctx.setColor(LIGHT);
                        drawPixel(ctx, gfx::Point(m_anchor.getX() - i - 1, r.getTopY() + i));
                        if (i > 0) {
                            ctx.setColor(BODY);
                            drawHLine(ctx, m_anchor.getX() - i, m_anchor.getY() + i, m_anchor.getX() - 1);
                        }
                    }

                    // Text
                    m_icon.draw(skinContext, gfx::Rectangle(r.getLeftX() + FRAME/2, r.getTopY() + OUT + FRAME/2, r.getWidth() - FRAME, r.getHeight() - OUT - FRAME), ui::ButtonFlags_t());
                }
            }

        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { requestRedraw(); }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                if (util::classifyKey(key) != util::ModifierKey) {
                    m_root.ungetKeyEvent(key, prefix);
                    m_loop.stop(0);
                }
                return true;
            }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            {
                int dx = m_anchor.getX() - pt.getX();
                int dy = m_anchor.getY() - pt.getY();
                if (!pressedButtons.empty() || dx*dx + dy*dy > MAX_MOVEMENT_DIST2) {
                    m_loop.stop(0);
                }
                return true;
            }

     private:
        void computePosition()
            {
                // ex UITooltip::computePosition
                gfx::Point contentSize = m_icon.getSize();
                int grossHeight = OUT + contentSize.getY() + FRAME;

                int y;
                if (m_anchor.getY() < grossHeight) {
                    m_anchor += gfx::Point(-1, +1);
                    y = m_anchor.getY();
                    m_top = false;
                } else {
                    m_anchor += gfx::Point(-1, -1);
                    y = m_anchor.getY() - grossHeight;
                    m_top = true;
                }
                int h = grossHeight;
                int x = m_anchor.getX() + OUT - (contentSize.getX() + FRAME);
                int w = contentSize.getX() + FRAME;
                gfx::Rectangle r(x, y, w, h);
                r.moveIntoRectangle(m_root.getExtent());
                setExtent(r);
            }

        ui::Root& m_root;
        ui::icons::Icon& m_icon;
        gfx::Point m_anchor;
        ui::EventLoop m_loop;
        bool m_top;
    };
}

ui::Tooltip::Tooltip(Root& root)
    : sig_hover(),
      m_root(root),
      m_timer(root.engine().createTimer()),
      m_pos(),
      m_active(false)
{
    m_timer->sig_fire.add(this, &Tooltip::onTimer);
}

ui::Tooltip::~Tooltip()
{ }

void
ui::Tooltip::handleMouse(gfx::Point pt, gfx::EventConsumer::MouseButtons_t pressedButtons, bool inside)
{
    // ex UITooltipControl::checkEvent (part)
    if (!inside || !pressedButtons.empty()) {
        cancel();
    } else {
        if (m_active) {
            // Try to check speed of movement; PCC2 does not seem to do that.
            int dx = m_pos.getX() - pt.getX();
            int dy = m_pos.getY() - pt.getY();
            if (dx*dx + dy*dy > MAX_MOVEMENT_DIST2) {
                cancel();
            }
            m_pos = pt;
        }
        if (!m_active) {
            m_active = true;
            m_pos = pt;
            m_timer->setInterval(TOOLTIP_INTERVAL_MS);
        }
    }
}

void
ui::Tooltip::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UITooltipControl::checkEvent (part)
    if (util::classifyKey(key) == util::NormalKey) {
        cancel();
    }
}

void
ui::Tooltip::handleStateChange(Widget::State st, bool enable)
{
    // ex UITooltipControl::checkState
    /* If this function is not called by the user, focus change will not prevent
       tooltips from appearing. For example, when the mouse hovers over a widget,
       keypresses should inhibit tooltips. They do so by obtaining logical focus,
       i.e. the st_Selected bit, causing the previous st_Selected widget to lose
       it. In addition, this catches widgets that steal mouse events when the
       mouse is moved very fast from the controlled widget to another one. */
    if (st == Widget::ActiveState && !enable) {
        cancel();
    }
}

void
ui::Tooltip::cancel()
{
    if (m_active) {
        m_active = false;
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
    }
}

void
ui::Tooltip::showPopup(gfx::Point pt, ui::icons::Icon& icon)
{
    // ex doTooltip
    TooltipWidget(m_root, icon, pt).run();
}


void
ui::Tooltip::onTimer()
{
    if (m_active) {
        m_active = false;
        sig_hover.raise(m_pos);
    }
}
