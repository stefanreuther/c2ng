/**
  *  \file client/widgets/decayingmessage.cpp
  *  \brief Decaying Message widget
  */

#include "client/widgets/decayingmessage.hpp"
#include "ui/eventloop.hpp"
#include "gfx/context.hpp"
#include "ui/simplewidget.hpp"

namespace {
    const int MAX_STATE = 8;

    class DecayingMessage : public ui::SimpleWidget {
     public:
        DecayingMessage(ui::Root& root, const String_t& text);
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void run();

     private:
        afl::base::Ref<gfx::Font> getFont() const;
        void startTimer();
        void onTimer();

        ui::Root& m_root;
        ui::EventLoop m_loop;
        String_t m_text;

        afl::base::Ref<gfx::Timer> m_timer;
        int m_state;
    };
}


DecayingMessage::DecayingMessage(ui::Root& root, const String_t& text)
    : m_root(root), m_loop(root), m_text(text),
      m_timer(m_root.engine().createTimer()),
      m_state(0)
{
    setState(ModalState, true);
    m_timer->sig_fire.add(this, &DecayingMessage::onTimer);
}

void
DecayingMessage::draw(gfx::Canvas& can)
{
    // ex WDecayingMessage::drawContent
    // "Frame" in COLOR_DARK
    // "Text" in COLOR_GRAYSCALE+15 .. COLOR_GRAYSCALE+7
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*getFont());
    ctx.setColor(ui::Color_Dark);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);

    // "Frame"
    gfx::Point pt = getExtent().getCenter();
    for (int i = -1; i <= +1; ++i) {
        for (int j = -1; j <= +1; ++j) {
            if (i != 0 || j != 0) {
                outText(ctx, pt + gfx::Point(i, j), m_text);
            }
        }
    }

    // "Text"
    ctx.setColor(uint8_t(ui::Color_Grayscale+15 - std::min(m_state, MAX_STATE)));
    outText(ctx, pt, m_text);
}

void
DecayingMessage::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
DecayingMessage::handlePositionChange()
{ }

ui::layout::Info
DecayingMessage::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font = getFont();
    return gfx::Point(font->getTextWidth(m_text) + 2,
                      font->getTextHeight(m_text) + 2);
}

bool
DecayingMessage::handleKey(util::Key_t key, int prefix)
{
    if (util::classifyKey(key) != util::ModifierKey) {
        m_root.ungetKeyEvent(key, prefix);
        m_loop.stop(0);
    }
    return true;
}

bool
DecayingMessage::handleMouse(gfx::Point /*pt*/, MouseButtons_t pressedButtons)
{
    if (!pressedButtons.empty()) {
        m_root.postMouseEvent();
        m_loop.stop(0);
    }
    return true;
}

void
DecayingMessage::run()
{
    startTimer();
    m_loop.run();
}

afl::base::Ref<gfx::Font>
DecayingMessage::getFont() const
{
    return m_root.provider().getFont("+");
}

void
DecayingMessage::startTimer()
{
    if (m_state == 0) {
        m_timer->setInterval(500);
    } else {
        m_timer->setInterval(75);
    }
}

void
DecayingMessage::onTimer()
{
    ++m_state;
    if (m_state >= MAX_STATE) {
        m_loop.stop(0);
    } else {
        startTimer();
        requestRedraw();
    }
}


void
client::widgets::showDecayingMessage(ui::Root& root, String_t text)
{
    DecayingMessage msg(root, text);
    msg.setExtent(gfx::Rectangle(gfx::Point(), msg.getLayoutInfo().getPreferredSize()));
    root.centerWidget(msg);
    root.add(msg);
    msg.run();
}
