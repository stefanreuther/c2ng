/**
  *  \file client/dialogs/zoomlevel.cpp
  *  \brief Zoom Level Input
  */

#include "client/dialogs/zoomlevel.hpp"
#include "afl/base/deleter.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/root.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/string.hpp"

namespace {
    class Dialog {
     public:
        Dialog(const client::map::Renderer& renderer, client::dialogs::ZoomLevel& result, ui::Root& root, afl::string::Translator& tx)
            : m_renderer(renderer),
              m_result(result),
              m_root(root),
              m_translator(tx),
              m_input(20, 10, root),
              m_buttons(root, tx),
              m_loop(root)
            {
                m_input.setText(util::formatZoomLevel(renderer.getZoomMultiplier(), renderer.getZoomDivider()));
                m_input.sig_change.add(this, &Dialog::onChange);
                m_buttons.ok().sig_fire.add(this, &Dialog::onOK);
                m_buttons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
                onChange();
            }

        bool run()
            {
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Zoom"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(del.addNew(new ui::widgets::StaticText(m_translator("Zoom level:"), util::SkinColor::Static, "+", m_root.provider())));
                win.add(m_input);
                win.add(del.addNew(new ui::rich::StaticText(m_translator("Enter zoom level in format \"3\" (zoom in) or \"1/4\" resp. \"1:4\" (zoom out)."), m_root.provider().getFont(gfx::FontRequest())->getEmWidth()*20, m_root.provider())));
                win.add(m_buttons);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run() != 0;
            }

        void onOK()
            {
                int m = 0, d = 0;
                if (util::parseZoomLevel(m_input.getText(), m, d) && m_renderer.isValidZoomLevel(m, d)) {
                    m_result.mult = m;
                    m_result.divi = d;
                    m_loop.stop(1);
                }
            }

        void onChange()
            {
                m_buttons.ok().setState(ui::Widget::DisabledState, !isValid());
            }

        bool isValid()
            {
                int m = 0, d = 0;
                return util::parseZoomLevel(m_input.getText(), m, d) && m_renderer.isValidZoomLevel(m, d);
            }

     private:
        const client::map::Renderer& m_renderer;
        client::dialogs::ZoomLevel& m_result;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::widgets::InputLine m_input;
        ui::widgets::StandardDialogButtons m_buttons;
        ui::EventLoop m_loop;
    };
}

bool
client::dialogs::editZoomLevel(const client::map::Renderer& renderer, ZoomLevel& result, ui::Root& root, afl::string::Translator& tx)
{
    // ex chartdlg.pas:EditZoom
    return Dialog(renderer, result, root, tx).run();
}
