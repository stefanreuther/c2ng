/**
  *  \file client/widgets/configvaluelist.hpp
  *  \brief Class client::widgets::ConfigValueList
  */
#ifndef C2NG_CLIENT_WIDGETS_CONFIGVALUELIST_HPP
#define C2NG_CLIENT_WIDGETS_CONFIGVALUELIST_HPP

#include "game/config/configurationeditor.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    /** List of configuration values.
        Displays content of a ConfigurationEditor. */
    class ConfigValueList : public ui::widgets::AbstractListbox {
     public:
        /** Shortcut for Info vector. */
        typedef std::vector<game::config::ConfigurationEditor::Info> Infos_t;

        /** Constructor.
            @param root UI root (fonts, colors) */
        explicit ConfigValueList(ui::Root& root);

        /** Set highlighted source.
            An option having a source strictly higher than the given value will be highlighted in bold.
            For example, with setting System, all options up to System will be in regular font,
            options in User and Game are bold.

            Highlighting can be turned off by setting the value Game, which is the highest value.

            @param source Threshold; default is Game */
        void setHighlightedSource(game::config::ConfigurationEditor::Source source);

        /** Set width of name column in ems.
            This value is used for layout and drawing.
            @param widthInEms Width */
        void setNameColumnWidth(int widthInEms);

        /** Set width of value column in ems.
            This value is used for layout.
            @param widthInEms Width */
        void setValueColumnWidth(int widthInEms);

        /** Set preferred height in lines.
            This value is used for layout.
            @param numLines Height */
        void setPreferredHeight(int numLines);

        /** Set content.
            Exchanges all content at once.
            Use (at least) once on startup.
            @param infos New content */
        void setContent(const Infos_t& infos);

        /** Set content for a single item.
            Can be used as handler for ConfigurationEditorProxy::sig_itemChange.

            This function only updates existing items;
            out-of-range indexes are ignored.

            @param index 0-based index
            @param info  New value */
        void setItemContent(size_t index, const game::config::ConfigurationEditor::Info& info);

        /** Get currently-selected option.
            @return Option; null if none */
        const game::config::ConfigurationEditor::Info* getCurrentOption() const;

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
        Infos_t m_content;
        game::config::ConfigurationEditor::Source m_highlightedSource;
        int m_nameColumnWidth;
        int m_valueColumnWidth;
        int m_preferredHeight;
    };

} }

#endif
