/**
  *  \file client/widgets/filterdisplay.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_FILTERDISPLAY_HPP
#define C2NG_CLIENT_WIDGETS_FILTERDISPLAY_HPP

#include "ui/simplewidget.hpp"
#include "ui/root.hpp"
#include "afl/string/translator.hpp"
#include "game/spec/info/types.hpp"

namespace client { namespace widgets {

    /** Filter display.
        Displays a game::spec::info::FilterInfos_t and controls to manipulate it.

        The widget sort-of mimics a list (but currently does not scroll).
        It provides a couple of signals the caller needs to hook to edit the filter.

        The widget provides mouse and keyboard control. */
    class FilterDisplay : public ui::SimpleWidget {
     public:
        /** Constructor.
            \param root Root
            \param tx Translator */
        FilterDisplay(ui::Root& root, afl::string::Translator& tx);

        /** Set content.
            Sets the filter items to display.
            \param infos Infos */
        void setContent(const game::spec::info::FilterInfos_t& infos);

        /** Set sort order.
            \param label Text to show (sort key label)
            \param active true if sort key is valid. Note that passing false here
                          will just fade the display; it will not prevent sig_sort
                          from being triggered. */
        void setSort(String_t label, bool active);

        /** Set availability of "add filter" function (sig_add).
            \param flag Availability */
        void setFilterAvailable(bool flag);

        /** Get anchor point for a "filter" drop-down menu.
            \return anchor point */
        gfx::Point getFilterAnchor() const;

        /** Get anchor point for a "sort" drop-down menu.
            \return anchor point */
        gfx::Point getSortAnchor() const;

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Signal: edit filter.
            \param index 0-based index of filter element to edit */
        afl::base::Signal<void(size_t)> sig_edit;

        /** Signal: delete filter.
            \param index 0-based index of filter element to delete */
        afl::base::Signal<void(size_t)> sig_delete;

        /** Signal: add a new filter. */
        afl::base::Signal<void()> sig_add;

        /** Signal: change the sort order. */
        afl::base::Signal<void()> sig_sort;

     private:
        enum Highlight {
            None,
            Delete,
            Edit,
            Add,
            Sort
        };

        enum Focus {
            fEdit,
            fAdd,
            fSort
        };

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        game::spec::info::FilterInfos_t m_content;
        Highlight m_highlight;
        size_t m_highlightIndex;

        Focus m_focus;
        size_t m_focusIndex;

        bool m_mouseDown;
        bool m_filterAvailable;
        String_t m_sortLabel;
        bool m_sortActive;

        int getFilterHeight() const;
        int getMenuHeight() const;
        int getFilterButtonWidth() const;
        void setHighlight(Highlight h, size_t index);
        void setFocus(Focus f, size_t index);
    };

} }

#endif
