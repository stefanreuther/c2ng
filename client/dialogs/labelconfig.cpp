/**
  *  \file client/dialogs/labelconfig.cpp
  *  \brief Label configuration dialog
  */

#include "client/dialogs/labelconfig.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/widgets/busyindicator.hpp"
#include "client/widgets/expressionlist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/expressionlistproxy.hpp"
#include "game/proxy/labelproxy.hpp"
#include "game/proxy/waitindicator.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

using game::config::ExpressionLists;
using game::proxy::ExpressionListProxy;

namespace {
    /*
     *  Input compound: two identical widget groups
     */
    struct Compound {
        ui::widgets::InputLine input;
        ui::widgets::Button button;

        Compound(ui::Root& root)
            : input(4096, 30, root),
              button(UTF_DOWN_ARROW, 0, root)
            { }
    };

    void addCompound(afl::base::Deleter& del, ui::Window& win, String_t title, Compound& comp)
    {
        ui::Root& root = comp.button.root();

        win.add(del.addNew(new ui::widgets::StaticText(title, util::SkinColor::Static, "+", root.provider())));

        ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance0));
        g.add(ui::widgets::FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, comp.input));
        g.add(comp.button);
        win.add(g);
    }

    /*
     *  Dialog
     *
     *  Confirming this dialog will wait for the LabelProxy to confirm the change,
     *  UI will be blocked during that time.
     *
     *  For now, this dialog doesn't protest too violently against invalid input.
     */

    class Dialog : private gfx::KeyEventConsumer {
     public:
        Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);
        void init(game::proxy::WaitIndicator& ind);
        void run();

        void onOK();
        void onCancel();
        void onPlanetDropdown();
        void onShipDropdown();
        void onDropdown(Compound& comp, ExpressionLists::Area area);
        void onConfigurationApplied(const game::proxy::LabelProxy::Status& st);

        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        void saveLRU();
        bool checkResult(const afl::base::Optional<String_t> error, Compound& comp, String_t msg);
        void showBusyIndicator();
        void hideBusyIndicator();

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        game::proxy::LabelProxy m_labelProxy;
        ui::EventLoop m_loop;

        Compound m_ship;
        Compound m_planet;

        client::widgets::BusyIndicator m_applyBlocker;
    };
}

Dialog::Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
    : m_root(root), m_translator(tx), m_gameSender(gameSender),
      m_labelProxy(gameSender, root.engine().dispatcher()),
      m_loop(root),
      m_ship(root),
      m_planet(root),
      m_applyBlocker(root, tx("Working..."))
{
    m_planet.input.setHotkey(util::KeyMod_Alt + 'p');
    m_ship.input.setHotkey(util::KeyMod_Alt + 's');
    m_planet.button.sig_fire.add(this, &Dialog::onPlanetDropdown);
    m_ship.button.sig_fire.add(this, &Dialog::onShipDropdown);
    m_labelProxy.sig_configurationApplied.add(this, &Dialog::onConfigurationApplied);
}

void
Dialog::init(game::proxy::WaitIndicator& ind)
{
    String_t shipExpr, planetExpr;
    m_labelProxy.getConfiguration(ind, shipExpr, planetExpr);
    m_ship.input.setText(shipExpr);
    m_planet.input.setText(planetExpr);
    saveLRU();
}

