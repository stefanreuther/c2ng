/**
  *  \file client/dialogs/simulationconfiguration.cpp
  *  \brief Simulation Configuration Editor
  */

#include "client/dialogs/simulationconfiguration.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/request.hpp"

using ui::widgets::OptionGrid;
using game::sim::Configuration;

namespace {
    enum {
        SetMode,
        SetEngineShieldBonus,
        SetScottyBonus,
        SetRandomLeftRight,
        SetBalancingMode,
        SetHonorAlliances,
        SetOnlyOneSimulation,
        SetSeedControl,
        SetRandomizeFriendlyCode
    };

    class SimulationConfigurationEditor {
     public:
        SimulationConfigurationEditor(ui::Root& root,
                                      util::RequestSender<game::Session> gameSender,
                                      Configuration& config,
                                      afl::string::Translator& tx)
            : m_root(root),
              m_gameSender(gameSender),
              m_grid(0, 0, root),
              m_config(config),
              m_translator(tx)
            { init(); }

        bool run();

     private:
        void init();
        void render();
        void onItemClick(int n);
        void editMode();
        void editEngineShieldBonus();
        void setMode(Configuration::VcrMode mode);

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        ui::widgets::OptionGrid m_grid;
        Configuration& m_config;
        afl::string::Translator& m_translator;
    };

    String_t formatYesNo(bool flag, afl::string::Translator& tx)
    {
        return flag ? tx("Yes") : tx("No");
    }

    String_t formatOnlyOneSimulation(bool flag, afl::string::Translator& tx)
    {
        return flag ? tx("one") : tx("complete");
    }

    String_t formatRandomizeFriendlyCode(bool flag, afl::string::Translator& tx)
    {
        return flag ? tx("every time") : tx("once");
    }

    String_t formatEngineShieldBonus(int value, afl::string::Translator& tx)
    {
        return value == 0
            ? tx("none")
            : String_t(afl::string::Format("%d%%", value));
    }

    void addModeValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        Configuration::VcrMode mode = Configuration::VcrHost;
        do {
            ref.addPossibleValue(toString(mode, tx));
            mode = getNext(mode);
        } while (mode != Configuration::VcrHost);
    }

    void addEsbValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        ref.addPossibleValue(formatEngineShieldBonus(0, tx))
            .addPossibleValue(formatEngineShieldBonus(19999, tx));
    }

    void addYesNoValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        ref.addPossibleValue(formatYesNo(false, tx))
            .addPossibleValue(formatYesNo(true, tx));
    }

    void addBalancingValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        Configuration::BalancingMode mode = Configuration::BalanceNone;
        do {
            ref.addPossibleValue(toString(mode, tx));
            mode = getNext(mode);
        } while (mode != Configuration::BalanceNone);
    }

    void addOnlyOneValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        ref.addPossibleValue(formatOnlyOneSimulation(false, tx))
            .addPossibleValue(formatOnlyOneSimulation(true, tx));
    }

    void addRandomizeValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        ref.addPossibleValue(formatRandomizeFriendlyCode(false, tx))
            .addPossibleValue(formatRandomizeFriendlyCode(true, tx));
    }
}

bool
SimulationConfigurationEditor::run()
{
    // ex simoptions.cc:editSimulatorOptions
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Simulator Options"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(m_grid);

    ui::EventLoop loop(m_root);
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:simopts"));
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(btn);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}

void
SimulationConfigurationEditor::init()
{
    // ex WSimOptionTile::init, COptionsWindow.DrawInterior
    addModeValues     (m_translator, m_grid.addItem(SetMode,                  'm', m_translator("Mode")).setFont("b"));
    addEsbValues      (m_translator, m_grid.addItem(SetEngineShieldBonus,     'e', m_translator("Engine-Shield Bonus")));
    addYesNoValues    (m_translator, m_grid.addItem(SetScottyBonus,           'c', m_translator("Fed Crew Bonus")));
    addYesNoValues    (m_translator, m_grid.addItem(SetRandomLeftRight,       'r', m_translator("Random Left/Right")));
    addBalancingValues(m_translator, m_grid.addItem(SetBalancingMode,         'l', m_translator("Left/Right Balance")));
    addYesNoValues    (m_translator, m_grid.addItem(SetHonorAlliances,        'a', m_translator("Honor alliances/teams")));
    addOnlyOneValues  (m_translator, m_grid.addItem(SetOnlyOneSimulation,     'b', m_translator("Battles per simulation")));
    addYesNoValues    (m_translator, m_grid.addItem(SetSeedControl,           's', m_translator("Seed control")));
    addRandomizeValues(m_translator, m_grid.addItem(SetRandomizeFriendlyCode, 'f', m_translator("Randomize FCodes")));

    m_grid.sig_click.add(this, &SimulationConfigurationEditor::onItemClick);

    render();
}

