/**
  *  \file client/widgets/consolecontroller.hpp
  *  \brief Class client::widgets::ConsoleController
  */
#ifndef C2NG_CLIENT_WIDGETS_CONSOLECONTROLLER_HPP
#define C2NG_CLIENT_WIDGETS_CONSOLECONTROLLER_HPP

#include "afl/container/ptrvector.hpp"
#include "ui/invisiblewidget.hpp"
#include "util/skincolor.hpp"

namespace client { namespace widgets {

    class ConsoleView;

    /** Console Controller.
        Adds simple interactive behaviour to a ConsoleView.

        ConsoleView does not store content; it is intended to be driven from a separate console message buffer.
        Therefore, ConsoleController stores messages.

        It accepts keystrokes to scroll the console. */
    class ConsoleController : public ui::InvisibleWidget {
     public:
        /** Constructor.
            Make a new ConsoleController.
            @param view ConsoleView */
        explicit ConsoleController(ConsoleView& view);

        /** Add a line of text.
            @param text  Text
            @param align Alignment
            @param bold  Font weight
            @param color Color */
        void addLine(String_t text, gfx::HorizontalAlignment align, int bold, util::SkinColor::Color color);

        // Widget:
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        /** Link to ConsoleView. */
        ConsoleView& m_view;

        /** Storage for a single line. */
        struct Line {
            String_t text;
            gfx::HorizontalAlignment align;
            int bold;
            util::SkinColor::Color color;

            Line(const String_t& text, gfx::HorizontalAlignment align, int bold, util::SkinColor::Color color)
                : text(text), align(align), bold(bold), color(color)
                { }
        };

        /** All lines. */
        afl::container::PtrVector<Line> m_lines;

        /** Topmost displayed line, index into m_lines. */
        size_t m_topLine;

        void render();
        void scrollUp(size_t n);
        void scrollDown(size_t n);
        void scrollToUnchecked(size_t n);
    };

} }

#endif