void
Dialog::run()
{
    // ex WLabelExprEditDialog::init
    afl::base::Deleter del;

    // Window [VBox]
    //   StaticText "Planets:"
    //   HBox
    //     InputLine
    //     Button dropdown
    //   StaticText "Ships:"
    //   HBox
    //     InputLine
    //     Button dropdown
    //   StandardDialogButtons

    ui::Window& win = del.addNew(new ui::Window(m_translator("Ship and Planet Labels"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    addCompound(del, win, m_translator("Planets"), m_planet);
    addCompound(del, win, m_translator("Ships"),   m_ship);

    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    win.add(btn);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:labels"));
    btn.addHelp(help);
    win.add(help);

    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    it.add(m_planet.input);
    it.add(m_ship.input);
    win.add(it);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    btn.ok().sig_fire.add(this, &Dialog::onOK);
    btn.cancel().sig_fire.add(this, &Dialog::onCancel);

    win.pack();
    m_planet.input.requestFocus();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

/* Event handler: OK button. Set configuration on LabelProxy and wait for onConfigurationApplied(). */
void
Dialog::onOK()
{
    // WLabelExprEditDialog::onOK()
    m_labelProxy.setConfiguration(m_ship.input.getText(), m_planet.input.getText());
    showBusyIndicator();
}

/* Event handler: Cancel button. */
void
Dialog::onCancel()
{
    // ex WLabelExprEditDialog::onCancel
    m_loop.stop(0);
}

/* Event handler: dropdown for planets */
void
Dialog::onPlanetDropdown()
{
    // WLabelExprEditDialog::onPlanetDropdown()
    onDropdown(m_planet, ExpressionLists::PlanetLabels);
}

/* Event handler: dropdown for ships */
void
Dialog::onShipDropdown()
{
    // WLabelExprEditDialog::onShipDropdown()
    onDropdown(m_ship, ExpressionLists::ShipLabels);
}

/* Common part of dropdown handlers */
void
Dialog::onDropdown(Compound& comp, ExpressionLists::Area area)
{
    // WLabelExprEditDialog::onDropdown(int slot)
    ExpressionListProxy exProxy(m_gameSender, area);
    String_t value = comp.input.getText();
    String_t flags;
    if (client::widgets::doExpressionListPopup(m_root, exProxy, comp.button.getExtent().getBottomLeft(), value, flags, m_translator)) {
        comp.input.setText(value);
    }
}

/* Event handler: configuration applied. Triggered by onOK > LabelProxy. */
void
Dialog::onConfigurationApplied(const game::proxy::LabelProxy::Status& st)
{
    hideBusyIndicator();
    if (checkResult(st.planetError, m_planet, m_translator("Your planet expression was not accepted."))
        && checkResult(st.shipError, m_ship, m_translator("Your ship expression was not accepted.")))
    {
        saveLRU();
        m_loop.stop(0);
    }
}

/* Event handler: key input */
bool
Dialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // WLabelExprEditDialog::handleEvent(const UIEvent& event, bool second_pass)
    if (key == util::Key_Down) {
        if (m_ship.input.getFocusState() == ui::Widget::PrimaryFocus) {
            onShipDropdown();
            return true;
        }
        if (m_planet.input.getFocusState() == ui::Widget::PrimaryFocus) {
            onPlanetDropdown();
            return true;
        }
    }
    return false;
}

/* Save current text in LRU lists */
void
Dialog::saveLRU()
{
    // ex saveLRU()
    ExpressionListProxy(m_gameSender, ExpressionLists::ShipLabels).
        pushRecent("", m_ship.input.getText());
    ExpressionListProxy(m_gameSender, ExpressionLists::PlanetLabels).
        pushRecent("", m_planet.input.getText());
}

/* Check result for one expression */
bool
Dialog::checkResult(const afl::base::Optional<String_t> error, Compound& comp, String_t msg)
{
    // ex WLabelExprEditDialog::setLabel (sort-of)
    if (const String_t* errMsg = error.get()) {
        // Not accepted
        util::rich::Text richMsg(msg);
        richMsg += "\n\n";
        richMsg +=
            util::rich::Text(m_translator("Error message: ")).withStyle(util::rich::StyleAttribute::Bold)
            .append(*errMsg)
            .append("\n\n")
            .append(m_translator("Use \"OK\" to review and correct the expression.")).withStyle(util::rich::StyleAttribute::Small);
        ui::dialogs::MessageBox box(richMsg, m_translator("Error"), m_root);
        box.addKey(0, util::Key_Escape);
        box.addKey(0, ' ');
        box.addButton(0, m_translator("OK"), util::Key_Return);
        box.addButton(1, util::KeyString("Ignore"));
        if (box.run() != 0) {
            // Ignore
            return true;
        } else {
            // OK (review)
            comp.input.requestFocus();
            return false;
        }
    } else {
        // Accepted
        return true;
    }
}

void
Dialog::showBusyIndicator()
{
    if (m_applyBlocker.getParent() == 0) {
        m_applyBlocker.setExtent(gfx::Rectangle(gfx::Point(), m_applyBlocker.getLayoutInfo().getPreferredSize()));
        m_root.moveWidgetToEdge(m_applyBlocker, gfx::CenterAlign, gfx::BottomAlign, 10);
        m_root.add(m_applyBlocker);
    }
}

void
Dialog::hideBusyIndicator()
{
    if (m_applyBlocker.getParent() != 0) {
        m_root.remove(m_applyBlocker);
        // do not replayEvents here; if there's an error, user shall read the dialog, not discard it by accident
    }
}

/*
 *  Main Entry Point
 */

void
client::dialogs::editLabelConfiguration(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    // ex editLabelExpressions
    Dialog dlg(root, tx, gameSender);
    Downlink link(root, tx);
    dlg.init(link);
    dlg.run();
}
