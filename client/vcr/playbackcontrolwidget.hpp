/**
  *  \file client/vcr/playbackcontrolwidget.hpp
  *  \brief Class client::vcr::PlaybackControlWidget
  */
#ifndef C2NG_CLIENT_VCR_PLAYBACKCONTROLWIDGET_HPP
#define C2NG_CLIENT_VCR_PLAYBACKCONTROLWIDGET_HPP

#include "afl/base/signal.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace vcr {

    /** Widget to control VCR playback.
        Shows a set of Play/FF/Rew buttons and handles those buttons as well as keypresses to generate events. */
    class PlaybackControlWidget : public ui::Widget {
     public:
        /** Constructor.
            @param root             UI root
            @param acceptShiftMove  If true, accept Shift+Left/Right for single-frame forward/rewind. */
        PlaybackControlWidget(ui::Root& root, bool acceptShiftMove);

        /** Destructor. */
        ~PlaybackControlWidget();

        /** Set play status.
            Play status is shown as highlight of the "Play" button.
            @param playing If true, we are playing */
        void setPlayState(bool playing);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
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

        /** Signal: speed change.
            \param bool true for faster; false for slower */
        afl::base::Signal<void(bool)> sig_changeSpeed;

     private:
        ui::Root& m_root;
        ui::widgets::Button m_startButton;
        ui::widgets::Button m_rewindButton;
        ui::widgets::Button m_playButton;
        ui::widgets::Button m_forwardButton;
        ui::widgets::Button m_endButton;
        bool m_acceptShiftMove;

        void onStart();
        void onRewind();
        void onPlay();
        void onForward();
        void onEnd();
    };

} }

#endif
