/**
  *  \file client/dialogs/taxationdialog.cpp
  *  \brief Taxation Dialog
  */

#include "client/dialogs/taxationdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/proxy/taxationproxy.hpp"
#include "game/tables/happinesschangename.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

using client::proxy::TaxationProxy;
using game::actions::TaxationAction;

namespace {
    TaxationAction::Areas_t allAreas()
    {
        return TaxationAction::Areas_t() + TaxationAction::Colonists + TaxationAction::Natives;
    }

    /*
     *  TaxationKeyWidget: dispatch keys
     */
    class TaxationKeyWidget : public ui::InvisibleWidget {
     public:
        TaxationKeyWidget(TaxationProxy& proxy, TaxationAction::Area area);
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        TaxationProxy& m_proxy;
        TaxationAction::Area m_area;
    };

    /*
     *  TaxationWidget: widget assembly for one taxation area
     *
     *  Displays the content of a TaxationProxy::AreaStatus:
     *  - heading
     *  - textual information
     *  - "+"/"-" buttons
     *
     *  Size is 30em x 5 lines for both.
     *
     *  PCC2: 380px x 3 lines for colonists, 380px x 5 lines for natives.
     */
    class TaxationWidget : public ui::widgets::FocusableGroup {
     public:
        TaxationWidget(afl::base::Deleter& del, afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, TaxationAction::Area area);
        void setContent(const TaxationProxy::AreaStatus& st);

     private:
        afl::string::Translator& m_translator;
        ui::widgets::StaticText* m_pTitle;
        ui::rich::DocumentView* m_pInfo;
    };

    /*
     *  TaxationDialog: entire dialog
     */
    class TaxationDialog {
     public:
        TaxationDialog(afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy);
        bool run(const TaxationProxy::Status& initialStatus);
        void update(const TaxationProxy::Status& st);

     private:
        afl::string::Translator& m_translator;
        ui::Root& m_root;
        TaxationProxy& m_proxy;
        TaxationWidget* m_pNativeTaxes;
        TaxationWidget* m_pColonistTaxes;
        afl::base::SignalConnection conn_change;
    };
}


/*
 *  TaxationKeyWidget implementation
 */

TaxationKeyWidget::TaxationKeyWidget(TaxationProxy& proxy, TaxationAction::Area area)
    : m_proxy(proxy),
      m_area(area)
{ }

bool
TaxationKeyWidget::handleKey(util::Key_t key, int prefix)
{
    // ex WColonistTaxSelector::handleEvent, WNativeTaxSelector::handleEvent, UINumberSelector::handleEvent (part)
    switch (key) {
     case '-':
     case util::Key_Left:
        // Decrement
        m_proxy.changeTax(m_area, prefix ? -prefix : -1);
        return true;

     case '+':
     case util::Key_Right:
        // Increment
        m_proxy.changeTax(m_area, prefix ? +prefix : +1);
        return true;

     case util::KeyMod_Ctrl + '-':
     case util::KeyMod_Ctrl + util::Key_Left:
     case util::KeyMod_Alt + '-':
     case util::KeyMod_Alt + util::Key_Left:
        // Min
        m_proxy.setTaxLimited(m_area, 0);
        return true;

     case util::KeyMod_Ctrl + '+':
     case util::KeyMod_Ctrl + util::Key_Right:
     case util::KeyMod_Alt + '+':
     case util::KeyMod_Alt + util::Key_Right:
        // Max
        m_proxy.setTaxLimited(m_area, 100);
        return true;

     case util::KeyMod_Shift + util::Key_Left:
        // Decrease until income changes
        m_proxy.changeRevenue(m_area, TaxationAction::Down);
        return true;

     case util::KeyMod_Shift + util::Key_Right:
        // Increase until income changes
        m_proxy.changeRevenue(m_area, TaxationAction::Up);
        return true;

     case ' ':
        // Auto-tax all
        m_proxy.setSafeTax(allAreas());
        return true;

     case ' ' + util::KeyMod_Shift:
        // Auto-tax area
        m_proxy.setSafeTax(TaxationAction::Areas_t(m_area));
        return true;

     case '=':
     case '%':
        if (prefix != 0) {
            m_proxy.setTaxLimited(m_area, prefix);
        }
        return true;

     case 'u':
        // Undo
        m_proxy.revert(TaxationAction::Areas_t(m_area));
        return true;

     default:
        return false;
    }
}


/*
 *  TaxationWidget implementation
 */

TaxationWidget::TaxationWidget(afl::base::Deleter& del, afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy, TaxationAction::Area area)
    : FocusableGroup(ui::layout::HBox::instance5, 5),
      m_translator(tx),
      m_pTitle(&del.addNew(new ui::widgets::StaticText(String_t("?"), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider()))),
      m_pInfo(&del.addNew(new ui::rich::DocumentView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 5), 0, root.provider())))
{
    // HBox
    //   VBox
    //     Title
    //     Info
    //   VBox
    //     "+"
    //     "-"
    //     Spacer
    TaxationKeyWidget& keys = del.addNew(new TaxationKeyWidget(proxy, area));
    add(keys);
    add(del.addNew(new ui::PrefixArgument(root)));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g1.add(*m_pTitle);
    g1.add(*m_pInfo);
    add(g1);

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::widgets::Button& btnInc = del.addNew(new ui::widgets::Button("+", '+', root));
    ui::widgets::Button& btnDec = del.addNew(new ui::widgets::Button("-", '-', root));
    g2.add(btnInc);
    g2.add(btnDec);
    g2.add(del.addNew(new ui::Spacer()));

    btnInc.dispatchKeyTo(keys);
    btnDec.dispatchKeyTo(keys);

    add(g2);
}

