/**
  *  \file client/widgets/pluginlist.hpp
  *  \brief Class client::widgets::PluginList
  */
#ifndef C2NG_CLIENT_WIDGETS_PLUGINLIST_HPP
#define C2NG_CLIENT_WIDGETS_PLUGINLIST_HPP

#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/plugin/manager.hpp"
#include "util/skincolor.hpp"

namespace client { namespace widgets {

    /** List of plugins.
        Displays a list of two-line items, each describing a plugin:
        - plugin name in normal font
        - id and status in small font */
    class PluginList : public ui::widgets::AbstractListbox {
     public:
        typedef util::plugin::Manager::Info Info_t;
        typedef util::plugin::Manager::Infos_t Infos_t;

        /** Constructor.
            @param root  UI root
            @param tx    Translator */
        PluginList(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~PluginList();

        /** Set content.
            Attempts to keep the current plugin in focus.
            @param content New content; will be copied */
        void setContent(const Infos_t& content);

        /** Mark widget loading.
            Call when you know that content will take a while to arrive.
            In this status, the widget will display nothing instead of "no plugins loaded". */
        void setLoading();

        /** Get currently-selected plugin.
            @return pointer to object; null if none. */
        const Info_t* getCurrentPlugin() const;

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Infos_t m_content;
        bool m_loading;

        afl::base::Ref<gfx::Font> getNormalFont() const;
        afl::base::Ref<gfx::Font> getTitleFont() const;
        afl::base::Ref<gfx::Font> getSubtitleFont() const;
        int getItemHeight() const;
    };

    /** Format subtitle of plugin information.
        Produces a piece of text of the form "(id, status)".
        @param [out] out   result
        @param [in]  in    Plugin information provided by plugin manager
        @param [in]  tx    Translator
        @return suggested skin color to format the result */
    util::SkinColor::Color formatSubtitle(String_t& out, const util::plugin::Manager::Info& in, afl::string::Translator& tx);

} }

#endif
