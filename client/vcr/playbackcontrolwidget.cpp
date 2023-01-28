/**
  *  \file client/vcr/playbackcontrolwidget.cpp
  */

#include "client/vcr/playbackcontrolwidget.hpp"

client::vcr::PlaybackControlWidget::PlaybackControlWidget(ui::Root& root, bool acceptShiftMove)
    : Widget(),
      m_root(root),
      m_startButton("\xEE\x85\x84""\xE2\x97\x80", util::KeyMod_Alt + util::Key_Left, root),
      m_rewindButton("\xE2\x97\x80""\xE2\x97\x80", util::KeyMod_Ctrl + util::Key_Left, root),
      m_playButton("\xE2\x96\xB6", util::Key_Right, root),
      m_forwardButton("\xE2\x96\xB6""\xE2\x96\xB6", util::KeyMod_Ctrl + util::Key_Right, root),
      m_endButton("\xE2\x96\xB6""\xEE\x85\x84", util::KeyMod_Alt + util::Key_Right, root),
      m_acceptShiftMove(acceptShiftMove)
{
    // ex WVcrPlayWindow::init [part]
    addChild(m_startButton, 0);
    addChild(m_rewindButton, 0);
    addChild(m_playButton, 0);
    addChild(m_forwardButton, 0);
    addChild(m_endButton, 0);

    m_startButton.sig_fire.add(this, &PlaybackControlWidget::onStart);
    m_rewindButton.sig_fire.add(this, &PlaybackControlWidget::onRewind);
    m_playButton.sig_fire.add(this, &PlaybackControlWidget::onPlay);
    m_forwardButton.sig_fire.add(this, &PlaybackControlWidget::onForward);
    m_endButton.sig_fire.add(this, &PlaybackControlWidget::onEnd);
}

client::vcr::PlaybackControlWidget::~PlaybackControlWidget()
{ }

void
client::vcr::PlaybackControlWidget::setPlayState(bool playing)
{
    m_playButton.setFlag(ui::HighlightedButton, playing);
}

void
client::vcr::PlaybackControlWidget::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
client::vcr::PlaybackControlWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::vcr::PlaybackControlWidget::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::vcr::PlaybackControlWidget::handleChildAdded(Widget& /*child*/)
{ }

void
client::vcr::PlaybackControlWidget::handleChildRemove(Widget& /*child*/)
{ }

void
client::vcr::PlaybackControlWidget::handlePositionChange()
{
    gfx::Rectangle r = getExtent();
    int width = (r.getWidth()-4)/5;
    m_startButton.setExtent(r.splitX(width));
    r.consumeX(1);
    m_rewindButton.setExtent(r.splitX(width));
    r.consumeX(1);
    m_playButton.setExtent(r.splitX(width));
    r.consumeX(1);
    m_forwardButton.setExtent(r.splitX(width));
    r.consumeX(1);
    m_endButton.setExtent(r);
}

void
client::vcr::PlaybackControlWidget::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::vcr::PlaybackControlWidget::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    int size = font->getCellSize().getY() + 2;

    return gfx::Point(size*5 + 4, size);
}

bool
client::vcr::PlaybackControlWidget::handleKey(util::Key_t key, int prefix)
{
    switch (key) {
     case util::KeyMod_Shift + util::Key_Left:
        if (m_acceptShiftMove) {
            sig_moveBy.raise(-1);
            return true;
        } else {
            return false;
        }

     case util::KeyMod_Shift + util::Key_Right:
        if (m_acceptShiftMove) {
            sig_moveBy.raise(+1);
            return true;
        } else {
            return false;
        }

     case 'F':
        sig_moveBy.raise(+1);
        return true;

     case 'B':
        sig_moveBy.raise(-1);
        return true;

     case ' ':
     case util::Key_Return:
        sig_togglePlay.raise();
        return true;

     default:
        return defaultHandleKey(key, prefix);
    }
}

bool
client::vcr::PlaybackControlWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::vcr::PlaybackControlWidget::onStart()
{
    sig_moveToBeginning.raise();
}

void
client::vcr::PlaybackControlWidget::onRewind()
{
    sig_moveBy.raise(-20);
}

void
client::vcr::PlaybackControlWidget::onPlay()
{
    sig_togglePlay.raise();
}

void
client::vcr::PlaybackControlWidget::onForward()
{
    sig_moveBy.raise(+20);
}

void
client::vcr::PlaybackControlWidget::onEnd()
{
    sig_moveToEnd.raise();
}
