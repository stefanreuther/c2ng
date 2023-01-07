/**
  *  \file client/screens/browserscreen.hpp
  *  \brief Class client::screens::BrowserScreen
  */
#ifndef C2NG_CLIENT_SCREENS_BROWSERSCREEN_HPP
#define C2NG_CLIENT_SCREENS_BROWSERSCREEN_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/folderlistbox.hpp"
#include "game/proxy/browserproxy.hpp"
#include "game/session.hpp"
#include "gfx/timer.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/simpleiconbox.hpp"
#include "util/requestreceiver.hpp"
#include "util/rich/text.hpp"

namespace client { namespace screens {

    /** Browser screen.
        Operates on a game::proxy::BrowserProxy to select a game.

        The browser session must have been pre-initialized by the caller:
        - configure the browser instance
        - place the browser instance in a valid folder

        BrowserScreen will let the user deal with the browser session and select new folders etc.
        When a game is selected for loading, sig_gameSelection will be raised. */
    class BrowserScreen {
     public:
        /** Constructor.
            Prepares a BrowserScreen.
            \param us UserSide
            \param proxy Browser proxy for most operations */
        BrowserScreen(client::si::UserSide& us, game::proxy::BrowserProxy& proxy);

        /** Display this screen.
            Returns when the user cancels the dialog.
            \return Exit code. 0 if user cancelled normally, otherwise, parameter of stop(). */
        int run(gfx::ColorScheme<util::SkinColor::Color>& parentColors);

        /** Callback: stop this screen.
            \param n Exit code. */
        void stop(int n);

        /** Block the user interface.
            Can be used from a callback. */
        void setBlockState(bool flag);

        /** Get sender.
            This can be used to send requests to this object. */
        util::RequestSender<BrowserScreen> getSender();

        /** Signal: game selected.
            At this time, the browser will have a selected child and root (game::browser::Browser::getSelectedChildIndex(), game::browser::Browser::getSelectedRoot())
            which identify the game to play, and that game will have the given player number.
            The BrowserScreen will still be running.
            \param player Player number */
        afl::base::Signal<void(int)> sig_gameSelection;

     private:
        enum State {
            Working,            // Folder list is working, info is current
            WorkingLoad,        // Folder list is working, loading info
            Blocked,            // Folder list is blocked, info is empty, loading folder
            Disabled            // Folder list is blocked, info is empty, loading folder
        };

        enum InfoAction {
            NoAction,
            PlayAction,
            FolderAction,
            RootAction
        };

        struct InfoItem {
            util::rich::Text text;
            String_t iconName;
            InfoAction action;
            int actionParameter;
            InfoItem(util::rich::Text text, String_t iconName, InfoAction action, int actionParameter)
                : text(text), iconName(iconName), action(action), actionParameter(actionParameter)
                { }
        };

        bool isUpLink(size_t index) const;
        size_t getIndex(size_t index) const;
        game::proxy::BrowserProxy::OptionalIndex_t getEffectiveIndex(size_t index) const;

        void requestLoad();

        void onItemDoubleClicked(size_t nr);
        void onCrumbClicked(size_t nr);
        void onTimer();
        void onListMoved();
        void onKeyTab(int);
        void onKeyEnter(int);
        void onKeyLeft(int);
        void onKeyHelp(int);
        void onKeyPlugin(int);
        void onKeyQuit(int);
        void onAddAccount(int);
        void onRootAction(size_t index);
        bool preparePlayAction(size_t index);

        void setState(State st);
        void setList(client::widgets::FolderListbox::Items_t& items,
                     ui::widgets::SimpleIconBox::Items_t& crumbs,
                     size_t index,
                     bool hasUp);
        void onUpdate(const game::proxy::BrowserProxy::Info& info);
        void onSelectedInfoUpdate(game::proxy::BrowserProxy::OptionalIndex_t index, const game::proxy::BrowserProxy::FolderInfo& info);

        void buildInfo();

        client::si::UserSide& m_userSide;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<BrowserScreen> m_receiver;
        game::proxy::BrowserProxy& m_proxy;

        client::widgets::FolderListbox m_list;
        ui::widgets::SimpleIconBox m_crumbs;
        ui::widgets::RichListbox m_info;
        ui::widgets::Button m_optionButton;
        afl::container::PtrVector<InfoItem> m_infoItems;
        size_t m_infoIndex;
        game::Root::Actions_t m_infoActions;
        ui::EventLoop m_loop;

        bool m_hasUp;
        State m_state;
        bool m_blockState;
        afl::base::Ref<gfx::Timer> m_timer;

        afl::base::SignalConnection conn_browserUpdate;
        afl::base::SignalConnection conn_browserSelectedInfoUpdate;
    };

} }

#endif
