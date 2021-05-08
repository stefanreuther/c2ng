/**
  *  \file client/dialogs/simulationfleetcostoptions.cpp
  */

#include "client/dialogs/simulationfleetcostoptions.hpp"
#include "afl/base/deleter.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

using ui::widgets::OptionGrid;

namespace {
    enum {
        SetFighterMode,
        SetUseTorpedoes,
        SetUseEngines,
        SetUsePlanetDefense,
        SetUseBaseCost,
        SetUseBaseTech,
        SetShipTechMode,
        SetByTeam
    };

    /*
     *  Utilities
     */

    String_t formatYesNo(bool flag, afl::string::Translator& tx)
    {
        return flag ? tx("Yes") : tx("No");
    }

    String_t formatByTeam(bool* flag, afl::string::Translator& tx)
    {
        return (flag
                ? (*flag
                   ? tx("by team")
                   : tx("by player"))
                : UTF_EN_DASH);
    }

    template<typename T>
    void addModeValues(afl::string::Translator& tx, OptionGrid::Ref ref, T initialValue)
    {
        T value = initialValue;
        do {
            ref.addPossibleValue(toString(value, tx));
            value = getNext(value);
        } while (value != initialValue);
    }

    void addYesNoValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        ref.addPossibleValue(formatYesNo(false, tx))
            .addPossibleValue(formatYesNo(true, tx));
    }

    void addByTeamValues(afl::string::Translator& tx, OptionGrid::Ref ref)
    {
        bool t = true, f = false;
        ref.addPossibleValue(formatByTeam(0, tx))
            .addPossibleValue(formatByTeam(&t, tx))
            .addPossibleValue(formatByTeam(&f, tx));
    }

    /*
     *  Dialog
     */

    class FleetCostOptionsDialog {
     public:
        FleetCostOptionsDialog(ui::Root& root,
                               util::RequestSender<game::Session> gameSender,
                               game::sim::FleetCostOptions& options,
                               bool* byTeam,
                               afl::string::Translator& tx)
            : m_root(root),
              m_gameSender(gameSender),
              m_grid(0, 0, root),
              m_options(options),
              m_byTeam(byTeam),
              m_translator(tx)
            { init(); }

        bool run();

     private:
        void init();
        void render();
        void onClick(int id);

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        ui::widgets::OptionGrid m_grid;
        game::sim::FleetCostOptions& m_options;
        bool* m_byTeam;
        afl::string::Translator& m_translator;
    };
}


bool
FleetCostOptionsDialog::run()
{
    // ex editFleetCostOptions
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Fleet Cost Comparison"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(m_grid);

    ui::EventLoop loop(m_root);
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:fleetcostopts"));
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
FleetCostOptionsDialog::init()
{
    // ex WFleetCostOptionsTile::init
    addModeValues  (m_translator, m_grid.addItem(SetFighterMode,      'f', m_translator("Fighter cost")), game::sim::FleetCostOptions::FreeFighters);
    addYesNoValues (m_translator, m_grid.addItem(SetUseTorpedoes,     't', m_translator("Include torpedoes")));
    addYesNoValues (m_translator, m_grid.addItem(SetUseEngines,       'e', m_translator("Include engines")));
    addYesNoValues (m_translator, m_grid.addItem(SetUsePlanetDefense, 'p', m_translator("Include planet defense")));
    addYesNoValues (m_translator, m_grid.addItem(SetUseBaseCost,      'b', m_translator("Include starbase")));
    addYesNoValues (m_translator, m_grid.addItem(SetUseBaseTech,      'l', m_translator("Include starbase tech levels")));
    addModeValues  (m_translator, m_grid.addItem(SetShipTechMode,     's', m_translator("Include ship tech levels")), game::sim::FleetCostOptions::ShipTech);
    addByTeamValues(m_translator, m_grid.addItem(SetByTeam,           'y', m_translator("Display by")));
    m_grid.sig_click.add(this, &FleetCostOptionsDialog::onClick);
    render();
}

void
FleetCostOptionsDialog::render()
{
    m_grid.findItem(SetFighterMode)     .setValue(toString(m_options.fighterMode,         m_translator));
    m_grid.findItem(SetUseTorpedoes)    .setValue(formatYesNo(m_options.useTorpedoes,     m_translator));
    m_grid.findItem(SetUseEngines)      .setValue(formatYesNo(m_options.useEngines,       m_translator));
    m_grid.findItem(SetUsePlanetDefense).setValue(formatYesNo(m_options.usePlanetDefense, m_translator));
    m_grid.findItem(SetUseBaseCost)     .setValue(formatYesNo(m_options.useBaseCost,      m_translator));
    m_grid.findItem(SetUseBaseTech)     .setValue(formatYesNo(m_options.useBaseTech,      m_translator));
    m_grid.findItem(SetShipTechMode)    .setValue(toString(m_options.shipTechMode,        m_translator));
    m_grid.findItem(SetByTeam)          .setValue(formatByTeam(m_byTeam,                  m_translator)).setEnabled(m_byTeam != 0);
}

void
FleetCostOptionsDialog::onClick(int id)
{
    // ex WFleetCostOptionsTile::onClick(int id)
    switch (id) {
     case SetFighterMode:
        m_options.fighterMode = getNext(m_options.fighterMode);
        render();
        break;
     case SetUseTorpedoes:
        m_options.useTorpedoes = !m_options.useTorpedoes;
        render();
        break;
     case SetUseEngines:
        m_options.useEngines = !m_options.useEngines;
        render();
        break;
     case SetUsePlanetDefense:
        m_options.usePlanetDefense = !m_options.usePlanetDefense;
        render();
        break;
     case SetUseBaseCost:
        m_options.useBaseCost = !m_options.useBaseCost;
        render();
        break;
     case SetUseBaseTech:
        m_options.useBaseTech = !m_options.useBaseTech;
        render();
        break;
     case SetShipTechMode:
        m_options.shipTechMode = getNext(m_options.shipTechMode);
        render();
        break;
     case SetByTeam:
        if (m_byTeam) {
            *m_byTeam = !*m_byTeam;
            render();
        }
        break;
    }
}

bool
client::dialogs::editSimulationFleetCostOptions(ui::Root& root,
                                                util::RequestSender<game::Session> gameSender,
                                                game::sim::FleetCostOptions& options,
                                                bool* byTeam,
                                                afl::string::Translator& tx)
{
    // ex ccsim.pas:FleetCostOptions
    return FleetCostOptionsDialog(root, gameSender, options, byTeam, tx).run();
}
