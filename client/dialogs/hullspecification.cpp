/**
  *  \file client/dialogs/hullspecification.cpp
  *  \brief Hull Specification Dialog
  */

#include "client/dialogs/hullspecification.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/dialogs/hullfunctionview.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/hullspecificationsheet.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/hullspecificationproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/shipinfoproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/math.hpp"

using afl::string::Format;
using game::proxy::ConfigurationProxy;
using game::proxy::HullSpecificationProxy;
using game::proxy::PlayerProxy;
using game::proxy::ShipInfoProxy;

namespace {
    /** Dialog.

        Content is loaded asynchronously from HullSpecificationProxy.
        Sub-dialogs are populated with synchronous requests. */
    class Dialog : private gfx::KeyEventConsumer {
     public:
        Dialog(HullSpecificationProxy& proxy, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender,
               game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames, util::NumberFormatter fmt, bool useIcons);

        void setShipId(game::Id_t shipId);

        void run(String_t title);

        bool handleKey(util::Key_t key, int prefix);

        void showWeaponEffects();
        void showHullFunctionDetails();

     private:
        HullSpecificationProxy& m_proxy;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        util::NumberFormatter m_numberFormatter;

        ui::EventLoop m_loop;
        client::widgets::HullSpecificationSheet m_widget;
        game::Id_t m_shipId;

        afl::base::SignalConnection conn_update;
    };

    /* Render single weapon effect.
       \param [out] doc       Document
       \param [in]  x         X location
       \param [in]  effect    Effect (e.g. beamEffects[].damageEffect etc.)
       \param [in]  limit     Limit (e.g. damageLimit)
       \param [in]  scale     Scale factor
       \param [in]  fmt       Number formatter */
    void renderEffect(ui::rich::Document& doc, const int x, int32_t effect, int32_t limit, int32_t scale, const util::NumberFormatter& fmt)
    {
        // ex showEffectI, showEffectF, shipspec.pas:ShowEffect
        if (effect == 0) {
            // Totally ineffective weapon
            doc.addCentered(x, util::rich::Text("-").withColor(util::SkinColor::Faded));
        } else {
            if (scale == 1) {
                // Integer value
                doc.addRight(x, fmt.formatNumber(effect));
            } else {
                // Float value
                doc.addRight(x, String_t(Format("%.2f", double(effect) / scale)));
            }
            doc.add(Format(" (%d" UTF_TIMES ")", fmt.formatNumber(util::divideAndRoundUp(limit * scale, effect))));
        }
    }

    /* Render a single weapon kind's effects (for all types).
       \param [out] doc          Document
       \param [in]  eff          Entire WeaponEffects structure
       \param [in]  areaEffects  Area to render
       \param [in]  fmt          NumberFormatter
       \param [in]  em           em-width for placing text */
    void renderWeaponArea(ui::rich::Document& doc,
                          const game::spec::info::WeaponEffects& eff,
                          const std::vector<game::spec::info::WeaponEffect>& areaEffects,
                          const util::NumberFormatter& fmt,
                          const int em)
    {
        for (size_t i = 0; i < areaEffects.size(); ++i) {
            doc.add(areaEffects[i].name);
            renderEffect(doc, 17*em, areaEffects[i].shieldEffect, 100,             eff.effectScale, fmt);
            renderEffect(doc, 25*em, areaEffects[i].damageEffect, eff.damageLimit, eff.effectScale, fmt);
            renderEffect(doc, 33*em, areaEffects[i].crewEffect,   eff.crew,        eff.effectScale, fmt);
            doc.addNewline();
        }
    }

    /* Render all weapon effects.
       \param [out] doc          Document
       \param [in]  eff          Entire WeaponEffects structure
       \param [in]  name         Player name
       \param [in]  fmt          NumberFormatter
       \param [in]  em           em-width for placing text
       \param [in]  tx           Translator */
    void renderWeaponEffects(ui::rich::Document& doc,
                             const game::spec::info::WeaponEffects& eff,
                             const String_t& name,
                             const util::NumberFormatter& fmt,
                             const int em,
                             afl::string::Translator& tx)
    {
        // ex CWeaponEffectWidget.Draw
        doc.setPageWidth(40 * em);
        doc.add(Format(tx("Effects on %d kt %s ship"), fmt.formatNumber(eff.mass), name));
        if (eff.usedESBRate != 0) {
            doc.add(Format(tx(" (using %d%% ESB)"), eff.usedESBRate));
        }
        doc.add(Format(tx(", %d crewm%1{a%|e%}n"), fmt.formatNumber(eff.crew)));
        doc.addParagraph();
        doc.addCentered(17*em, util::rich::Text(tx("Shield")).withStyle(util::rich::StyleAttribute::Bold));
        doc.addCentered(25*em, util::rich::Text(tx("Hull")).withStyle(util::rich::StyleAttribute::Bold));
        doc.addCentered(33*em, util::rich::Text(tx("Crew")).withStyle(util::rich::StyleAttribute::Bold));
        doc.addNewline();
        renderWeaponArea(doc, eff, eff.beamEffects, fmt, em);
        doc.addNewline();
        renderWeaponArea(doc, eff, eff.torpedoEffects, fmt, em);
        doc.addNewline();
        renderWeaponArea(doc, eff, eff.fighterEffects, fmt, em);
        doc.finish();
    }
}


/*
 *  Dialog
 */

