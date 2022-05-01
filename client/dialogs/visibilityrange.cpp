/**
  *  \file client/dialogs/visibilityrange.cpp
  *  \brief Visibility Range Editor
  */

#include "client/dialogs/visibilityrange.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/visibilityrangeproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using afl::string::Translator;
using client::Downlink;
using game::map::VisConfig;
using game::proxy::VisibilityRangeProxy;
using ui::Group;
using ui::widgets::Button;
using ui::widgets::DecimalSelector;
using ui::widgets::OptionGrid;
using ui::widgets::StandardDialogButtons;

namespace {
    // Option Ids for OptionGrid
    enum {
        IdMode = 1,
        IdTeam = 2
    };

    /*
     *  Dialog
     */
    class RangeDialog {
     public:
        RangeDialog(ui::Root& root, VisibilityRangeProxy& proxy, Translator& tx);
        std::auto_ptr<game::map::RangeSet> run(util::RequestSender<game::Session>& gameSender);

     private:
        void renderOptions();
        void renderRange();
        void onOptionClick(int id);
        void onDropdownClick();

        // Links
        ui::Root& m_root;
        VisibilityRangeProxy& m_proxy;
        Translator& m_translator;
        Downlink m_link;

        // Data
        VisConfig m_visConfig;

        // Widgets
        afl::base::Observable<int32_t> m_range;
        OptionGrid m_options;
        DecimalSelector m_rangeInput;
        Button m_dropdownButton;
    };
}

RangeDialog::RangeDialog(ui::Root& root, VisibilityRangeProxy& proxy, Translator& tx)
    : m_root(root),
      m_proxy(proxy),
      m_translator(tx),
      m_link(root, tx),
      m_visConfig(),
      m_range(0),
      m_options(0, root.provider().getFont(gfx::FontRequest())->getEmWidth() * 10, root),
      m_rangeInput(root, tx, m_range, 0, 1000, 10),
      m_dropdownButton(UTF_DOWN_ARROW, util::Key_Down, root)
{
    m_options.addItem(IdMode, 'r', tx("Ranges around"));
    m_options.addItem(IdTeam, 't', tx("Include team units"));
    m_options.sig_click.add(this, &RangeDialog::onOptionClick);
    m_dropdownButton.sig_fire.add(this, &RangeDialog::onDropdownClick);
}

std::auto_ptr<game::map::RangeSet>
RangeDialog::run(util::RequestSender<game::Session>& gameSender)
{
    // ex WRangeDialog::init
    // Initialize
    m_visConfig = m_proxy.loadVisibilityConfiguration(m_link);
    renderOptions();
    renderRange();

    // Build dialog
    //   VBox
    //     StaticText "Range"
    //     HBox [DecimalSelector, Button "Down"]
    //   OptionGrid
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::EventLoop loop(m_root);
    ui::Window& win = del.addNew(new ui::Window(m_translator("Ranges"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    Group& g1 = del.addNew(new Group(ui::layout::VBox::instance0));
    g1.add(del.addNew(new ui::widgets::StaticText(m_translator("Range [ly]:"), util::SkinColor::Static, "+", m_root.provider())));

    Group& g12 = del.addNew(new Group(ui::layout::HBox::instance0));
    g12.add(m_rangeInput);
    g12.add(m_dropdownButton);
    g1.add(g12);
    win.add(g1);
    win.add(m_options);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:starchart:ranges"));
    StandardDialogButtons& btn = del.addNew(new StandardDialogButtons(m_root, m_translator));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(btn);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    // Operate
    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    bool ok = loop.run();
    m_root.remove(win);

    // Do it
    std::auto_ptr<game::map::RangeSet> result;
    if (ok) {
        m_visConfig.range = m_range.get();
        result = m_proxy.buildVisibilityRange(m_link, m_visConfig);

        // Check emptiness
        if (result.get() != 0 && result->isEmpty()) {
            ui::dialogs::MessageBox(m_translator("Your selection does not contain any objects."),
                                    m_translator("Ranges"),
                                    m_root).doOkDialog(m_translator);
            result.reset();
        }
    }
    return result;
}

void
RangeDialog::renderOptions()
{
    // ex WRangeDialog::update
    m_options.findItem(IdMode).setValue(toString(m_visConfig.mode, m_translator));
    m_options.findItem(IdTeam).setValue(m_visConfig.useTeam ? m_translator("yes") : m_translator("no"))
        .setEnabled(m_visConfig.mode != game::map::VisModeMarked);
}

void
RangeDialog::renderRange()
{
    m_range.set(m_visConfig.range);
}

void
RangeDialog::onOptionClick(int id)
{
    // WRangeDialog::onClick
    switch (id) {
     case IdMode: {
        int n = m_visConfig.mode + 1;
        if (n > game::map::VisModeMax) {
            n = 0;
        }
        m_visConfig.mode = static_cast<game::map::VisMode>(n);
        renderOptions();
        break;
     }

     case IdTeam:
        m_visConfig.useTeam = !m_visConfig.useTeam;
        renderOptions();
        break;
    }
}

void
RangeDialog::onDropdownClick()
{
    // ex WRangeDialog::handleCommand (part)
    game::map::VisSettings_t settings = m_proxy.getVisibilityRangeSettings(m_link);

    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0; i < settings.size(); ++i) {
        list.addItem(int(i), Format(m_translator("%s (%d ly)"), settings[i].name, settings[i].range));
    }

    ui::EventLoop loop(m_root);
    if (ui::widgets::MenuFrame(ui::layout::VBox::instance0, m_root, loop).doMenu(list, m_dropdownButton.getExtent().getBottomLeft())) {
        int32_t key;
        if (list.getCurrentKey(key) && key >= 0 && key < int32_t(settings.size())) {
            m_visConfig.mode = settings[key].mode;
            m_visConfig.range = settings[key].range;
            renderOptions();
            renderRange();
        }
    }
}


/*
 *  Main Entry Point
 */

std::auto_ptr<game::map::RangeSet>
client::dialogs::editVisibilityRange(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // ex doEditRanges(GRangeSet& ranges), chartusr.pas:SelectDisplayRange
    VisibilityRangeProxy proxy(gameSender);
    return RangeDialog(root, proxy, tx).run(gameSender);
}
