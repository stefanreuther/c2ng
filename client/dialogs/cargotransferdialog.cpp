/**
  *  \file client/dialogs/cargotransferdialog.cpp
  */

#include "client/dialogs/cargotransferdialog.hpp"
#include "afl/base/countof.hpp"
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
#include "ui/widgets/quit.hpp"
#include "util/rich/parser.hpp"

using game::Element;

struct client::dialogs::CargoTransferDialog::AddHelper {
    game::proxy::CargoTransferProxy::General gen;
    game::proxy::CargoTransferProxy::Participant left, right;
    util::NumberFormatter fmt;

    AddHelper()
        : gen(), left(), right(), fmt(false, false)
        { }
};

client::dialogs::CargoTransferDialog::CargoTransferDialog(ui::Root& root, afl::string::Translator& tx, game::proxy::CargoTransferProxy& proxy)
    : m_root(root),
      m_translator(tx),
      m_proxy(proxy),
      m_cargo(),
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

    AddHelper helper;
    Downlink link(m_root, tx);
    m_proxy.getGeneralInformation(link, helper.gen);
    m_proxy.getParticipantInformation(link, 0, helper.left);
    m_proxy.getParticipantInformation(link, 1, helper.right);
    m_cargo[0] = helper.left.cargo;
    m_cargo[1] = helper.right.cargo;
    helper.fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    if (helper.gen.validTypes.empty()) {
        ui::dialogs::MessageBox(afl::string::Format(tx("There is nothing you could transfer to or from %s."), helper.right.name),
                                tx("Cargo Transfer"), m_root).doOkDialog(tx);
        return false;
    }

    afl::base::Deleter del;
    ui::Window& win(del.addNew(new ui::Window(tx("Cargo Transfer"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));
    win.add(del.addNew(new client::widgets::CargoTransferHeader(m_root, tx, helper.left.name, helper.right.name)));

    ui::Group& lineGroup = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    ui::widgets::FocusIterator& iter = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical + ui::widgets::FocusIterator::Wrap));
    game::ElementTypes_t validTypes = helper.gen.validTypes;

    // Add cargo transfer lines.
    // Do not just use the "native" order of game::Element; it seems muscle memory is relevant after all:
    // With native order, Colonists and Supplies are swapped, and we don't want to depend on the native order.
    // Thus, first add in a fixed order...
    static const Element::Type FIXED_TYPES[] = {
        Element::Neutronium,
        Element::Tritanium,
        Element::Duranium,
        Element::Molybdenum,
        Element::Supplies,
        Element::Colonists,
        Element::Money,
    };
    for (size_t i = 0; i < countof(FIXED_TYPES); ++i) {
        const Element::Type type = FIXED_TYPES[i];
        if (validTypes.contains(type)) {
            addCargoTransferLine(type, helper, lineGroup, iter, del);
        }
        validTypes -= type;
    }

    // ...then add the remainder
    Element::Type type = Element::Type(0);
    while (!validTypes.empty()) {
        if (validTypes.contains(type)) {
            addCargoTransferLine(type, helper, lineGroup, iter, del);
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

    if (helper.gen.allowUnload) {
        ui::widgets::Button& btnUnload = del.addNew(new ui::widgets::Button(helper.left.isUnloadTarget ? tx("\xE2\x86\x90 Unload") : tx("Unload \xE2\x86\x92") , 'u', m_root));
        btnUnload.sig_fire.add(this, &CargoTransferDialog::onUnload);
        g.add(btnUnload);
    }
    if (helper.gen.allowSupplySale) {
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
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

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
client::dialogs::CargoTransferDialog::onMove(game::Element::Type id, bool target, int amount)
{
    m_proxy.move(id, amount, !target, target, m_sellSupplies.get());
}

void
client::dialogs::CargoTransferDialog::onLoadAmount(game::Element::Type id, bool target, int amount)
{
    // ex getEffectiveAmount, sort of
    bool s2m = m_sellSupplies.get();
    int toMove;
    if (id == Element::Supplies && s2m) {
        // Converting supplies to money: allow converting more toward a fixed goal
        // (=add to target), but do not allow removing.
        int32_t have = m_cargo[target].amount.get(Element::Money);
        if (amount >= have) {
            toMove = amount - have;
        } else {
            toMove = 0;
        }
    } else {
        // Normal behaviour: allow add and remove
        toMove = amount - m_cargo[target].amount.get(id);
    }

    m_proxy.move(id, toMove, !target, target, s2m);
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
        m_cargo[side] = cargo;
        for (Element::Type e = Element::Type(0); e != m_lines.size(); ++e) {
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

void
client::dialogs::CargoTransferDialog::addCargoTransferLine(game::Element::Type type,
                                                           const AddHelper& helper,
                                                           ui::Group& lineGroup,
                                                           ui::widgets::FocusIterator& iter,
                                                           afl::base::Deleter& del)
{
    String_t name = helper.gen.typeNames.get(type);
    String_t unit = helper.gen.typeUnits.get(type);
    if (!unit.empty()) {
        name += " [";
        name += unit;
        name += "]";
    }

    client::widgets::CargoTransferLine& line = del.addNew(new client::widgets::CargoTransferLine(m_root, m_translator, name, type, helper.fmt));
    line.setAmounts(false, helper.left.cargo.amount.get(type), helper.left.cargo.remaining.get(type));
    line.setAmounts(true, helper.right.cargo.amount.get(type), helper.right.cargo.remaining.get(type));
    line.sig_move.add(this, &CargoTransferDialog::onMove);
    line.sig_loadAmount.add(this, &CargoTransferDialog::onLoadAmount);
    ui::Widget& w = ui::widgets::FocusableGroup::wrapWidget(del, 1, line);
    lineGroup.add(w);
    iter.add(w);
    m_lines.set(type, &line);
}
