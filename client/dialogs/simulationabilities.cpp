/**
  *  \file client/dialogs/simulationabilities.cpp
  *  \brief Simulation Unit Abilities Editor
  */

#include "client/dialogs/simulationabilities.hpp"
#include "afl/base/deleter.hpp"
#include "afl/functional/stringtable.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

namespace {
    /*
     *  Possible statuses of an ability
     */

    const char*const VALUES[] = {
        N_("no"),
        N_("yes"),
        N_("default (=no)"),
        N_("default (=yes)"),
    };

    size_t getTableIndex(const game::proxy::SimulationSetupProxy::AbilityChoices& choices, game::sim::Ability a)
    {
        // ex WSimFunctionEditor::getFunctionType
        if (choices.set.contains(a)) {
            if (choices.active.contains(a)) {
                return 1;
            } else {
                return 0;
            }
        } else {
            if (choices.implied.contains(a)) {
                return 3;
            } else {
                return 2;
            }
        }
    }

    /*
     *  Dialog Class
     */

    class SimulationAbilityEditor {
     public:
        SimulationAbilityEditor(ui::Root& root, game::proxy::SimulationSetupProxy::AbilityChoices& choices, afl::string::Translator& tx)
            : m_root(root),
              m_grid(0, 0, root),
              m_choices(choices),
              m_translator(tx)
            { init(); }

        bool run(util::RequestSender<game::Session> gameSender);

     private:
        ui::Root& m_root;
        ui::widgets::OptionGrid m_grid;
        game::proxy::SimulationSetupProxy::AbilityChoices& m_choices;
        afl::string::Translator& m_translator;

        void init();
        void addAbility(util::Key_t key, game::sim::Ability ability);
        void render();
        void onItemClick(int n);
    };
}

bool
SimulationAbilityEditor::run(util::RequestSender<game::Session> gameSender)
{
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Abilities"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(m_grid);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:simfunctions"));
    win.add(help);

    ui::EventLoop loop(m_root);
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(btn);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}

void
SimulationAbilityEditor::init()
{
    // ex WSimFunctionEditor::init (part)
    addAbility('i', game::sim::PlanetImmunityAbility);
    addAbility('f', game::sim::FullWeaponryAbility);
    addAbility('c', game::sim::CommanderAbility);
    addAbility('k', game::sim::TripleBeamKillAbility);
    addAbility('b', game::sim::DoubleBeamChargeAbility);
    addAbility('t', game::sim::DoubleTorpedoChargeAbility);
    addAbility('e', game::sim::ElusiveAbility);
    addAbility('q', game::sim::SquadronAbility);
    addAbility('g', game::sim::ShieldGeneratorAbility);
    addAbility('y', game::sim::CloakedBaysAbility);
    m_grid.sig_click.add(this, &SimulationAbilityEditor::onItemClick);
    render();
}

void
SimulationAbilityEditor::addAbility(util::Key_t key, game::sim::Ability ability)
{
    if (m_choices.available.contains(ability)) {
        ui::widgets::OptionGrid::Ref ref = m_grid.addItem(ability, key, toString(ability, m_translator));
        ref.addPossibleValues(afl::functional::createStringTable(VALUES).map(m_translator));
    }
}

void
SimulationAbilityEditor::render()
{
    // ex WSimFunctionEditor::render
    for (int i = game::sim::FIRST_ABILITY; i <= game::sim::LAST_ABILITY; ++i) {
        m_grid.findItem(i).setValue(m_translator(VALUES[getTableIndex(m_choices, game::sim::Ability(i))]));
    }
}

void
SimulationAbilityEditor::onItemClick(int n)
{
    // ex WSimFunctionEditor::onClick, WSimFunctionEditor::toggle
    game::sim::Ability a = game::sim::Ability(n);
    if (!m_choices.set.contains(a)) {
        // Default > Yes
        m_choices.set += a;
        m_choices.active += a;
    } else if (m_choices.active.contains(a)) {
        // Yes > No
        m_choices.active -= a;
    } else {
        // No > Default
        m_choices.set -= a;
    }

    render();
}


bool
client::dialogs::editSimulationAbilities(ui::Root& root,
                                         util::RequestSender<game::Session> gameSender,
                                         game::proxy::SimulationSetupProxy::AbilityChoices& choices,
                                         afl::string::Translator& tx)
{
    return SimulationAbilityEditor(root, choices, tx).run(gameSender);
}
