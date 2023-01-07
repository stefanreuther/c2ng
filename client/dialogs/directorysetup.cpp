/**
  *  \file client/dialogs/directorysetup.cpp
  *  \brief Game Directory Setup dialog
  */

#include "client/dialogs/directorysetup.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/directoryselectiondialog.hpp"
#include "client/downlink.hpp"
#include "client/imageloader.hpp"
#include "gfx/canvas.hpp"
#include "gfx/rgbapixmap.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using afl::string::Format;
using game::browser::Browser;
using game::proxy::BrowserProxy;
using ui::dialogs::MessageBox;

namespace {
    afl::base::Ptr<gfx::Canvas> makeSubImage(afl::base::Ptr<gfx::Canvas> orig, int x, int y, int w, int h)
    {
        if (orig.get() == 0) {
            return 0;
        } else {
            // FIXME: we should be able to make a canvas compatible to the UI window
            afl::base::Ref<gfx::Canvas> pix = gfx::RGBAPixmap::create(w, h)->makeCanvas();
            pix->blit(gfx::Point(-x, -y), *orig, gfx::Rectangle(x, y, w, h));
            return pix.asPtr();
        }
    }

    bool verifyLocalDirectory(ui::Root& root, afl::string::Translator& tx, BrowserProxy& proxy, const String_t& dirName)
    {
        client::Downlink link(root, tx);
        BrowserProxy::DirectoryStatus_t result = proxy.verifyLocalDirectory(link, dirName);
        bool ok = false;
        switch (result) {
         case Browser::Missing:
            MessageBox(Format(tx("The directory \"%s\" is not accessible and cannot be used."), dirName),
                       tx("Game Directory Setup"), root).doOkDialog(tx);
            break;

         case Browser::Success:
            ok = true;
            break;

         case Browser::NotWritable:
            MessageBox(Format(tx("The directory \"%s\" is not writable and cannot be used."), dirName),
                       tx("Game Directory Setup"), root).doOkDialog(tx);
            break;

         case Browser::NotEmpty:
            ok = MessageBox(Format(tx("The directory \"%s\" is not empty. Use anyway?"), dirName),
                            tx("Game Directory Setup"), root).doYesNoDialog(tx);
            break;
        }
        return ok;
    }
}

bool
client::dialogs::doDirectorySetupDialog(game::proxy::BrowserProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx)
{
    ImageLoader loader(root, tx);
    loader.loadImage("gamedirsetup");
    loader.wait();

    afl::base::Ptr<gfx::Canvas> pix = root.provider().getImage("gamedirsetup");

    const int WIDTH = 600; /* FIXME */
    ui::widgets::RichListbox box(root.provider(), root.colorScheme());
    box.setPreferredWidth(WIDTH);
    box.setRenderFlag(box.UseBackgroundColorScheme, true);
    box.addItem(util::rich::Parser::parseXml(tx("<big>Automatic</big>\n"
                                                "PCC2 will automatically assign a directory within your profile directory. "
                                                "If unsure, choose this.")), makeSubImage(pix, 0, 0, 72, 64), true);
    box.addItem(util::rich::Parser::parseXml(tx("<big>Manual</big>\n"
                                                "Manually assign a directory. Use if you want to have full control.")), makeSubImage(pix, 0, 64, 72, 64), true);
    box.addItem(util::rich::Parser::parseXml(tx("<big>None</big>\n"
                                                "Do not assign a directory. The game will be opened for viewing only, and no changes can be saved.")), makeSubImage(pix, 0, 128, 72, 64), true);

    ui::Window window(tx("Game Directory Setup"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    ui::rich::StaticText intro(util::rich::Parser::parseXml(tx("<font color=\"static\">This game does not yet have an associated game directory. "
                                                               "PCC2 needs a directory on your computer to store configuration and history data. "
                                                               "Please choose how the directory should be assigned.</font>")),
                               WIDTH, root.provider());
    window.add(intro);
    window.add(box);

    ui::widgets::StandardDialogButtons btns(root, tx);
    if (pHelp != 0) {
        btns.addHelp(*pHelp);
        window.add(*pHelp);
    }
    window.add(btns);
    window.pack();

    ui::EventLoop loop(root);
    btns.addStop(loop);
    box.requestFocus();

    root.centerWidget(window);
    root.add(window);
    if (!loop.run()) {
        return false;
    }
    root.remove(window);
    switch (box.getCurrentItem()) {
     case 0:
        // Auto
        proxy.setLocalDirectoryAutomatically();
        break;

     case 1: {
        // Manual
        String_t s;
        while (1) {
            if (!doDirectorySelectionDialog(root, tx, proxy.fileSystem(), s)) {
                return false;
            }
            if (verifyLocalDirectory(root, tx, proxy, s)) {
                break;
            }
        }
        proxy.setLocalDirectoryName(s);
        break;
     }
     case 2:
        // None
        proxy.setLocalDirectoryNone();
        break;
    }
    return true;
}
