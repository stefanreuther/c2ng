/**
  *  \file client/dialogs/buildshipmain.hpp
  *  \brief Class client::dialogs::BuildShipMain
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSHIPMAIN_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSHIPMAIN_HPP

#include "afl/base/deleter.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/componentlist.hpp"
#include "client/widgets/itemcostdisplay.hpp"
#include "game/proxy/basestorageproxy.hpp"
#include "game/proxy/buildshipproxy.hpp"
#include "game/proxy/specbrowserproxy.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/info/types.hpp"
#include "game/types.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/numberformatter.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Build Ship Dialog - Main Part.

        Represents part of a dialog to configure a ship for building.
        This class takes over the bulk of widget building and event handling.

        We use...
        - a BuildShipProxy to set up the build order.
        - a BaseStorageProxy to get the list of components.
        - a SpecBrowserProxy to obtain the current component's specs.

        Lists of components are retrieved once and kept up-to-date.
        Whenever focus changes, the new component is selected on the BuildShipProxy and the SpecBrowserProxy;
        corresponding updates are received asynchronously.

        To use,
        - create a BuildShipProxy and BaseStorageProxy
        - call init()
        - call buildDialog() to build the bulk of the dialog
        - add additional buttons you may need, in particular "OK"/"Cancel" buttons,
          help, and ui::widgets::Quit
        - run the dialog */
    class BuildShipMain {
     public:
        /** Constructor.
            @param root          UI root
            @param buildProxy    BuildShipProxy
            @param storageProxy  BaseStorageProxy
            @param gameSender    Game sender, for additional proxies (TechUpgradeProxy, SpecBrowserProxy, ConfigurationProxy)
            @param planetId      Planet Id. If given as nonzero, dialog will offer part building using a BuildPartsProxy.
            @param tx            Translator */
        BuildShipMain(ui::Root& root,
                      game::proxy::BuildShipProxy& buildProxy,
                      game::proxy::BaseStorageProxy& storageProxy,
                      util::RequestSender<game::Session> gameSender,
                      game::Id_t planetId,
                      afl::string::Translator& tx);

        /** Initialize dialog.
            This will retrieve the current status from the game side,
            and create widgets depending on that state.
            @param del  Deleter to control lifetime of created objects */
        void init(afl::base::Deleter& del);

        /** Build dialog.
            This will build the main part of the dialog.
            @param del   Deleter to control lifetime of created objects
            @param title Window title
            @return ui::Window instance in VBox layout, containing most widgets */
        ui::Window& buildDialog(afl::base::Deleter& del, String_t title);

        /** Make a button to display the Detailed Bill.
            @param del Deleter to control lifetime of created objects
            @return button */
        ui::Widget& makeDetailedBillButton(afl::base::Deleter& del);

        /** Make a help widget.
            This is just a convenience method because we already have all required dependencies.
            @param del Deleter to control lifetime of created objects
            @param helpId Help page Id
            @return widget */
        ui::Widget& makeHelpWidget(afl::base::Deleter& del, String_t helpId);

        /** Access UI root.
            @return root */
        ui::Root& root()
            { return m_root; }

        /** Access translator.
            @return translator */
        afl::string::Translator& translator()
            { return m_translator; }

        /** Access game sender.
            @return game sender */
        util::RequestSender<game::Session> gameSender()
            { return m_gameSender; }

        /** Get planet Id.
            @return planet Id as passed to constructor */
        game::Id_t getPlanetId() const
            { return m_planetId; }

        /** Signal: change of current build order status.
            Called whenever the status changes, either by an event from the BuildShipProxy, or from init(). */
        afl::base::Signal<void(const game::proxy::BuildShipProxy::Status&)> sig_change;

     private:
        // UI actions
        void onDetailedBill();
        void onHullSpecification();
        void onBuildParts();
        void onChooseHull();
        void addBeam();
        void removeBeam();
        void addLauncher();
        void removeLauncher();

        // Updates
        void onStorageUpdate(game::TechLevel area, const game::proxy::BaseStorageProxy::Parts_t& parts);
        void onOrderUpdate(const game::proxy::BuildShipProxy::Status& st);
        void onSelectionChange();
        void onSpecificationChange(const game::spec::info::PageContent& content);

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;

        game::proxy::BuildShipProxy& m_buildProxy;
        game::proxy::BaseStorageProxy& m_storageProxy;
        game::proxy::SpecBrowserProxy m_specProxy;
        game::Id_t m_planetId;

        // Widgets:
        client::widgets::ComponentList* m_pComponentList[game::NUM_TECH_AREAS];
        ui::widgets::ImageButton* m_pImageButtons[game::NUM_TECH_AREAS];
        ui::rich::DocumentView* m_pSpecificationDisplay[game::NUM_TECH_AREAS];
        ui::widgets::StaticText* m_pInStorage[game::NUM_TECH_AREAS];
        ui::rich::DocumentView m_orderDisplay;
        client::widgets::ItemCostDisplay m_costDisplay;
        ui::widgets::StaticText m_numEngines;
        ui::widgets::StaticText m_numBeams;
        ui::widgets::StaticText m_numLaunchers;
        ui::widgets::Button m_moreBeams;
        ui::widgets::Button m_fewerBeams;
        ui::widgets::Button m_moreLaunchers;
        ui::widgets::Button m_fewerLaunchers;

        // State
        util::NumberFormatter m_formatter;
        bool m_useIcons;
        game::spec::info::Page m_specPage;
        game::Id_t m_specId;
        int m_currentHull;
        game::spec::Cost m_availableAmount;

        // Signal connections
        // During destruction, we get focus change events.
        // Disconnect these first so they do not cause rendering, which could access a proxy that is already gone.
        afl::base::SignalConnection conn_componentSelectionChange[game::NUM_TECH_AREAS];
        afl::base::SignalConnection conn_mainSelectionChange;

        // Widget building
        ui::Widget& wrapComponentList(afl::base::Deleter& del, game::TechLevel area);
        ui::Widget& makeStorageColumn(afl::base::Deleter& del, game::TechLevel area);
        ui::Widget& makeWeaponInfoGroup(afl::base::Deleter& del, ui::rich::DocumentView& specDisplay, ui::widgets::StaticText& num, ui::widgets::Button& more, ui::widgets::Button& fewer);

        // UI helpers
        void setCursors(const game::proxy::BuildShipProxy::Status& st);
        bool checkTechUpgrade(game::proxy::WaitIndicator& ind, game::TechLevel area, int level);
        void renderBuildOrder(const game::proxy::BuildShipProxy::Status& st);
        void renderSpecification(game::TechLevel area, const game::spec::info::PageContent& content);
        void updateBuildOrder();

        game::TechLevel getCurrentArea() const;
        game::spec::info::Page getCurrentPage() const;
    };

} }

#endif
