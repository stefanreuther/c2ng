/**
  *  \file client/dialogs/pluginmanager.hpp
  *  \brief Class client::dialogs::PluginManager
  */
#ifndef C2NG_CLIENT_DIALOGS_PLUGINMANAGER_HPP
#define C2NG_CLIENT_DIALOGS_PLUGINMANAGER_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Plugin Manager Dialog.
        Provides an environment for the plugin manager dialog.
        User must provide callbacks to load and unload plugins.

        Create an instance and call run() to operate the dialog. */
    class PluginManager {
     public:
        /** Constructor.
            @param root        UI root
            @param gameSender  Game sender
            @param tx          Translator */
        PluginManager(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        /** Execute the dialog. */
        void run();

        /** Callback: unload plugin.
            Unloading means removing references to resources attributed to that plugin (e.g. resource files).
            Actual plugin management (i.e. removal from the plugin manager) is done by the dialog.

            This function should return when the plugin is unloaded,
            and needs to block UI during that time.

            @param id Plugin Id */
        virtual void unloadPlugin(const String_t& id) = 0;

        /** Callback: load plugin.
            At this time, the plugin is present in the plugin manager.

            This function should return when the plugin is loaded,
            and needs to block UI during that time.
            It should mark the plugin loaded in the plugin manager.

            @param id Plugin Id */
        virtual void loadPlugin(const String_t& id) = 0;

        /** Access UI root.
            @return root */
        ui::Root& root();

        /** Access game sender.
            @return game sender */
        const util::RequestSender<game::Session>& gameSender();

        /** Access translator.
            @return translator */
        afl::string::Translator& translator();

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
    };

} }

#endif
