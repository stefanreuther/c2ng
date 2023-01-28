/**
  *  \file client/widgets/stoppablebusyindicator.hpp
  *  \brief Class client::widgets::StoppableBusyIndicator
  */
#ifndef C2NG_CLIENT_WIDGETS_STOPPABLEBUSYINDICATOR_HPP
#define C2NG_CLIENT_WIDGETS_STOPPABLEBUSYINDICATOR_HPP

#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/statictext.hpp"

namespace client { namespace widgets {

    /** "Busy" indicator with a "Stop" button.
        Use for background operations that take long, but are interruptible.

        Usage:
        - create it
        - set up sig_stop to deliver a stop request to the background operation
        - set up the background operation to call stop() from a UI thread callback when it stopped voluntarily or on request
        - call run()

        A possible Quit request (Key_Quit) will also be accepted and be re-posted when run completes.

        This is implemented as a compound Widget for now, but this is not contractual. */
    class StoppableBusyIndicator : private ui::Widget {
     public:
        /** Constructor.
            \param root Root
            \param tx Translator */
        StoppableBusyIndicator(ui::Root& root, afl::string::Translator& tx);
        ~StoppableBusyIndicator();

        /** Show the indicator.
            This will add the indicator to Root and process UI messages until stop() is called,
            at which time it will return.
            \retval true User had requested stop
            \retval false User did not request stop */
        bool run();

        /** Operation stopped.
            Call from a UI thread callback (i.e. RequestDispatcher).
            This will cause run() to exit. */
        void stop();

        /** Signal: user requested stop. */
        afl::base::Signal<void()> sig_stop;

     private:
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

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::SkinColorScheme m_colors;
        ui::widgets::Button m_button;
        ui::widgets::StaticText m_text;
        ui::EventLoop m_loop;
        bool m_canceled;
        bool m_quit;

        void doLayout();
        void onStop();
    };

} }

#endif
