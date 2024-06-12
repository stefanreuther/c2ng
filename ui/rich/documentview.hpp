/**
  *  \file ui/rich/documentview.hpp
  */
#ifndef C2NG_UI_RICH_DOCUMENTVIEW_HPP
#define C2NG_UI_RICH_DOCUMENTVIEW_HPP

#include "ui/scrollablewidget.hpp"
#include "ui/rich/document.hpp"

namespace ui { namespace rich {

    // /** Rich document widget. Displays a RichDocument and allows the user
    //     to interact with it. Since the RichDocument stores preformatted text,
    //     there are two ways to fill the document:
    //     - after doing layout, but before showing the widget, populate the
    //       document normally
    //     - while the widget is active, manipulate the document and finally
    //       call handleDocumentUpdate(). */
    class DocumentView : public ScrollableWidget {
     public:
        enum {
            fl_Tab    = 1,
            fl_Return = 2,
            fl_Space  = 4,
            fl_Scroll = 8,
            fl_ScrollMark = 16,

            /* Combination: single-page hypertext */
            fl_SingleHyper = fl_Tab + fl_Return + fl_Space,

            /* Combination: help viewer */
            fl_Help = fl_Tab + fl_Return + fl_Scroll
        };
        DocumentView(gfx::Point pref_size, uint16_t key_flags, gfx::ResourceProvider& provider);
        ~DocumentView();

        Document& getDocument();
        const Document& getDocument() const;

        void handleDocumentUpdate();
        void adjustToDocumentSize();

        void setPreferredSize(gfx::Point prefSize);

        Document::LinkId_t getSelectedLink() const;
        void setSelectedLink(Document::LinkId_t link);

        void setTopY(int topY);
        void addTopY(int deltaY);

        afl::base::Signal<void(String_t)> sig_linkClick;

        // ScrollableWidget:
        virtual int getPageTop() const;
        virtual int getPageSize() const;
        virtual int getTotalSize() const;
        virtual void setPageTop(int top);
        virtual void scroll(Operation op);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        void setHoverLink(Document::LinkId_t link);
        void setLink(Document::LinkId_t link);
        int getScrollStep() const;
        void updateScrollable();

        gfx::ResourceProvider& m_provider;
        Document doc;
        gfx::Point pref_size;

        uint16_t key_flags;
        bool mdown;
        Document::LinkId_t selected_link;
        Document::LinkId_t hover_link;

        // Scrolling
        int m_topY;
    };

} }

#endif