Dialog::Dialog(HullSpecificationProxy& proxy, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender,
               game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames, util::NumberFormatter fmt, bool useIcons)
    : m_proxy(proxy),
      m_root(root),
      m_translator(tx),
      m_gameSender(gameSender),
      m_numberFormatter(fmt),
      m_loop(root),
      m_widget(root, tx, allPlayers, playerNames, fmt, useIcons),
      m_shipId(),
      conn_update(m_proxy.sig_update.add(&m_widget, &client::widgets::HullSpecificationSheet::setContent))
{ }

inline void
Dialog::setShipId(game::Id_t shipId)
{
    m_shipId = shipId;
}

void
Dialog::run(String_t title)
{
    // Window [VBox]
    //   HullSpecificationSheet
    //   HBox (Functions, Weapon || Close)
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));
    win.add(m_widget);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnFunc   = del.addNew(new ui::widgets::Button(m_translator("Functions"), 'f', m_root));
    ui::widgets::Button& btnWeapon = del.addNew(new ui::widgets::Button(m_translator("Weapons"),   'w', m_root));
    ui::widgets::Button& btnClose  = del.addNew(new ui::widgets::Button(m_translator("Close"),     util::Key_Escape, m_root));
    g.add(btnFunc);
    g.add(btnWeapon);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnClose);
    win.add(g);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnFunc.dispatchKeyTo(*this);
    btnWeapon.dispatchKeyTo(*this);

    win.pack();
    m_root.moveWidgetToEdge(win, gfx::RightAlign, gfx::MiddleAlign, 0);
    m_root.add(win);
    m_loop.run();
}

bool
Dialog::handleKey(util::Key_t key, int /*prefix*/)
{
    switch (key) {
     case ' ':
     case util::Key_Escape:
     case util::Key_Return:
        m_loop.stop(0);
        return true;

     case util::Key_F1:
     case 'f':
        showHullFunctionDetails();
        return true;

     case 'w':
        showWeaponEffects();
        return true;

     case 'h':
     case 'h' + util::KeyMod_Alt:
        client::dialogs::doHelpDialog(m_root, m_translator, m_gameSender, "pcc2:specsheet");
        return true;

     default:
        return false;
    }
}

void
Dialog::showWeaponEffects()
{
    // Retrieve data
    client::Downlink link(m_root, m_translator);
    game::spec::info::WeaponEffects eff;
    m_proxy.describeWeaponEffects(link, eff);

    String_t name = PlayerProxy(m_gameSender).getPlayerName(link, eff.player, game::Player::AdjectiveName);

    // Render into a DocumentView
    gfx::Point cellSize = m_root.provider().getFont(gfx::FontRequest())->getCellSize();
    ui::rich::DocumentView docView(cellSize.scaledBy(40, 2), 0, m_root.provider());
    ui::rich::Document& doc = docView.getDocument();

    renderWeaponEffects(doc, eff, name, m_numberFormatter, cellSize.getX(), m_translator);

    // Show dialog
    docView.adjustToDocumentSize();
    ui::dialogs::MessageBox(docView, m_translator("Weapon Effects"), m_root)
        .doOkDialog(m_translator);
}

void
Dialog::showHullFunctionDetails()
{
    // Retrieve data
    client::Downlink link(m_root, m_translator);
    game::spec::info::AbilityDetails_t details;
    m_proxy.describeHullFunctionDetails(link, details, true);

    game::map::ShipExperienceInfo expInfo;
    if (m_shipId > 0) {
        expInfo = ShipInfoProxy(m_gameSender).getExperienceInfo(link, m_shipId);
    }

    // Show dialog
    client::dialogs::showHullFunctions(details, expInfo, m_root, m_gameSender, m_translator);
}

void
client::dialogs::showHullSpecificationForShip(game::Id_t shipId, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    // ex WSpecView::doStandardDialog (sort-of), shipspec.pas:ShowShipSpecSheet
    HullSpecificationProxy proxy(gameSender, root.engine().dispatcher(), std::auto_ptr<game::spec::info::PictureNamer>(new PictureNamer()));
    Downlink link(root, tx);
    Dialog dlg(proxy, root, tx, gameSender,
               PlayerProxy(gameSender).getAllPlayers(link),
               PlayerProxy(gameSender).getPlayerNames(link, game::Player::AdjectiveName),
               ConfigurationProxy(gameSender).getNumberFormatter(link),
               ConfigurationProxy(gameSender).getOption(link, game::config::UserConfiguration::Display_HullfuncImages));
    proxy.setExistingShipId(shipId);
    dlg.setShipId(shipId);
    dlg.run(tx("Ship Specification"));
}

void
client::dialogs::showHullSpecification(const game::ShipQuery& q, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    HullSpecificationProxy proxy(gameSender, root.engine().dispatcher(), std::auto_ptr<game::spec::info::PictureNamer>(new PictureNamer()));
    Downlink link(root, tx);
    Dialog dlg(proxy, root, tx, gameSender,
               PlayerProxy(gameSender).getAllPlayers(link),
               PlayerProxy(gameSender).getPlayerNames(link, game::Player::AdjectiveName),
               ConfigurationProxy(gameSender).getNumberFormatter(link),
               ConfigurationProxy(gameSender).getOption(link, game::config::UserConfiguration::Display_HullfuncImages));
    proxy.setQuery(q);
    dlg.setShipId(q.getShipId());
    dlg.run(tx("Ship Specification"));
}
