/**
  *  \file client/widgets/markertemplatelist.hpp
  *  \brief Class client::widgets::MarkerTemplateList
  */
#ifndef C2NG_CLIENT_WIDGETS_MARKERTEMPLATELIST_HPP
#define C2NG_CLIENT_WIDGETS_MARKERTEMPLATELIST_HPP

#include <vector>
#include "afl/string/translator.hpp"
#include "game/config/markeroption.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List of marker templates (canned markers).
        Displays the list with no further interaction logic. */
    class MarkerTemplateList : public ui::widgets::AbstractListbox {
     public:
        /** Shortcut for the definition of a marker template. */
        typedef game::config::MarkerOption::Data Data_t;

        /** Shortcut for a list of marker templates. */
        typedef std::vector<Data_t> DataVector_t;

        /** Constructor.
            @param root UI root (for colors, fonts)
            @param tx   Translator */
        MarkerTemplateList(ui::Root& root, afl::string::Translator& tx);
        ~MarkerTemplateList();

        /** Set content.
            Replaces the entire list.
            @param content Content */
        void setContent(const DataVector_t& content);

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
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        int getLineHeight() const;
        afl::base::Ref<gfx::Font> getFont() const;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        DataVector_t m_content;
    };

} }

#endif
