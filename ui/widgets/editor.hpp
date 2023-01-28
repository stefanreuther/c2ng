/**
  *  \file ui/widgets/editor.hpp
  *  \brief Class ui::widgets::Editor
  */
#ifndef C2NG_UI_WIDGETS_EDITOR_HPP
#define C2NG_UI_WIDGETS_EDITOR_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/base/closure.hpp"
#include "afl/charset/unicode.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/editor/editor.hpp"
#include "util/syntax/highlighter.hpp"

namespace ui { namespace widgets {

    /** Editor widget.
        Allows control of a multi-line util::editor::Editor.
        Changes to the underlying Editor will be displayed.

        Additional features:
        - allows scrolling if the editor size exceeds that of the widget
        - line-based syntax highlighting
        - filtering of typed characters */
    class Editor : public SimpleWidget {
     public:
        /** Character filter.
            \see setCharacterFilter */
        typedef afl::base::Closure<bool(afl::charset::Unichar_t)> CharacterFilter_t;


        /** Constructor.
            \param ed   Editor. Must outlive this widget.
            \param root UI root (for color scheme, font) */
        Editor(util::editor::Editor& ed, Root& root);
        ~Editor();

        /** Set preferred size in pixels.
            \param size Size */
        void setPreferredSize(gfx::Point size);

        /** Set preferred size in font cells.
            \param columns Number of columns
            \param lines   Number of lines */
        void setPreferredSizeInCells(size_t columns, size_t lines);

        /** Set first column to show (scroll horizontally).
            \param fc First column (0-based) */
        void setFirstColumn(size_t fc);

        /** Set first line to show (scroll vertically).
            \param fl First line (0-based) */
        void setFirstLine(size_t fl);

        /** Toggle whether scrolling is allowed.
            Note that disabling this allows the user to move the cursor out of view.
            \param flag Flag */
        void setAllowScrolling(bool flag);

        /** Set editor flag.
            Use to toggle the Overwrite, WordWrap, AllowCursorAfterEnd flags.
            \param flag   Flag
            \param enable New value */
        void setFlag(util::editor::Flag flag, bool enable);

        /** Toggle editor flag.
            Use to toggle the Overwrite, WordWrap, AllowCursorAfterEnd flags.
            \param flag   Flag */
        void toggleFlag(util::editor::Flag flag);

        /** Set highlighter to use.
            This enables syntax-coloring.

            Text is highlighted on by-line basis.
            The highlighter therefore must not carry over state from one line to another,
            but be able to colorize each line individually.
            This means, things like C comments or line continuations are not supported.

            \param p Highlighter. Can be null to disable; otherwise, lifetime must exceed that of the editor. */
        void setHighlighter(util::syntax::Highlighter* p);

        /** Set character filter.
            When set, only characters accepted by it are accepted.
            When no character filter is set (default), all Unicode characters are accepted.

            \param p Character filter. Can be null to disable; otherwise, lifetime must exceed that of the editor. */
        void setCharacterFilter(CharacterFilter_t* p);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        void onEditorChange(size_t firstLine, size_t lastLine);
        bool acceptUnicode(afl::charset::Unichar_t ch) const;
        afl::base::Ref<gfx::Font> getFont();

        util::editor::Editor& m_editor;
        util::editor::Flags_t m_editorFlags;
        gfx::Point m_preferredSize;
        Root& m_root;
        size_t m_firstColumn;
        size_t m_firstLine;
        bool m_allowScrolling;

        util::syntax::Highlighter* m_pHighlighter;
        CharacterFilter_t* m_pCharacterFilter;

        afl::base::SignalConnection conn_editorChange;
    };

} }

#endif
