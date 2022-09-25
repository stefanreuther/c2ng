/**
  *  \file client/widgets/plugininfo.hpp
  *  \brief Class client::widgets::PluginInfo
  */
#ifndef C2NG_CLIENT_WIDGETS_PLUGININFO_HPP
#define C2NG_CLIENT_WIDGETS_PLUGININFO_HPP

#include "afl/string/translator.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "util/plugin/manager.hpp"

namespace client { namespace widgets {

    /** Display information about a plugin.
        Contains logic to format and display a util::plugin::Manager::Details object.

        In addition, contains logic to deal with content taking a while to arrive,
        i.e. plugin information loaded from a network.
        If information takes a while, the widget is cleared instead of displaying stale information. */
    class PluginInfo : public ui::rich::DocumentView {
     public:
        /** Constructor.
            @param root UI root
            @param tx   Translator */
        PluginInfo(ui::Root& root, afl::string::Translator& tx);
        ~PluginInfo();

        /** Set content.
            Should be called after the widget has received its final size.
            @param d content */
        void setContent(const util::plugin::Manager::Details& d);

        /** Mark widget loading.
            Call when you know that content will take a while to arrive.
            If content remains unavailable for too long, clears the widget. */
        void setLoading();

     private:
        void render();
        void onTimer();

        afl::string::Translator& m_translator;
        util::plugin::Manager::Details m_content;
        afl::base::Ref<gfx::Timer> m_timer;
        bool m_timerRunning;
    };

} }

#endif
