/**
  *  \file ui/tooltip.hpp
  *  \brief Class ui::Tooltip
  */
#ifndef C2NG_UI_TOOLTIP_HPP
#define C2NG_UI_TOOLTIP_HPP

#include "afl/base/ref.hpp"
#include "afl/base/signal.hpp"
#include "gfx/timer.hpp"
#include "ui/icons/icon.hpp"
#include "ui/widget.hpp"

namespace ui {

    class Root;

    /** Tooltip implementation helper.
        This class can be used in implementing components that have tooltips.
        It observes user actions and emits a signal when a tooltip shall be displayed.

        To use,
        - create Tooltip instance as part of the widget
        - connect sig_hover
        - dispatch the widget's handleMouse, handleKey, handleStateChange methods into the Tooltip instance.
        - when sig_hover fires, call showPopup(). */
    class Tooltip {
     public:
        /** Constructor.
            \param root UI Root */
        explicit Tooltip(Root& root);
        ~Tooltip();

        /** Handle mouse event.
            Call from the Widget::handleMouse implementation.
            If you pass inside=true, you should also call the widget's requestActive() for reliable detection of activation loss.

            This function does not consume the event and should not by itself cause Widget::handleMouse to return true.

            \param pt              Position (from Widget::handleMouse)
            \param pressedButtons  Buttons (from Widget::handleMouse)
            \param inside          Whether the position is considered inside the widget/sensitive area */
        void handleMouse(gfx::Point pt, gfx::EventConsumer::MouseButtons_t pressedButtons, bool inside);

        /** Handle key event.
            Call from the Widget::handleKey implementation.

            This function does not consume the event and should not by itself cause Widget::handleKey to return true.

            \param key    Key (from Widget::handleKey)
            \param prefix Prefix (from Widget::handleKey) */
        void handleKey(util::Key_t key, int prefix);

        /** Handle state change.
            Call from the Widget::handleStateChange implementation.
            This is required to detect activation loss.

            \param st     State (from Widget::handleStateChange)
            \param enable New status (from Widget::handleStateChange) */
        void handleStateChange(Widget::State st, bool enable);

        /** Cancel popup.
            Call whenever some external event causes the popup to be suppressed. */
        void cancel();

        /** Show popup.
            Intended to be called from the sig_hover callback, but can also be called directly.

            \param pt    Anchor point for popup. Should be (close to) the position received from sig_hover
            \param icon  Content of popup */
        void showPopup(gfx::Point pt, ui::icons::Icon& icon);

        /** Signal: mouse is hovering over sensitive area.
            \param pt Position */
        afl::base::Signal<void(gfx::Point)> sig_hover;

     private:
        Root& m_root;
        afl::base::Ref<gfx::Timer> m_timer;
        gfx::Point m_pos;
        bool m_active;

        void onTimer();
    };

}

#endif