void
TaxationWidget::setContent(const TaxationProxy::AreaStatus& st)
{
    // ex WColonistTaxSelector::drawContent, WNativeTaxSelector::drawContent, showChange
    // Title
    m_pTitle->setText(st.title);

    // Info
    ui::rich::Document& doc = m_pInfo->getDocument();
    doc.clear();
    doc.setPageWidth(m_pInfo->getExtent().getWidth());
    doc.add(afl::string::Format(m_translator("Tax Rate: %d%%"), st.tax));
    doc.add(" " UTF_EM_DASH " ");

    String_t changeLabel = st.changeLabel;
    if (st.change != 0) {
        changeLabel += afl::string::Format(" (%d)", st.change);
    }
    doc.add(util::rich::Text(st.change < 0 ? util::SkinColor::Red : util::SkinColor::Green, changeLabel));
    doc.addParagraph();
    doc.add(st.description);
    doc.finish();
    m_pInfo->handleDocumentUpdate();
}


/*
 *  TaxationDialog implementation
 */

TaxationDialog::TaxationDialog(afl::string::Translator& tx, ui::Root& root, TaxationProxy& proxy)
    : m_translator(tx),
      m_root(root),
      m_proxy(proxy),
      m_pNativeTaxes(0),
      m_pColonistTaxes(0),
      conn_change(proxy.sig_change.add(this, &TaxationDialog::update))
{ }

bool
TaxationDialog::run(const TaxationProxy::Status& initialStatus)
{
    // ex WTaxationDialog::init
    // VBox
    //   VBox
    //     Colonists
    //     Natives
    //   HBox
    //     "OK", "Cancel", "Space", "Help"
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Taxes"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Taxation widgets.
    // We put these into a separate group to have a safe target for the "Space" button.
    // The button needs to route its keypress into the currently-focused widget.
    // We cannot route the key into /win/ because that would possibly trigger the button again.
    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab + ui::widgets::FocusIterator::Vertical));
    ui::Group& top = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    if (initialStatus.colonists.available) {
        TaxationWidget& colTax = del.addNew(new TaxationWidget(del, m_translator, m_root, m_proxy, TaxationAction::Colonists));
        top.add(colTax);
        it.add(colTax);
        m_pColonistTaxes = &colTax;
    }
    if (initialStatus.natives.available) {
        TaxationWidget& natTax = del.addNew(new TaxationWidget(del, m_translator, m_root, m_proxy, TaxationAction::Natives));
        top.add(natTax);
        it.add(natTax);
        m_pNativeTaxes = &natTax;
    }
    win.add(top);

    // Buttons
    ui::EventLoop loop(m_root);
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(m_translator("OK"),     util::Key_Return, m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
    ui::widgets::Button& btnAuto   = del.addNew(new ui::widgets::Button(m_translator("Space - Auto Tax"), ' ', m_root));
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    // FIXME: add(h.add(new WHelpWidget("pcc2:taxes")));
    // FIXME: port this:
    // if (host.isPHost()) {
    //     p_growth = &h.add(new WPlanetGrowthTile(planet));
    //     add(*p_growth);
    //     p_growth->setStructures(str_Mines, mi);
    //     p_growth->setStructures(str_Factories, fa);
    //     ctax.sig_changed.add(this, &WTaxationDialog::onChange);
    //     if (p_ntax)
    //         p_ntax->sig_changed.add(this, &WTaxationDialog::onChange);
    // }
    g.add(btnOK);
    g.add(btnCancel);
    g.add(btnAuto);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnHelp);
    win.add(g);
    win.add(it);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.pack();

    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    btnAuto.dispatchKeyTo(top);

    m_root.centerWidget(win);

    // Set initial content (after layout so word-wrap works ok)
    update(initialStatus);

    m_root.add(win);
    return loop.run() != 0;
}

void
TaxationDialog::update(const TaxationProxy::Status& st)
{
    if (m_pNativeTaxes != 0) {
        m_pNativeTaxes->setContent(st.natives);
    }
    if (m_pColonistTaxes != 0) {
        m_pColonistTaxes->setContent(st.colonists);
    }
}

/*
 *  Main Entry
 */
void
client::dialogs::doTaxationDialog(game::Id_t planetId,
                                  afl::base::Optional<int> numBuildings,
                                  ui::Root& root,
                                  afl::string::Translator& tx,
                                  util::RequestSender<game::Session> gameSender)
{
    // ex doTaxation
    // Set up proxy
    TaxationProxy proxy(root.engine().dispatcher(), gameSender, planetId);
    if (const int* p = numBuildings.get()) {
        proxy.setNumBuildings(*p);
    }

    // Check status
    TaxationProxy::Status st;
    Downlink link(root);
    proxy.getStatus(link, st);
    if (!st.valid) {
        return;
    }

    // Build dialog
    TaxationDialog dlg(tx, root, proxy);
    if (dlg.run(st)) {
        proxy.commit();
    }
}
