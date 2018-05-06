/**
  *  \file client/vcr/playbackcontrolwidget.hpp
  */
#ifndef C2NG_CLIENT_VCR_PLAYBACKCONTROLWIDGET_HPP
#define C2NG_CLIENT_VCR_PLAYBACKCONTROLWIDGET_HPP

#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"
#include "ui/root.hpp"
#include "afl/base/signal.hpp"

namespace client { namespace vcr {

    class PlaybackControlWidget : public ui::Widget {
     public:
        PlaybackControlWidget(ui::Root& root);
        ~PlaybackControlWidget();

        void setPlayState(bool playing);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Signal: toggle playback state.
            Called to toggle playback. */
        afl::base::Signal<void()> sig_togglePlay;

        /** Signal: move by a number of ticks.
            \param n Number of ticks (positive for FF, negative for rewind) */
        afl::base::Signal<void(int)> sig_moveBy;

        /** Signal: move to beginning of fight. */
        afl::base::Signal<void()> sig_moveToBeginning;

        /** Signal: move to end of fight. */
        afl::base::Signal<void()> sig_moveToEnd;

     private:
        ui::Root& m_root;
        ui::widgets::Button m_startButton;
        ui::widgets::Button m_rewindButton;
        ui::widgets::Button m_playButton;
        ui::widgets::Button m_forwardButton;
        ui::widgets::Button m_endButton;

        void onStart();
        void onRewind();
        void onPlay();
        void onForward();
        void onEnd();
    };

} }

#endif