void
SimulationConfigurationEditor::render()
{
    // ex WSimOptionTile::render, COptionsWindow.Update, COptionsWindow.DrawOptions
    // Mode
    m_grid.findItem(SetMode).setValue(toString(m_config.getMode(), m_translator));

    // ESB
    m_grid.findItem(SetEngineShieldBonus).setValue(formatEngineShieldBonus(m_config.getEngineShieldBonus(), m_translator));

    // Crew Bonus
    m_grid.findItem(SetScottyBonus).setValue(formatYesNo(m_config.hasScottyBonus(), m_translator));

    // Random left/right
    m_grid.findItem(SetRandomLeftRight).setValue(formatYesNo(m_config.hasRandomLeftRight(), m_translator));

    // Left/right balance
    m_grid.findItem(SetBalancingMode).setValue(toString(m_config.getBalancingMode(), m_translator));

    // Honor alliances
    m_grid.findItem(SetHonorAlliances).setValue(formatYesNo(m_config.hasHonorAlliances(), m_translator));

    // Battles per simulation
    m_grid.findItem(SetOnlyOneSimulation).setValue(formatOnlyOneSimulation(m_config.hasOnlyOneSimulation(), m_translator));

    // Seed control
    m_grid.findItem(SetSeedControl).setValue(formatYesNo(m_config.hasSeedControl(), m_translator));

    // Random fcodes
    m_grid.findItem(SetRandomizeFriendlyCode).setValue(formatRandomizeFriendlyCode(m_config.hasRandomizeFCodesOnEveryFight(), m_translator));
}

void
SimulationConfigurationEditor::onItemClick(int n)
{
    // ex WSimOptionTile::onClick, COptionsWindow.Handle
    switch (n) {
     case SetMode:
        editMode();
        render();
        break;
     case SetEngineShieldBonus:
        editEngineShieldBonus();
        render();
        break;
     case SetScottyBonus:
        m_config.setScottyBonus(!m_config.hasScottyBonus());
        render();
        break;
     case SetRandomLeftRight:
        m_config.setRandomLeftRight(!m_config.hasRandomLeftRight());
        render();
        break;
     case SetBalancingMode:
        m_config.setBalancingMode(getNext(m_config.getBalancingMode()));
        render();
        break;
     case SetHonorAlliances:
        m_config.setHonorAlliances(!m_config.hasHonorAlliances());
        render();
        break;
     case SetOnlyOneSimulation:
        m_config.setOnlyOneSimulation(!m_config.hasOnlyOneSimulation());
        render();
        break;
     case SetSeedControl:
        m_config.setSeedControl(!m_config.hasSeedControl());
        render();
        break;
     case SetRandomizeFriendlyCode:
        m_config.setRandomizeFCodesOnEveryFight(!m_config.hasRandomizeFCodesOnEveryFight());
        render();
        break;
    }
}

void
SimulationConfigurationEditor::editMode()
{
    // ex WSimOptionTile::chooseMode
    // Build list of modes
    ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
    Configuration::VcrMode mode = Configuration::VcrHost;
    do {
        box.addItem(mode, toString(mode, m_translator));
        mode = getNext(mode);
    } while (mode != Configuration::VcrHost);
    box.setCurrentKey(m_config.getMode());

    // Ask user
    if (ui::widgets::doStandardDialog(m_translator("Mode"), String_t(), box, true, m_root, m_translator)) {
        int32_t value;
        if (box.getCurrentKey(value)) {
            setMode(Configuration::VcrMode(value));
        }
    }
}

void
SimulationConfigurationEditor::editEngineShieldBonus()
{
    afl::base::Observable<int32_t> value(m_config.getEngineShieldBonus());
    ui::widgets::DecimalSelector select(m_root, m_translator, value, 0, 500, 10);

    if (ui::widgets::doStandardDialog(m_translator("Engine-Shield Bonus"), m_translator("Rate [%]"), select, false, m_root, m_translator)) {
        m_config.setEngineShieldBonus(value.get());
        render();
    }
}

void
SimulationConfigurationEditor::setMode(Configuration::VcrMode mode)
{
    // Configuration::setMode requires access to game::Session data.
    // This is the only game call we need, and we only need it here,
    // so for now we don't have a proxy for it.
    class Task : public util::Request<game::Session> {
     public:
        Task(Configuration& config, Configuration::VcrMode mode)
            : m_config(config), m_mode(mode)
            { }
        virtual void handle(game::Session& session)
            {
                game::Root* r = session.getRoot().get();
                game::Game* g = session.getGame().get();
                if (r != 0 && g != 0) {
                    // FIXME: this requires a game just for the team settings which makes it fail in a standalone simulator
                    m_config.setMode(m_mode, g->teamSettings(), r->hostConfiguration());
                }
            }
     private:
        Configuration& m_config;
        Configuration::VcrMode m_mode;
    };

    client::Downlink link(m_root, m_translator);
    Task t(m_config, mode);
    link.call(m_gameSender, t);

    render();
}

bool
client::dialogs::editSimulationConfiguration(ui::Root& root,
                                             util::RequestSender<game::Session> gameSender,
                                             game::sim::Configuration& config,
                                             afl::string::Translator& tx)
{
    // ex ccsim.pas:BSimOptions
    return SimulationConfigurationEditor(root, gameSender, config, tx).run();
}
