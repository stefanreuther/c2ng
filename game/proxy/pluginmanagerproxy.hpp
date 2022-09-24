/**
  *  \file game/proxy/pluginmanagerproxy.hpp
  *  \brief Class game::proxy::PluginManagerProxy
  */
#ifndef C2NG_GAME_PROXY_PLUGINMANAGERPROXY_HPP
#define C2NG_GAME_PROXY_PLUGINMANAGERPROXY_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "game/session.hpp"
#include "util/plugin/installer.hpp"
#include "util/plugin/manager.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Proxy for managing plugins.
        - bidirectional, asynchronous access to util::plugin::Manager
        - bidirectional, synchronous access to util::plugin::Installer

        Browsing plugins:
        - call requestList(), requestDetails()
        - responses on sig_list, sig_details

        Install plugins:
        - call prepareInstall() and examine result
        - call doInstall() to execute
        - this will not actually load the plugin, only update its status in the plugin manager

        Remove plugins:
        - call prepareRemove() and examine result
        - call doRemove() to execute
        - this will not actually unload the plugin, only remove it from the plugin manager */
    class PluginManagerProxy {
     public:
        /** Information about a pending installation. */
        struct InstallInfo {
            /** Success status.
                If this is true, all the following fields except for errorMessage are valid.
                If this is false, only the errorMessage is valid. */
            bool isValid;

            /** Update status. */
            bool isUpdate;

            /** Ambiguity status.
                - NoPlugin: no ambiguity
                - OnePlugin: one ambiguity. Consider calling prepareInstall(altName).
                - MultiplePlugins: multiple ambiguities. */
            util::plugin::Installer::ScanResult ambiguity;

            /** Error message.
                Valid if isValid=false, but can be empty.
                The usual cause for an empty errorMessage is that the plugin file has no supported format. */
            String_t errorMessage;

            /** File name as passed to prepareInstall(). */
            String_t fileName;

            /** File title (basename) as passed to prepareInstall(). */
            String_t fileTitle;

            /** Alternative file name.
                Valid if ambiguity=OnePlugin. */
            String_t altName;

            /** Alternative file title (basename).
                Valid if ambiguity=OnePlugin. */
            String_t altTitle;

            /** Plugin Id. */
            String_t pluginId;

            /** Plugin name (human-readable). */
            String_t pluginName;

            /** Plugin description (human-readable, long). */
            String_t pluginDescription;

            /** Human-readable list of conflicts, if any. */
            afl::base::Optional<String_t> conflicts;

            InstallInfo()
                : isValid(), isUpdate(), ambiguity(), errorMessage(), fileName(), fileTitle(), altName(), altTitle(), conflicts()
                { }
        };

        /** Result of an installation. */
        struct InstallResult {
            /** Success status.
                If this is true, all the following fields except for errorMessage are valid.
                If this is false, only the errorMessage is valid. */
            bool isValid;

            /** Id of the plugin that was installed.
                Caller must now cause it to be loaded. */
            String_t pluginId;

            /** Error message.
                Valid if isValid=false, but can be empty. */
            String_t errorMessage;

            InstallResult()
                : isValid(), pluginId(), errorMessage()
                { }
        };

        /** Result of an uninstallation (preparation or execution). */
        struct RemoveResult {
            /** Success status.
                If this is false, the errorMessage is valid. */
            bool isValid;

            /** Error message.
                Valid if isValid=false, but can be empty. */
            String_t errorMessage;

            RemoveResult()
                : isValid(), errorMessage()
                { }
        };

        /** Shortcut typedef. */
        typedef util::plugin::Manager::Infos_t Infos_t;

        /** Shortcut typedef. */
        typedef util::plugin::Manager::Details Details_t;


        /** Constructor.
            @param gameSender  Game sender
            @param reply       RequestDispatcher to receive updates in this thread */
        PluginManagerProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~PluginManagerProxy();

        /** Request list of plugins.
            The list will eventually arrive on sig_list. */
        void requestList();

        /** Request details about a plugin.
            The information will eventually arrive on sig_details.
            This function is de-bounced, such that multiple calls cause only one response; responses are not queued.
            @param id plugin Id
            @see util::plugin::Manager::describePlugin */
        void requestDetails(const String_t& id);

        /** Prepare installation.
            This loads the plugin definition and checks whether it can be installed.
            @param [in,out] ind        WaitIndicator for UI synchronisation
            @param [in]     fileName   File name
            @see util::plugin::Installer::prepareInstall
            @see util::plugin::Installer::checkInstallAmbiguity
            @see util::plugin::Installer::checkInstallPreconditions */
        InstallInfo prepareInstall(WaitIndicator& ind, const String_t& fileName);

        /** Perform installation.
            Must be called after prepareInstall().
            @param [in,out] ind        WaitIndicator for UI synchronisation
            @see util::plugin::Installer::doInstall */
        InstallResult doInstall(WaitIndicator& ind);

        /** Prepare uninstallation.
            This checks whether the plugin can be removed.
            @param [in,out] ind        WaitIndicator for UI synchronisation
            @param [in]     id         Plugin Id */
        RemoveResult prepareRemove(WaitIndicator& ind, const String_t& id);

        /** Perform uninstallation.
            Should be called after prepareRemove().
            @param [in,out] ind        WaitIndicator for UI synchronisation
            @param [in]     id         Plugin Id */
        RemoveResult doRemove(WaitIndicator& ind, const String_t& id);

        /** Cancel installation.
            Call after prepareInstall() if you decide that you don't wish to proceed with the installation.
            This frees resources associated with the current installation. */
        void cancelInstallation();

        /** Signal: updated plugin list.
            @param list List */
        afl::base::Signal<void(const Infos_t&)> sig_list;

        /** Signal: updated plugin details.
            @param d Plugin details */
        afl::base::Signal<void(const Details_t&)> sig_details;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<PluginManagerProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        bool m_detailsRequested;
        afl::base::Optional<String_t> m_detailRequest;

        void sendDetailRequest();
        void handleDetails(const Details_t& d);
    };

} }

#endif
