/**
  *  \file client/dialogs/cargotransferdialog.cpp
  */

#include "client/dialogs/cargotransferdialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/cargotransferheader.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/res/resid.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "util/rich/parser.hpp"

client::dialogs::CargoTransferDialog::CargoTransferDialog(ui::Root& root, afl::string::Translator& tx, game::proxy::CargoTransferProxy& proxy)
    : m_root(root),
      m_translator(tx),
      m_proxy(proxy),
      m_loop(root),
      m_lines(),
      m_sellSupplies(false),
      m_overloadCheckbox(root, 'o', tx("Overload mode"), gfx::Point(20, 20)),
      m_overload(false)
{
    proxy.sig_change.add(this, &CargoTransferDialog::onChange);
    m_overloadCheckbox.setImage(RESOURCE_ID("ui.cb0"));
    m_overloadCheckbox.sig_fire.add(this, &CargoTransferDialog::onEnableOverload);
    m_overloadCheckbox.setIsFocusable(false);
}

bool
client::dialogs::CargoTransferDialog::run(util::RequestSender<game::Session> gameSender)
{
    // ex transfer.pas:CargoTransfer
    afl::string::Translator& tx = m_translator;

    Downlink link(m_root, tx);
    game::proxy::CargoTransferProxy::General gen;
    m_proxy.getGeneralInformation(link, gen);

    game::proxy::CargoTransferProxy::Participant left, right;
    m_proxy.getParticipantInformation(link, 0, left);
    m_proxy.getParticipantInformation(link, 1, right);

    util::NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    if (gen.validTypes.empty()) {
        ui::dialogs::MessageBox(afl::string::Format(tx("There is nothing you could transfer to or from %s."), right.name),
                                tx("Cargo Transfer"), m_root).doOkDialog(tx);
        return false;
    }

    afl::base::Deleter del;
    ui::Window& win(del.addNew(new ui::Window(tx("Cargo Transfer"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));
    win.add(del.addNew(new client::widgets::CargoTransferHeader(m_root, tx, left.name, right.name)));

    ui::Group& lineGroup = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    ui::widgets::FocusIterator& iter = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical + ui::widgets::FocusIterator::Wrap));
    game::ElementTypes_t validTypes = gen.validTypes;
    game::Element::Type type = game::Element::Type(0);
    while (!validTypes.empty()) {
        // @change This displays lines in order of Element::Type.
        // PCC2 has an explicit table, which differs from the Element::Type order
        // by swapping colonists and supplies. I don't think that's significant.
        if (validTypes.contains(type)) {
            String_t name = gen.typeNames.get(type);
            String_t unit = gen.typeUnits.get(type);
            if (!unit.empty()) {
                name += " [";
                name += unit;
                name += "]";
            }
            client::widgets::CargoTransferLine& line = del.addNew(new client::widgets::CargoTransferLine(m_root, tx, name, int(type), fmt));
            line.setAmounts(false, left.cargo.amount.get(type), left.cargo.remaining.get(type));
            line.setAmounts(true, right.cargo.amount.get(type), right.cargo.remaining.get(type));
            line.sig_move.add(this, &CargoTransferDialog::onMove);
            ui::Widget& w = ui::widgets::FocusableGroup::wrapWidget(del, 1, line);
            lineGroup.add(w);
            iter.add(w);
            m_lines.set(type, &line);
        }
        validTypes -= type;
        ++type;
    }
    win.add(lineGroup);
    win.add(iter);
    win.add(del.addNew(new ui::PrefixArgument(m_root)));

    // Buttons
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(tx("OK"),     util::Key_Return, m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx("Cancel"), util::Key_Escape, m_root));

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    if (gen.allowUnload) {
        ui::widgets::Button& btnUnload = del.addNew(new ui::widgets::Button(left.isUnloadTarget ? tx("\xE2\x86\x90 Unload") : tx("Unload \xE2\x86\x92") , 'u', m_root));
        btnUnload.sig_fire.add(this, &CargoTransferDialog::onUnload);
        g.add(btnUnload);
    }
    if (gen.allowSupplySale) {
        ui::widgets::Checkbox& cb = del.addNew(new ui::widgets::Checkbox(m_root, 's', tx("Sell supplies"), m_sellSupplies));
        cb.addDefaultImages();
        cb.setIsFocusable(false);
        g.add(cb);
    }
    g.add(m_overloadCheckbox);

    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(btnCancel);
    win.add(g);

    btnOK.sig_fire.addNewClosure(m_loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));

    win.pack();
    m_root.moveWidgetToEdge(win, gfx::CenterAlign, gfx::TopAlign, 10);
    m_root.add(win);

    bool result = m_loop.run() != 0;

    m_lines.clear();

    return result;
}

void
client::dialogs::CargoTransferDialog::onMove(int id, bool target, int amount)
{
    m_proxy.move(game::Element::Type(id), amount, !target, target, m_sellSupplies.get());
}

void
client::dialogs::CargoTransferDialog::onUnload()
{
    m_proxy.unload(m_sellSupplies.get());
}

void
client::dialogs::CargoTransferDialog::onChange(size_t side, const game::proxy::CargoTransferProxy::Cargo& cargo)
{
    if (side == 0 || side == 1) {
        bool right = (side==1);
        for (game::Element::Type e = game::Element::Type(0); e != m_lines.size(); ++e) {
            if (client::widgets::CargoTransferLine* line = m_lines.get(e)) {
                line->setAmounts(right, cargo.amount.get(e), cargo.remaining.get(e));
            }
        }
    }
}

void
client::dialogs::CargoTransferDialog::onEnableOverload()
{
    if (m_overload) {
        return;
    }

    ui::dialogs::MessageBox box(util::rich::Parser::parseXml(m_translator(
                                                                 "<small>Overload Mode allows you to load more cargo onto ships than PCC usually permits. "
                                                                 "This is useful in some situations when you exactly know what you're doing; "
                                                                 "you need to clean up manually to stay within limits.\n"
                                                                 "Ending the turn with an overloaded ship is a rule violation; "
                                                                 "Host will usually detect that and destroy excess cargo.</small>\n"
                                                                 "Turn on Overload Mode?")),
                                m_translator("Cargo Transfer"),
                                m_root);
    if (box.doYesNoDialog(m_translator)) {
        m_overload = true;
        m_overloadCheckbox.setImage(RESOURCE_ID("ui.cb1"));
        m_overloadCheckbox.setState(ui::Widget::DisabledState, true);
        m_proxy.setOverload(true);
    }    
}
