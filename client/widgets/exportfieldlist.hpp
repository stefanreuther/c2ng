/**
  *  \file client/widgets/exportfieldlist.hpp
  *  \brief Class client::widgets::ExportFieldList
  */
#ifndef C2NG_CLIENT_WIDGETS_EXPORTFIELDLIST_HPP
#define C2NG_CLIENT_WIDGETS_EXPORTFIELDLIST_HPP

#include "afl/string/translator.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** Export Field List widget.
        Displays a interpreter::exporter::FieldList.
        The user can browse through the list, including an empty blank line at the bottom.
        Each entry shows the field name, as well as the optional width and alignment.

        Other than general list operations, this widget provides no user interaction. */
    class ExportFieldList : public ui::widgets::AbstractListbox {
     public:
        /** Constructor.
            @param root    UI Root
            @param tx      Translator (for placeholder in last line) */
        ExportFieldList(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~ExportFieldList();

        /** Set content.
            Replaces the entire field list.
            @param newContent New content */
        void setContent(const interpreter::exporter::FieldList& newContent);

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        interpreter::exporter::FieldList m_content;

        afl::base::Ref<gfx::Font> getFont() const;
    };

} }

#endif
