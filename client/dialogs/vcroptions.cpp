/**
  *  \file client/dialogs/vcroptions.cpp
  *  \brief VCR Options Dialog
  */

#include "client/dialogs/vcroptions.hpp"

#include "afl/functional/stringtable.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

using afl::functional::createStringTable;

namespace {
    const char*const RENDERER_MODE[] = {
        N_("standard"),
        N_("traditional"),
        N_("interleaved"),
    };

    const char*const EFFECTS_MODE[] = {
        N_("standard"),
        N_("simple"),
    };

    const char*const FLAK_RENDERER_MODE[] = {
        N_("3-D"),
        N_("flat"),
    };

    const char*const BOOL_MODE[] = {
        N_("no"),
        N_("yes"),
    };

    class Dialog {
     public:
        enum {
            RendererMode,
            EffectsMode,
            FlakRendererMode,
            FlakGrid
        };

        Dialog(ui::Root& root, afl::string::Translator& tx, client::vcr::Configuration& config)
            : m_root(root),
              m_translator(tx),
              m_config(config),
              m_grid1(0, 0, root),
              m_grid2(0, 0, root)
            {
                m_grid1.addItem(RendererMode,     'r', tx("Display mode"))
                    .addPossibleValues(createStringTable(RENDERER_MODE).map(tx));
                m_grid1.addItem(EffectsMode,      'e', tx("Effects"))
                    .addPossibleValues(createStringTable(EFFECTS_MODE).map(tx));
                m_grid2.addItem(FlakRendererMode, 'f', tx("Display mode"))
                    .addPossibleValues(createStringTable(FLAK_RENDERER_MODE).map(tx));
                m_grid2.addItem(FlakGrid,         'g', tx("Show grid"))
                    .addPossibleValues(createStringTable(BOOL_MODE).map(tx));
                m_grid1.sig_click.add(this, &Dialog::onOptionClick);
                m_grid2.sig_click.add(this, &Dialog::onOptionClick);
                render();
            }

        bool run(ui::Widget* pHelp)
            {
                ui::Window win(m_translator("VCR Options"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                ui::widgets::StaticText text1(m_translator("Classic"), util::SkinColor::Static, "b", m_root.provider());
                ui::widgets::StaticText text2(m_translator("FLAK"), util::SkinColor::Static, "b", m_root.provider());
                win.add(text1);
                win.add(m_grid1);
                win.add(text2);
                win.add(m_grid2);

                ui::widgets::StandardDialogButtons btn(m_root, m_translator);
                if (pHelp != 0) {
                    btn.addHelp(*pHelp);
                }
                win.add(btn);

                ui::EventLoop loop(m_root);
                btn.addStop(loop);

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return loop.run() != 0;
            }

     private:
        void onOptionClick(int id)
            {
                switch (id) {
                 case RendererMode:
                    m_config.cycleRendererMode();
                    break;
                 case EffectsMode:
                    m_config.cycleEffectsMode();
                    break;
                 case FlakRendererMode:
                    m_config.cycleFlakRendererMode();
                    break;
                 case FlakGrid:
                    m_config.toggleFlakGrid();
                    break;
                }
                render();
            }

        void render()
            {
                m_grid1.findItem(RendererMode)    .setValue(createStringTable(RENDERER_MODE)     .map(m_translator).get(m_config.getRendererMode()));
                m_grid1.findItem(EffectsMode)     .setValue(createStringTable(EFFECTS_MODE)      .map(m_translator).get(m_config.getEffectsMode()));
                m_grid2.findItem(FlakRendererMode).setValue(createStringTable(FLAK_RENDERER_MODE).map(m_translator).get(m_config.getFlakRendererMode()));
                m_grid2.findItem(FlakGrid)        .setValue(createStringTable(BOOL_MODE)         .map(m_translator).get(m_config.hasFlakGrid()));
            }

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::vcr::Configuration& m_config;
        ui::widgets::OptionGrid m_grid1;
        ui::widgets::OptionGrid m_grid2;
    };
}

bool
client::dialogs::editVcrConfiguration(ui::Root& root, afl::string::Translator& tx, client::vcr::Configuration& config, ui::Widget* pHelp)
{
    return Dialog(root, tx, config).run(pHelp);
}

void
client::dialogs::editVcrOptions(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
{
    // Load
    Downlink link(root, tx);
    game::proxy::ConfigurationProxy proxy(gameSender);
    client::vcr::Configuration config;
    config.load(link, proxy);

    // Edit
    client::widgets::HelpWidget help(root, tx, gameSender, "pcc2:vcr:options");
    if (editVcrConfiguration(root, tx, config, &help)) {
        config.save(proxy);
    }
}
